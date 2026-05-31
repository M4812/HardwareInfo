#pragma once

#include <map>
#include <string>
#include <vector>

#include <Wbemidl.h>

typedef std::map<std::wstring, std::vector<std::wstring> > WmiRow;

class WmiClient {
public:
    WmiClient();
    ~WmiClient();

    bool Initialize(std::wstring* error);
    bool IsInitialized() const;
    std::vector<WmiRow> Query(const std::wstring& wql, const std::vector<std::wstring>& fields, std::wstring* error) const;

private:
    IWbemLocator* locator_;
    IWbemServices* services_;
    bool comInitialized_;
    bool securityInitialized_;
    bool initialized_;

    WmiClient(const WmiClient&);
    WmiClient& operator=(const WmiClient&);
};

std::wstring FirstValue(const WmiRow& row, const std::wstring& field);
unsigned int FirstUInt(const WmiRow& row, const std::wstring& field);
unsigned long long FirstUInt64(const WmiRow& row, const std::wstring& field);
