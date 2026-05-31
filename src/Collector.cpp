#include "Collector.h"

#include "Utils.h"

#include <lmcons.h>
#include <sstream>
#include <windows.h>

namespace {
std::wstring GetComputerNameText() {
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(buffer, &size)) {
        return buffer;
    }
    return L"";
}

std::wstring GetUserNameText() {
    wchar_t buffer[UNLEN + 1] = {0};
    DWORD size = UNLEN + 1;
    if (GetUserNameW(buffer, &size)) {
        return buffer;
    }
    return L"";
}

std::wstring NormalizeSerial(const std::wstring& value) {
    std::wstring serial = Trim(value);
    if (serial == L"0" || EqualsIgnoreCase(serial, L"None") || EqualsIgnoreCase(serial, L"To be filled by O.E.M.")) {
        return L"";
    }
    return serial;
}

std::wstring BuildBootTime(const std::wstring& wmiDate) {
    if (wmiDate.size() < 14) {
        return wmiDate;
    }
    return wmiDate.substr(0, 4) + L"-" + wmiDate.substr(4, 2) + L"-" + wmiDate.substr(6, 2) +
        L" " + wmiDate.substr(8, 2) + L":" + wmiDate.substr(10, 2) + L":" + wmiDate.substr(12, 2);
}

}

