#pragma once

#include <string>
#include <vector>

struct CompatibilityInfo {
    std::wstring osCaption;
    std::wstring osVersion;
    std::wstring architecture;
    bool isAdministrator;
    bool wmiAvailable;
};

struct CpuInfo {
    std::wstring name;
    std::wstring manufacturer;
    std::wstring processorId;
    unsigned int cores;
    unsigned int logicalProcessors;
    unsigned int maxClockMhz;
};

struct DiskInfo {
    std::wstring model;
    std::wstring interfaceType;
    std::wstring mediaType;
    std::wstring serialNumber;
    unsigned long long sizeBytes;
};

struct LogicalDiskInfo {
    std::wstring deviceId;
    std::wstring fileSystem;
    std::wstring volumeName;
    unsigned long long sizeBytes;
    unsigned long long freeBytes;
};

struct BoardInfo {
    std::wstring manufacturer;
    std::wstring product;
    std::wstring serialNumber;
};

struct BiosInfo {
    std::wstring manufacturer;
    std::wstring name;
    std::wstring serialNumber;
    std::wstring version;
    std::wstring releaseDate;
};

struct MemoryInfo {
    std::wstring bankLabel;
    std::wstring deviceLocator;
    std::wstring manufacturer;
    std::wstring serialNumber;
    unsigned long long capacityBytes;
    unsigned int speedMhz;
};

struct NetworkInfo {
    std::wstring description;
    std::wstring macAddress;
    std::vector<std::wstring> ipAddresses;
    std::vector<std::wstring> gateways;
    std::vector<std::wstring> dnsServers;
    bool dhcpEnabled;
};

struct SystemReport {
    CompatibilityInfo compatibility;
    std::wstring computerName;
    std::wstring userName;
    std::wstring domainOrWorkgroup;
    std::wstring manufacturer;
    std::wstring model;
    unsigned long long totalPhysicalMemoryBytes;
    std::wstring bootTime;
    std::vector<CpuInfo> cpus;
    std::vector<DiskInfo> disks;
    std::vector<LogicalDiskInfo> logicalDisks;
    std::vector<BoardInfo> boards;
    std::vector<BiosInfo> bios;
    std::vector<MemoryInfo> memoryModules;
    std::vector<NetworkInfo> networks;
    std::vector<std::wstring> warnings;
};
