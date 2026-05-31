#include "Renderer.h"

#include "Utils.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace {
std::wstring ValueOrDash(const std::wstring& value) {
    return Trim(value).empty() ? L"-" : value;
}

void AppendLine(std::wostringstream& stream, const std::wstring& name, const std::wstring& value) {
    stream << name << L": " << ValueOrDash(value) << L"\n";
}

void AppendLine(std::wostringstream& stream, const std::wstring& name, unsigned int value) {
    stream << name << L": " << value << L"\n";
}

void AppendBytesLine(std::wostringstream& stream, const std::wstring& name, unsigned long long bytes) {
    stream << name << L": " << FormatBytes(bytes) << L"\n";
}

void AppendJsonField(std::wostringstream& stream, const std::wstring& name, const std::wstring& value, bool comma) {
    stream << L"    \"" << JsonEscape(name) << L"\": \"" << JsonEscape(value) << L"\"";
    if (comma) {
        stream << L",";
    }
    stream << L"\n";
}

void AppendJsonField(std::wostringstream& stream, const std::wstring& name, unsigned int value, bool comma) {
    stream << L"    \"" << JsonEscape(name) << L"\": " << value;
    if (comma) {
        stream << L",";
    }
    stream << L"\n";
}

void AppendJsonField64(std::wostringstream& stream, const std::wstring& name, unsigned long long value, bool comma) {
    stream << L"    \"" << JsonEscape(name) << L"\": " << value;
    if (comma) {
        stream << L",";
    }
    stream << L"\n";
}

void AppendJsonBool(std::wostringstream& stream, const std::wstring& name, bool value, bool comma) {
    stream << L"    \"" << JsonEscape(name) << L"\": " << (value ? L"true" : L"false");
    if (comma) {
        stream << L",";
    }
    stream << L"\n";
}

void AppendJsonStringArray(std::wostringstream& stream, const std::wstring& name, const std::vector<std::wstring>& values, bool comma) {
    stream << L"    \"" << JsonEscape(name) << L"\": [";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            stream << L", ";
        }
        stream << L"\"" << JsonEscape(values[i]) << L"\"";
    }
    stream << L"]";
    if (comma) {
        stream << L",";
    }
    stream << L"\n";
}

void AppendCsvRow(std::wostringstream& stream, const std::wstring& section, const std::wstring& item, const std::wstring& name, const std::wstring& value) {
    stream << CsvEscape(section) << L"," << CsvEscape(item) << L"," << CsvEscape(name) << L"," << CsvEscape(value) << L"\n";
}

std::wstring UIntText(unsigned int value) {
    std::wostringstream stream;
    stream << value;
    return stream.str();
}

std::wstring UInt64Text(unsigned long long value) {
    std::wostringstream stream;
    stream << value;
    return stream.str();
}
}

