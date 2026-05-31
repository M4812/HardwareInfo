#include "GuiViewModel.h"

#include "Utils.h"

#include <sstream>

namespace {
std::wstring ValueOrDash(const std::wstring& value) {
    return Trim(value).empty() ? L"未读取到" : value;
}

std::wstring UIntText(unsigned int value) {
    std::wostringstream stream;
    stream << value;
    return stream.str();
}

std::wstring CpuSummary(const SystemReport& report) {
    if (report.cpus.empty()) {
        return L"未读取到";
    }

    std::wostringstream stream;
    stream << ValueOrDash(report.cpus[0].name);
    if (report.cpus[0].cores > 0 || report.cpus[0].logicalProcessors > 0) {
        stream << L" | " << report.cpus[0].cores << L"核/" << report.cpus[0].logicalProcessors << L"线程";
    }
    return stream.str();
}

std::wstring FirstNetworkIp(const SystemReport& report) {
    for (size_t i = 0; i < report.networks.size(); ++i) {
        if (!report.networks[i].ipAddresses.empty()) {
            return Join(report.networks[i].ipAddresses, L"; ");
        }
    }
    return L"";
}

std::wstring FirstNetworkMac(const SystemReport& report) {
    for (size_t i = 0; i < report.networks.size(); ++i) {
        if (!report.networks[i].macAddress.empty()) {
            return report.networks[i].macAddress;
        }
    }
    return L"";
}

std::wstring FirstNetworkDescription(const SystemReport& report) {
    for (size_t i = 0; i < report.networks.size(); ++i) {
        if (!report.networks[i].description.empty()) {
            return report.networks[i].description;
        }
    }
    return L"";
}

std::wstring FirstBoardSerial(const SystemReport& report) {
    if (!report.boards.empty()) {
        return report.boards[0].serialNumber;
    }
    return L"";
}

std::wstring FirstBiosSerial(const SystemReport& report) {
    if (!report.bios.empty()) {
        return report.bios[0].serialNumber;
    }
    return L"";
}

void AddField(std::vector<GuiField>& fields, const std::wstring& label, const std::wstring& value) {
    GuiField field;
    field.label = label;
    field.value = ValueOrDash(value);
    fields.push_back(field);
}
}

std::wstring BuildDiskLine(const DiskInfo& disk) {
    std::wostringstream stream;
    stream << ValueOrDash(disk.model) << L" | " << FormatBytes(disk.sizeBytes) << L" | "
           << ValueOrDash(disk.interfaceType) << L" | " << ValueOrDash(disk.serialNumber);
    return stream.str();
}

std::wstring BuildMemoryLine(const MemoryInfo& memory) {
    std::wostringstream stream;
    stream << ValueOrDash(memory.deviceLocator) << L" | " << FormatBytes(memory.capacityBytes) << L" | ";
    if (memory.speedMhz > 0) {
        stream << memory.speedMhz << L"MHz";
    } else {
        stream << L"未读取到";
    }
    stream << L" | " << ValueOrDash(memory.serialNumber);
    return stream.str();
}

std::vector<GuiField> BuildSummaryFields(const SystemReport& report) {
    std::vector<GuiField> fields;

    AddField(fields, L"操作系统", report.compatibility.osCaption);
    AddField(fields, L"系统版本", report.compatibility.osVersion);
    AddField(fields, L"启动时间", report.bootTime);
    AddField(fields, L"主机名", report.computerName);
    AddField(fields, L"当前用户", report.userName);
    AddField(fields, L"厂商型号", ValueOrDash(report.manufacturer) + L" " + ValueOrDash(report.model));
    AddField(fields, L"物理内存", FormatBytes(report.totalPhysicalMemoryBytes));
    AddField(fields, L"CPU", CpuSummary(report));
    AddField(fields, L"CPU ID", report.cpus.empty() ? L"" : report.cpus[0].processorId);
    AddField(fields, L"IP 地址", FirstNetworkIp(report));
    AddField(fields, L"MAC 地址", FirstNetworkMac(report));
    AddField(fields, L"网卡名称", FirstNetworkDescription(report));
    AddField(fields, L"BIOS序列号", FirstBiosSerial(report));
    AddField(fields, L"主板序列号", FirstBoardSerial(report));
    AddField(fields, L"管理员权限", report.compatibility.isAdministrator ? L"是" : L"否");
    AddField(fields, L"WMI状态", report.compatibility.wmiAvailable ? L"正常" : L"异常");

    return fields;
}

std::wstring BuildClientCreditText() {
    return L"by@muhan";
}
