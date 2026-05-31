#include "Utils.h"

#include <algorithm>
#include <cwctype>
#include <iomanip>
#include <sstream>
#include <windows.h>

std::wstring Trim(const std::wstring& value) {
    const std::wstring whitespace = L" \t\r\n";
    const size_t start = value.find_first_not_of(whitespace);
    if (start == std::wstring::npos) {
        return L"";
    }
    const size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

std::wstring ToLower(const std::wstring& value) {
    std::wstring result = value;
    std::transform(result.begin(), result.end(), result.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return result;
}

bool EqualsIgnoreCase(const std::wstring& left, const std::wstring& right) {
    return ToLower(left) == ToLower(right);
}

std::wstring FormatBytes(unsigned long long bytes) {
    static const wchar_t* units[] = {L"B", L"KB", L"MB", L"GB", L"TB", L"PB"};
    double value = static_cast<double>(bytes);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 5) {
        value /= 1024.0;
        ++unitIndex;
    }

    std::wostringstream stream;
    if (unitIndex == 0) {
        stream << bytes << L" " << units[unitIndex];
    } else {
        stream << std::fixed << std::setprecision(2) << value << L" " << units[unitIndex];
    }
    return stream.str();
}

std::wstring CsvEscape(const std::wstring& value) {
    bool mustQuote = value.find_first_of(L"\",\r\n") != std::wstring::npos;
    if (!mustQuote) {
        return value;
    }

    std::wstring escaped = L"\"";
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == L'"') {
            escaped += L"\"\"";
        } else {
            escaped += value[i];
        }
    }
    escaped += L"\"";
    return escaped;
}

std::wstring JsonEscape(const std::wstring& value) {
    std::wostringstream stream;
    for (size_t i = 0; i < value.size(); ++i) {
        const wchar_t ch = value[i];
        switch (ch) {
        case L'\\':
            stream << L"\\\\";
            break;
        case L'"':
            stream << L"\\\"";
            break;
        case L'\b':
            stream << L"\\b";
            break;
        case L'\f':
            stream << L"\\f";
            break;
        case L'\n':
            stream << L"\\n";
            break;
        case L'\r':
            stream << L"\\r";
            break;
        case L'\t':
            stream << L"\\t";
            break;
        default:
            if (ch < 0x20) {
                stream << L"\\u" << std::hex << std::setw(4) << std::setfill(L'0') << static_cast<int>(ch);
            } else {
                stream << ch;
            }
            break;
        }
    }
    return stream.str();
}

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) {
        return std::string();
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), NULL, 0, NULL, NULL);
    if (size <= 0) {
        return std::string();
    }

    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), &result[0], size, NULL, NULL);
    return result;
}

std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return std::wstring();
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), NULL, 0);
    if (size <= 0) {
        return std::wstring();
    }

    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), &result[0], size);
    return result;
}

std::wstring Join(const std::vector<std::wstring>& values, const std::wstring& separator) {
    std::wostringstream stream;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            stream << separator;
        }
        stream << values[i];
    }
    return stream.str();
}

std::wstring NormalizeNewlinesForWindowsEdit(const std::wstring& value) {
    std::wstring result;
    result.reserve(value.size() + 16);

    for (size_t i = 0; i < value.size(); ++i) {
        const wchar_t ch = value[i];
        if (ch == L'\r') {
            if (i + 1 < value.size() && value[i + 1] == L'\n') {
                result += L"\r\n";
                ++i;
            } else {
                result += L"\r\n";
            }
        } else if (ch == L'\n') {
            result += L"\r\n";
        } else {
            result += ch;
        }
    }

    return result;
}