std::wstring RenderTextReport(const SystemReport& report) {
    std::wostringstream stream;
    stream << L"HardwareInfo 本机硬件信息报告\n";
    stream << L"================================\n\n";

    stream << L"兼容性检查\n";
    AppendLine(stream, L"系统", report.compatibility.osCaption);
    AppendLine(stream, L"系统版本", report.compatibility.osVersion);
    AppendLine(stream, L"系统架构", report.compatibility.architecture);
    AppendLine(stream, L"管理员权限", report.compatibility.isAdministrator ? L"是" : L"否");
    AppendLine(stream, L"WMI 状态", report.compatibility.wmiAvailable ? L"正常" : L"异常");
    AppendLine(stream, L"采集模式", L"本地离线");
    stream << L"\n";

    stream << L"服务器信息\n";
    AppendLine(stream, L"主机名", report.computerName);
    AppendLine(stream, L"当前用户", report.userName);
    AppendLine(stream, L"域/工作组", report.domainOrWorkgroup);
    AppendLine(stream, L"厂商", report.manufacturer);
    AppendLine(stream, L"型号", report.model);
    AppendBytesLine(stream, L"物理内存总量", report.totalPhysicalMemoryBytes);
    AppendLine(stream, L"上次启动时间", report.bootTime);
    stream << L"\n";

    stream << L"CPU 信息\n";
    for (size_t i = 0; i < report.cpus.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"型号", report.cpus[i].name);
        AppendLine(stream, L"厂商", report.cpus[i].manufacturer);
        AppendLine(stream, L"处理器ID", report.cpus[i].processorId);
        AppendLine(stream, L"核心数", report.cpus[i].cores);
        AppendLine(stream, L"线程数", report.cpus[i].logicalProcessors);
        AppendLine(stream, L"最大频率MHz", report.cpus[i].maxClockMhz);
    }
    stream << L"\n";

    stream << L"硬盘信息\n";
    for (size_t i = 0; i < report.disks.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"型号", report.disks[i].model);
        AppendLine(stream, L"接口", report.disks[i].interfaceType);
        AppendLine(stream, L"介质", report.disks[i].mediaType);
        AppendLine(stream, L"序列号", report.disks[i].serialNumber);
        AppendBytesLine(stream, L"容量", report.disks[i].sizeBytes);
    }
    stream << L"\n";

    stream << L"逻辑磁盘\n";
    for (size_t i = 0; i < report.logicalDisks.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"盘符", report.logicalDisks[i].deviceId);
        AppendLine(stream, L"卷标", report.logicalDisks[i].volumeName);
        AppendLine(stream, L"文件系统", report.logicalDisks[i].fileSystem);
        AppendBytesLine(stream, L"总容量", report.logicalDisks[i].sizeBytes);
        AppendBytesLine(stream, L"剩余容量", report.logicalDisks[i].freeBytes);
    }
    stream << L"\n";

    stream << L"主板信息\n";
    for (size_t i = 0; i < report.boards.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"厂商", report.boards[i].manufacturer);
        AppendLine(stream, L"型号", report.boards[i].product);
        AppendLine(stream, L"序列号", report.boards[i].serialNumber);
    }
    stream << L"\n";

    stream << L"BIOS 信息\n";
    for (size_t i = 0; i < report.bios.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"厂商", report.bios[i].manufacturer);
        AppendLine(stream, L"名称", report.bios[i].name);
        AppendLine(stream, L"版本", report.bios[i].version);
        AppendLine(stream, L"序列号", report.bios[i].serialNumber);
        AppendLine(stream, L"发布日期", report.bios[i].releaseDate);
    }
    stream << L"\n";

    stream << L"内存信息\n";
    for (size_t i = 0; i < report.memoryModules.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"Bank", report.memoryModules[i].bankLabel);
        AppendLine(stream, L"插槽", report.memoryModules[i].deviceLocator);
        AppendLine(stream, L"厂商", report.memoryModules[i].manufacturer);
        AppendLine(stream, L"序列号", report.memoryModules[i].serialNumber);
        AppendBytesLine(stream, L"容量", report.memoryModules[i].capacityBytes);
        AppendLine(stream, L"频率MHz", report.memoryModules[i].speedMhz);
    }
    stream << L"\n";

    stream << L"网卡信息\n";
    for (size_t i = 0; i < report.networks.size(); ++i) {
        stream << L"[" << i << L"]\n";
        AppendLine(stream, L"描述", report.networks[i].description);
        AppendLine(stream, L"MAC", report.networks[i].macAddress);
        AppendLine(stream, L"IP", Join(report.networks[i].ipAddresses, L"; "));
        AppendLine(stream, L"网关", Join(report.networks[i].gateways, L"; "));
        AppendLine(stream, L"DNS", Join(report.networks[i].dnsServers, L"; "));
        AppendLine(stream, L"DHCP", report.networks[i].dhcpEnabled ? L"是" : L"否");
    }
    stream << L"\n";

    if (!report.warnings.empty()) {
        stream << L"警告\n";
        for (size_t i = 0; i < report.warnings.size(); ++i) {
            stream << L"- " << report.warnings[i] << L"\n";
        }
    }

    return stream.str();
}

