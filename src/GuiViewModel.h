#pragma once

#include "Models.h"

#include <string>
#include <vector>

struct GuiField {
    std::wstring label;
    std::wstring value;
};

std::vector<GuiField> BuildSummaryFields(const SystemReport& report);
std::wstring BuildDiskLine(const DiskInfo& disk);
std::wstring BuildMemoryLine(const MemoryInfo& memory);
std::wstring BuildClientCreditText();
