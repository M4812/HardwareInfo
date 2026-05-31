#include "WmiClient.h"

#include "Utils.h"

#include <comdef.h>
#include <sstream>
#include <windows.h>

#pragma comment(lib, "wbemuuid.lib")

namespace {
std::wstring HResultToString(HRESULT hr) {
    std::wostringstream stream;
    stream << L"0x" << std::hex << static_cast<unsigned long>(hr);
    return stream.str();
}

std::wstring VariantToString(const VARIANT& value) {
    if (value.vt == VT_NULL || value.vt == VT_EMPTY) {
        return L"";
    }

    VARIANT copy;
    VariantInit(&copy);
    if (SUCCEEDED(VariantChangeType(&copy, const_cast<VARIANT*>(&value), 0, VT_BSTR))) {
        std::wstring result = copy.bstrVal ? copy.bstrVal : L"";
        VariantClear(&copy);
        return Trim(result);
    }

    return L"";
}

std::vector<std::wstring> VariantToStringVector(const VARIANT& value) {
    std::vector<std::wstring> values;
    if ((value.vt & VT_ARRAY) == VT_ARRAY) {
        SAFEARRAY* array = value.parray;
        if (!array) {
            return values;
        }

        LONG lower = 0;
        LONG upper = -1;
        if (FAILED(SafeArrayGetLBound(array, 1, &lower)) || FAILED(SafeArrayGetUBound(array, 1, &upper))) {
            return values;
        }

        for (LONG i = lower; i <= upper; ++i) {
            if ((value.vt & VT_BSTR) == VT_BSTR) {
                BSTR item = NULL;
                if (SUCCEEDED(SafeArrayGetElement(array, &i, &item))) {
                    const std::wstring text = Trim(item ? item : L"");
                    if (!text.empty()) {
                        values.push_back(text);
                    }
                    SysFreeString(item);
                }
            } else if ((value.vt & VT_VARIANT) == VT_VARIANT) {
                VARIANT item;
                VariantInit(&item);
                if (SUCCEEDED(SafeArrayGetElement(array, &i, &item))) {
                    const std::wstring text = VariantToString(item);
                    if (!text.empty()) {
                        values.push_back(text);
                    }
                }
                VariantClear(&item);
            }
        }
        return values;
    }

    const std::wstring text = VariantToString(value);
    if (!text.empty()) {
        values.push_back(text);
    }
    return values;
}
}

WmiClient::WmiClient()
    : locator_(NULL),
      services_(NULL),
      comInitialized_(false),
      securityInitialized_(false),
      initialized_(false) {
}

WmiClient::~WmiClient() {
    if (services_) {
        services_->Release();
    }
    if (locator_) {
        locator_->Release();
    }
    if (comInitialized_) {
        CoUninitialize();
    }
}

bool WmiClient::Initialize(std::wstring* error) {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        if (error) {
            *error = L"COM 初始化失败: " + HResultToString(hr);
        }
        return false;
    }
    comInitialized_ = SUCCEEDED(hr);

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    if (SUCCEEDED(hr)) {
        securityInitialized_ = true;
    } else if (hr != RPC_E_TOO_LATE) {
        if (error) {
            *error = L"COM 安全初始化失败: " + HResultToString(hr);
        }
        return false;
    }

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&locator_));
    if (FAILED(hr)) {
        if (error) {
            *error = L"创建 WMI Locator 失败: " + HResultToString(hr);
        }
        return false;
    }

    BSTR namespacePath = SysAllocString(L"ROOT\\CIMV2");
    hr = locator_->ConnectServer(namespacePath, NULL, NULL, 0, NULL, 0, 0, &services_);
    SysFreeString(namespacePath);

    if (FAILED(hr)) {
        if (error) {
            *error = L"连接 ROOT\\CIMV2 失败: " + HResultToString(hr);
        }
        return false;
    }

    hr = CoSetProxyBlanket(
        services_,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    if (FAILED(hr)) {
        if (error) {
            *error = L"设置 WMI 代理权限失败: " + HResultToString(hr);
        }
        return false;
    }

    initialized_ = true;
    return true;
}

bool WmiClient::IsInitialized() const {
    return initialized_;
}

std::vector<WmiRow> WmiClient::Query(const std::wstring& wql, const std::vector<std::wstring>& fields, std::wstring* error) const {
    std::vector<WmiRow> rows;
    if (!initialized_ || !services_) {
        if (error) {
            *error = L"WMI 尚未初始化。";
        }
        return rows;
    }

    IEnumWbemClassObject* enumerator = NULL;
    BSTR language = SysAllocString(L"WQL");
    BSTR query = SysAllocString(wql.c_str());
    HRESULT hr = services_->ExecQuery(
        language,
        query,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &enumerator);
    SysFreeString(language);
    SysFreeString(query);

    if (FAILED(hr)) {
        if (error) {
            *error = L"WMI 查询失败: " + wql + L", 错误: " + HResultToString(hr);
        }
        return rows;
    }

    IWbemClassObject* object = NULL;
    ULONG returned = 0;
    while (enumerator) {
        hr = enumerator->Next(WBEM_INFINITE, 1, &object, &returned);
        if (FAILED(hr) || returned == 0) {
            break;
        }

        WmiRow row;
        for (size_t i = 0; i < fields.size(); ++i) {
            VARIANT value;
            VariantInit(&value);
            hr = object->Get(fields[i].c_str(), 0, &value, 0, 0);
            if (SUCCEEDED(hr)) {
                row[fields[i]] = VariantToStringVector(value);
            } else {
                row[fields[i]] = std::vector<std::wstring>();
            }
            VariantClear(&value);
        }

        rows.push_back(row);
        object->Release();
        object = NULL;
    }

    if (object) {
        object->Release();
    }
    if (enumerator) {
        enumerator->Release();
    }

    return rows;
}

std::wstring FirstValue(const WmiRow& row, const std::wstring& field) {
    WmiRow::const_iterator found = row.find(field);
    if (found == row.end() || found->second.empty()) {
        return L"";
    }
    return found->second[0];
}

unsigned int FirstUInt(const WmiRow& row, const std::wstring& field) {
    const std::wstring text = FirstValue(row, field);
    if (text.empty()) {
        return 0;
    }
    return static_cast<unsigned int>(_wtoi(text.c_str()));
}

unsigned long long FirstUInt64(const WmiRow& row, const std::wstring& field) {
    const std::wstring text = FirstValue(row, field);
    if (text.empty()) {
        return 0;
    }
    return _wtoi64(text.c_str());
}