std::wstring RenderJsonReport(const SystemReport& report) {
    std::wostringstream stream;
    stream << L"{\n";
    stream << L"  \"compatibility\": {\n";
    AppendJsonField(stream, L"osCaption", report.compatibility.osCaption, true);
    AppendJsonField(stream, L"osVersion", report.compatibility.osVersion, true);
    AppendJsonField(stream, L"architecture", report.compatibility.architecture, true);
    AppendJsonBool(stream, L"isAdministrator", report.compatibility.isAdministrator, true);
    AppendJsonBool(stream, L"wmiAvailable", report.compatibility.wmiAvailable, false);
    stream << L"  },\n";

    stream << L"  \"system\": {\n";
    AppendJsonField(stream, L"computerName", report.computerName, true);
    AppendJsonField(stream, L"userName", report.userName, true);
    AppendJsonField(stream, L"domainOrWorkgroup", report.domainOrWorkgroup, true);
    AppendJsonField(stream, L"manufacturer", report.manufacturer, true);
    AppendJsonField(stream, L"model", report.model, true);
    AppendJsonField64(stream, L"totalPhysicalMemoryBytes", report.totalPhysicalMemoryBytes, true);
    AppendJsonField(stream, L"bootTime", report.bootTime, false);
    stream << L"  },\n";

    stream << L"  \"cpus\": [\n";
    for (size_t i = 0; i < report.cpus.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"name", report.cpus[i].name, true);
        AppendJsonField(stream, L"manufacturer", report.cpus[i].manufacturer, true);
        AppendJsonField(stream, L"processorId", report.cpus[i].processorId, true);
        AppendJsonField(stream, L"cores", report.cpus[i].cores, true);
        AppendJsonField(stream, L"logicalProcessors", report.cpus[i].logicalProcessors, true);
        AppendJsonField(stream, L"maxClockMhz", report.cpus[i].maxClockMhz, false);
        stream << L"  }" << (i + 1 < report.cpus.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"disks\": [\n";
    for (size_t i = 0; i < report.disks.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"model", report.disks[i].model, true);
        AppendJsonField(stream, L"interfaceType", report.disks[i].interfaceType, true);
        AppendJsonField(stream, L"mediaType", report.disks[i].mediaType, true);
        AppendJsonField(stream, L"serialNumber", report.disks[i].serialNumber, true);
        AppendJsonField64(stream, L"sizeBytes", report.disks[i].sizeBytes, false);
        stream << L"  }" << (i + 1 < report.disks.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"logicalDisks\": [\n";
    for (size_t i = 0; i < report.logicalDisks.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"deviceId", report.logicalDisks[i].deviceId, true);
        AppendJsonField(stream, L"fileSystem", report.logicalDisks[i].fileSystem, true);
        AppendJsonField(stream, L"volumeName", report.logicalDisks[i].volumeName, true);
        AppendJsonField64(stream, L"sizeBytes", report.logicalDisks[i].sizeBytes, true);
        AppendJsonField64(stream, L"freeBytes", report.logicalDisks[i].freeBytes, false);
        stream << L"  }" << (i + 1 < report.logicalDisks.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"boards\": [\n";
    for (size_t i = 0; i < report.boards.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"manufacturer", report.boards[i].manufacturer, true);
        AppendJsonField(stream, L"product", report.boards[i].product, true);
        AppendJsonField(stream, L"serialNumber", report.boards[i].serialNumber, false);
        stream << L"  }" << (i + 1 < report.boards.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"bios\": [\n";
    for (size_t i = 0; i < report.bios.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"manufacturer", report.bios[i].manufacturer, true);
        AppendJsonField(stream, L"name", report.bios[i].name, true);
        AppendJsonField(stream, L"version", report.bios[i].version, true);
        AppendJsonField(stream, L"serialNumber", report.bios[i].serialNumber, true);
        AppendJsonField(stream, L"releaseDate", report.bios[i].releaseDate, false);
        stream << L"  }" << (i + 1 < report.bios.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"memoryModules\": [\n";
    for (size_t i = 0; i < report.memoryModules.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"bankLabel", report.memoryModules[i].bankLabel, true);
        AppendJsonField(stream, L"deviceLocator", report.memoryModules[i].deviceLocator, true);
        AppendJsonField(stream, L"manufacturer", report.memoryModules[i].manufacturer, true);
        AppendJsonField(stream, L"serialNumber", report.memoryModules[i].serialNumber, true);
        AppendJsonField64(stream, L"capacityBytes", report.memoryModules[i].capacityBytes, true);
        AppendJsonField(stream, L"speedMhz", report.memoryModules[i].speedMhz, false);
        stream << L"  }" << (i + 1 < report.memoryModules.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"networks\": [\n";
    for (size_t i = 0; i < report.networks.size(); ++i) {
        stream << L"  {\n";
        AppendJsonField(stream, L"description", report.networks[i].description, true);
        AppendJsonField(stream, L"macAddress", report.networks[i].macAddress, true);
        AppendJsonStringArray(stream, L"ipAddresses", report.networks[i].ipAddresses, true);
        AppendJsonStringArray(stream, L"gateways", report.networks[i].gateways, true);
        AppendJsonStringArray(stream, L"dnsServers", report.networks[i].dnsServers, true);
        AppendJsonBool(stream, L"dhcpEnabled", report.networks[i].dhcpEnabled, false);
        stream << L"  }" << (i + 1 < report.networks.size() ? L"," : L"") << L"\n";
    }
    stream << L"  ],\n";

    stream << L"  \"warnings\": [";
    for (size_t i = 0; i < report.warnings.size(); ++i) {
        if (i > 0) {
            stream << L", ";
        }
        stream << L"\"" << JsonEscape(report.warnings[i]) << L"\"";
    }
    stream << L"]\n";
    stream << L"}\n";
    return stream.str();
}

std::wstring RenderCsvReport(const SystemReport& report) {
    std::wostringstream stream;
    stream << L"Section,Item,Name,Value\n";
    AppendCsvRow(stream, L"Compatibility", L"0", L"OS", report.compatibility.osCaption);
    AppendCsvRow(stream, L"Compatibility", L"0", L"OSVersion", report.compatibility.osVersion);
    AppendCsvRow(stream, L"Compatibility", L"0", L"Architecture", report.compatibility.architecture);
    AppendCsvRow(stream, L"Compatibility", L"0", L"IsAdministrator", report.compatibility.isAdministrator ? L"true" : L"false");
    AppendCsvRow(stream, L"Compatibility", L"0", L"WmiAvailable", report.compatibility.wmiAvailable ? L"true" : L"false");
    AppendCsvRow(stream, L"System", L"0", L"ComputerName", report.computerName);
    AppendCsvRow(stream, L"System", L"0", L"UserName", report.userName);
    AppendCsvRow(stream, L"System", L"0", L"DomainOrWorkgroup", report.domainOrWorkgroup);
    AppendCsvRow(stream, L"System", L"0", L"Manufacturer", report.manufacturer);
    AppendCsvRow(stream, L"System", L"0", L"Model", report.model);
    AppendCsvRow(stream, L"System", L"0", L"TotalPhysicalMemoryBytes", UInt64Text(report.totalPhysicalMemoryBytes));
    AppendCsvRow(stream, L"System", L"0", L"BootTime", report.bootTime);

    for (size_t i = 0; i < report.cpus.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"CPU", item, L"Name", report.cpus[i].name);
        AppendCsvRow(stream, L"CPU", item, L"Manufacturer", report.cpus[i].manufacturer);
        AppendCsvRow(stream, L"CPU", item, L"ProcessorId", report.cpus[i].processorId);
        AppendCsvRow(stream, L"CPU", item, L"Cores", UIntText(report.cpus[i].cores));
        AppendCsvRow(stream, L"CPU", item, L"LogicalProcessors", UIntText(report.cpus[i].logicalProcessors));
        AppendCsvRow(stream, L"CPU", item, L"MaxClockMhz", UIntText(report.cpus[i].maxClockMhz));
    }

    for (size_t i = 0; i < report.disks.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"Disk", item, L"Model", report.disks[i].model);
        AppendCsvRow(stream, L"Disk", item, L"InterfaceType", report.disks[i].interfaceType);
        AppendCsvRow(stream, L"Disk", item, L"MediaType", report.disks[i].mediaType);
        AppendCsvRow(stream, L"Disk", item, L"SerialNumber", report.disks[i].serialNumber);
        AppendCsvRow(stream, L"Disk", item, L"SizeBytes", UInt64Text(report.disks[i].sizeBytes));
    }

    for (size_t i = 0; i < report.logicalDisks.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"LogicalDisk", item, L"DeviceId", report.logicalDisks[i].deviceId);
        AppendCsvRow(stream, L"LogicalDisk", item, L"VolumeName", report.logicalDisks[i].volumeName);
        AppendCsvRow(stream, L"LogicalDisk", item, L"FileSystem", report.logicalDisks[i].fileSystem);
        AppendCsvRow(stream, L"LogicalDisk", item, L"SizeBytes", UInt64Text(report.logicalDisks[i].sizeBytes));
        AppendCsvRow(stream, L"LogicalDisk", item, L"FreeBytes", UInt64Text(report.logicalDisks[i].freeBytes));
    }

    for (size_t i = 0; i < report.boards.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"Board", item, L"Manufacturer", report.boards[i].manufacturer);
        AppendCsvRow(stream, L"Board", item, L"Product", report.boards[i].product);
        AppendCsvRow(stream, L"Board", item, L"SerialNumber", report.boards[i].serialNumber);
    }

    for (size_t i = 0; i < report.bios.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"BIOS", item, L"Manufacturer", report.bios[i].manufacturer);
        AppendCsvRow(stream, L"BIOS", item, L"Name", report.bios[i].name);
        AppendCsvRow(stream, L"BIOS", item, L"Version", report.bios[i].version);
        AppendCsvRow(stream, L"BIOS", item, L"SerialNumber", report.bios[i].serialNumber);
        AppendCsvRow(stream, L"BIOS", item, L"ReleaseDate", report.bios[i].releaseDate);
    }

    for (size_t i = 0; i < report.memoryModules.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"Memory", item, L"BankLabel", report.memoryModules[i].bankLabel);
        AppendCsvRow(stream, L"Memory", item, L"DeviceLocator", report.memoryModules[i].deviceLocator);
        AppendCsvRow(stream, L"Memory", item, L"Manufacturer", report.memoryModules[i].manufacturer);
        AppendCsvRow(stream, L"Memory", item, L"SerialNumber", report.memoryModules[i].serialNumber);
        AppendCsvRow(stream, L"Memory", item, L"CapacityBytes", UInt64Text(report.memoryModules[i].capacityBytes));
        AppendCsvRow(stream, L"Memory", item, L"SpeedMhz", UIntText(report.memoryModules[i].speedMhz));
    }

    for (size_t i = 0; i < report.networks.size(); ++i) {
        const std::wstring item = UInt64Text(static_cast<unsigned long long>(i));
        AppendCsvRow(stream, L"Network", item, L"Description", report.networks[i].description);
        AppendCsvRow(stream, L"Network", item, L"MacAddress", report.networks[i].macAddress);
        AppendCsvRow(stream, L"Network", item, L"IpAddresses", Join(report.networks[i].ipAddresses, L"; "));
        AppendCsvRow(stream, L"Network", item, L"Gateways", Join(report.networks[i].gateways, L"; "));
        AppendCsvRow(stream, L"Network", item, L"DnsServers", Join(report.networks[i].dnsServers, L"; "));
        AppendCsvRow(stream, L"Network", item, L"DhcpEnabled", report.networks[i].dhcpEnabled ? L"true" : L"false");
    }

    for (size_t i = 0; i < report.warnings.size(); ++i) {
        AppendCsvRow(stream, L"Warning", UInt64Text(static_cast<unsigned long long>(i)), L"Message", report.warnings[i]);
    }

    return stream.str();
}

bool WriteUtf8File(const std::wstring& path, const std::wstring& content, std::wstring* error) {
    FILE* file = NULL;
    errno_t openResult = _wfopen_s(&file, path.c_str(), L"wb");
    if (openResult != 0 || !file) {
        if (error) {
            *error = L"无法写入文件: " + path;
        }
        return false;
    }

    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), file);
    const std::string utf8 = WideToUtf8(content);
    if (!utf8.empty()) {
        fwrite(utf8.data(), 1, utf8.size(), file);
    }
    const bool failed = ferror(file) != 0;
    fclose(file);
    if (failed) {
        if (error) {
            *error = L"写入文件失败: " + path;
        }
        return false;
    }
    return true;
}
