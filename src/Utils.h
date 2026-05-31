#pragma once

#include <string>
#include <vector>

std::wstring Trim(const std::wstring& value);
std::wstring ToLower(const std::wstring& value);
std::wstring FormatBytes(unsigned long long bytes);
std::wstring CsvEscape(const std::wstring& value);
std::wstring JsonEscape(const std::wstring& value);
std::string WideToUtf8(const std::wstring& value);
std::wstring Utf8ToWide(const std::string& value);
bool EqualsIgnoreCase(const std::wstring& left, const std::wstring& right);
std::wstring Join(const std::vector<std::wstring>& values, const std::wstring& separator);
std::wstring NormalizeNewlinesForWindowsEdit(const std::wstring& value);
