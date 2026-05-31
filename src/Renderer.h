#pragma once

#include "Models.h"

#include <string>

std::wstring RenderTextReport(const SystemReport& report);
std::wstring RenderJsonReport(const SystemReport& report);
std::wstring RenderCsvReport(const SystemReport& report);
bool WriteUtf8File(const std::wstring& path, const std::wstring& content, std::wstring* error);