bool IsRunningAsAdministrator() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(
            &ntAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

SystemReport CollectSystemReport(WmiClient& wmi) {
    SystemReport report;
    report.computerName = GetComputerNameText();
    report.userName = GetUserNameText();
    report.compatibility.isAdministrator = IsRunningAsAdministrator();
    report.compatibility.wmiAvailable = wmi.IsInitialized();
    report.compatibility.architecture =
#ifdef _WIN64
        L"x64";
#else
        L"x86";
#endif

    std::wstring error;

    std::vector<std::wstring> osFields;
    osFields.push_back(L"Caption");
    osFields.push_back(L"Version");
    osFields.push_back(L"OSArchitecture");
    osFields.push_back(L"LastBootUpTime");
    std::vector<WmiRow> osRows = wmi.Query(L"SELECT Caption, Version, OSArchitecture, LastBootUpTime FROM Win32_OperatingSystem", osFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    if (!osRows.empty()) {
        report.compatibility.osCaption = FirstValue(osRows[0], L"Caption");
        report.compatibility.osVersion = FirstValue(osRows[0], L"Version");
        const std::wstring osArch = FirstValue(osRows[0], L"OSArchitecture");
        if (!osArch.empty()) {
            report.compatibility.architecture = osArch;
        }
        report.bootTime = BuildBootTime(FirstValue(osRows[0], L"LastBootUpTime"));
    }

    std::vector<std::wstring> csFields;
    csFields.push_back(L"Domain");
    csFields.push_back(L"Manufacturer");
    csFields.push_back(L"Model");
    csFields.push_back(L"TotalPhysicalMemory");
    std::vector<WmiRow> csRows = wmi.Query(L"SELECT Domain, Manufacturer, Model, TotalPhysicalMemory FROM Win32_ComputerSystem", csFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    if (!csRows.empty()) {
        report.domainOrWorkgroup = FirstValue(csRows[0], L"Domain");
        report.manufacturer = FirstValue(csRows[0], L"Manufacturer");
        report.model = FirstValue(csRows[0], L"Model");
        report.totalPhysicalMemoryBytes = FirstUInt64(csRows[0], L"TotalPhysicalMemory");
    }

    std::vector<std::wstring> cpuFields;
    cpuFields.push_back(L"Name");
    cpuFields.push_back(L"Manufacturer");
    cpuFields.push_back(L"ProcessorId");
    cpuFields.push_back(L"NumberOfCores");
    cpuFields.push_back(L"NumberOfLogicalProcessors");
    cpuFields.push_back(L"MaxClockSpeed");
    std::vector<WmiRow> cpuRows = wmi.Query(L"SELECT Name, Manufacturer, ProcessorId, NumberOfCores, NumberOfLogicalProcessors, MaxClockSpeed FROM Win32_Processor", cpuFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < cpuRows.size(); ++i) {
        CpuInfo cpu;
        cpu.name = FirstValue(cpuRows[i], L"Name");
        cpu.manufacturer = FirstValue(cpuRows[i], L"Manufacturer");
        cpu.processorId = FirstValue(cpuRows[i], L"ProcessorId");
        cpu.cores = FirstUInt(cpuRows[i], L"NumberOfCores");
        cpu.logicalProcessors = FirstUInt(cpuRows[i], L"NumberOfLogicalProcessors");
        cpu.maxClockMhz = FirstUInt(cpuRows[i], L"MaxClockSpeed");
        report.cpus.push_back(cpu);
    }

    std::vector<std::wstring> diskFields;
    diskFields.push_back(L"Model");
    diskFields.push_back(L"InterfaceType");
    diskFields.push_back(L"MediaType");
    diskFields.push_back(L"SerialNumber");
    diskFields.push_back(L"Size");
    std::vector<WmiRow> diskRows = wmi.Query(L"SELECT Model, InterfaceType, MediaType, SerialNumber, Size FROM Win32_DiskDrive", diskFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < diskRows.size(); ++i) {
        DiskInfo disk;
        disk.model = FirstValue(diskRows[i], L"Model");
        disk.interfaceType = FirstValue(diskRows[i], L"InterfaceType");
        disk.mediaType = FirstValue(diskRows[i], L"MediaType");
        disk.serialNumber = NormalizeSerial(FirstValue(diskRows[i], L"SerialNumber"));
        disk.sizeBytes = FirstUInt64(diskRows[i], L"Size");
        report.disks.push_back(disk);
    }

    std::vector<std::wstring> physicalFields;
    physicalFields.push_back(L"SerialNumber");
    std::vector<WmiRow> physicalRows = wmi.Query(L"SELECT SerialNumber FROM Win32_PhysicalMedia", physicalFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < report.disks.size() && i < physicalRows.size(); ++i) {
        if (report.disks[i].serialNumber.empty()) {
            report.disks[i].serialNumber = NormalizeSerial(FirstValue(physicalRows[i], L"SerialNumber"));
        }
    }

    std::vector<std::wstring> logicalFields;
    logicalFields.push_back(L"DeviceID");
    logicalFields.push_back(L"FileSystem");
    logicalFields.push_back(L"VolumeName");
    logicalFields.push_back(L"Size");
    logicalFields.push_back(L"FreeSpace");
    std::vector<WmiRow> logicalRows = wmi.Query(L"SELECT DeviceID, FileSystem, VolumeName, Size, FreeSpace FROM Win32_LogicalDisk WHERE DriveType=3", logicalFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < logicalRows.size(); ++i) {
        LogicalDiskInfo disk;
        disk.deviceId = FirstValue(logicalRows[i], L"DeviceID");
        disk.fileSystem = FirstValue(logicalRows[i], L"FileSystem");
        disk.volumeName = FirstValue(logicalRows[i], L"VolumeName");
        disk.sizeBytes = FirstUInt64(logicalRows[i], L"Size");
        disk.freeBytes = FirstUInt64(logicalRows[i], L"FreeSpace");
        report.logicalDisks.push_back(disk);
    }

    std::vector<std::wstring> boardFields;
    boardFields.push_back(L"Manufacturer");
    boardFields.push_back(L"Product");
    boardFields.push_back(L"SerialNumber");
    std::vector<WmiRow> boardRows = wmi.Query(L"SELECT Manufacturer, Product, SerialNumber FROM Win32_BaseBoard", boardFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < boardRows.size(); ++i) {
        BoardInfo board;
        board.manufacturer = FirstValue(boardRows[i], L"Manufacturer");
        board.product = FirstValue(boardRows[i], L"Product");
        board.serialNumber = NormalizeSerial(FirstValue(boardRows[i], L"SerialNumber"));
        report.boards.push_back(board);
    }

    std::vector<std::wstring> biosFields;
    biosFields.push_back(L"Manufacturer");
    biosFields.push_back(L"Name");
    biosFields.push_back(L"SerialNumber");
    biosFields.push_back(L"SMBIOSBIOSVersion");
    biosFields.push_back(L"ReleaseDate");
    std::vector<WmiRow> biosRows = wmi.Query(L"SELECT Manufacturer, Name, SerialNumber, SMBIOSBIOSVersion, ReleaseDate FROM Win32_BIOS", biosFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < biosRows.size(); ++i) {
        BiosInfo bios;
        bios.manufacturer = FirstValue(biosRows[i], L"Manufacturer");
        bios.name = FirstValue(biosRows[i], L"Name");
        bios.serialNumber = NormalizeSerial(FirstValue(biosRows[i], L"SerialNumber"));
        bios.version = FirstValue(biosRows[i], L"SMBIOSBIOSVersion");
        bios.releaseDate = BuildBootTime(FirstValue(biosRows[i], L"ReleaseDate"));
        report.bios.push_back(bios);
    }

    std::vector<std::wstring> memoryFields;
    memoryFields.push_back(L"BankLabel");
    memoryFields.push_back(L"DeviceLocator");
    memoryFields.push_back(L"Manufacturer");
    memoryFields.push_back(L"SerialNumber");
    memoryFields.push_back(L"Capacity");
    memoryFields.push_back(L"Speed");
    std::vector<WmiRow> memoryRows = wmi.Query(L"SELECT BankLabel, DeviceLocator, Manufacturer, SerialNumber, Capacity, Speed FROM Win32_PhysicalMemory", memoryFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < memoryRows.size(); ++i) {
        MemoryInfo memory;
        memory.bankLabel = FirstValue(memoryRows[i], L"BankLabel");
        memory.deviceLocator = FirstValue(memoryRows[i], L"DeviceLocator");
        memory.manufacturer = FirstValue(memoryRows[i], L"Manufacturer");
        memory.serialNumber = NormalizeSerial(FirstValue(memoryRows[i], L"SerialNumber"));
        memory.capacityBytes = FirstUInt64(memoryRows[i], L"Capacity");
        memory.speedMhz = FirstUInt(memoryRows[i], L"Speed");
        report.memoryModules.push_back(memory);
    }

    std::vector<std::wstring> networkFields;
    networkFields.push_back(L"Description");
    networkFields.push_back(L"MACAddress");
    networkFields.push_back(L"IPAddress");
    networkFields.push_back(L"DefaultIPGateway");
    networkFields.push_back(L"DNSServerSearchOrder");
    networkFields.push_back(L"DHCPEnabled");
    std::vector<WmiRow> networkRows = wmi.Query(L"SELECT Description, MACAddress, IPAddress, DefaultIPGateway, DNSServerSearchOrder, DHCPEnabled FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled=True", networkFields, &error);
    if (!error.empty()) {
        report.warnings.push_back(error);
        error.clear();
    }
    for (size_t i = 0; i < networkRows.size(); ++i) {
        NetworkInfo network;
        network.description = FirstValue(networkRows[i], L"Description");
        network.macAddress = FirstValue(networkRows[i], L"MACAddress");
        network.ipAddresses = networkRows[i][L"IPAddress"];
        network.gateways = networkRows[i][L"DefaultIPGateway"];
        network.dnsServers = networkRows[i][L"DNSServerSearchOrder"];
        network.dhcpEnabled = EqualsIgnoreCase(FirstValue(networkRows[i], L"DHCPEnabled"), L"TRUE");
        report.networks.push_back(network);
    }

    if (!report.compatibility.isAdministrator) {
        report.warnings.push_back(L"当前不是管理员权限，部分硬件序列号可能无法读取。");
    }

    for (size_t i = 0; i < report.disks.size(); ++i) {
        if (report.disks[i].serialNumber.empty()) {
            std::wostringstream warning;
            warning << L"硬盘[" << i << L"] 未读取到序列号，可能是权限不足、虚拟机、RAID 控制器或系统未暴露该字段。";
            report.warnings.push_back(warning.str());
        }
    }

    return report;
}
