#include "../src/Args.h"
#include "../src/GuiViewModel.h"
#include "../src/Utils.h"

#include <iostream>
#include <string>

static int failures = 0;

static void AssertTrue(bool condition, const char* message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << std::endl;
    }
}

static void AssertEqual(const std::wstring& actual, const std::wstring& expected, const char* message) {
    if (actual != expected) {
        ++failures;
        std::wcerr << L"FAIL: " << message << L"\n  expected: [" << expected << L"]\n  actual:   [" << actual << L"]" << std::endl;
    }
}

static void TestTrimRemovesOuterWhitespace() {
    AssertEqual(Trim(L" \t abc \r\n"), L"abc", "Trim removes surrounding whitespace");
    AssertEqual(Trim(L" \t\r\n"), L"", "Trim returns empty for whitespace-only strings");
}

static void TestFormatBytesUsesBinaryUnits() {
    AssertEqual(FormatBytes(0), L"0 B", "FormatBytes handles zero");
    AssertEqual(FormatBytes(1024), L"1.00 KB", "FormatBytes handles KB");
    AssertEqual(FormatBytes(1073741824ULL), L"1.00 GB", "FormatBytes handles GB");
}

static void TestCsvEscapeQuotesSpecialFields() {
    AssertEqual(CsvEscape(L"plain"), L"plain", "CsvEscape leaves plain field unchanged");
    AssertEqual(CsvEscape(L"a,b"), L"\"a,b\"", "CsvEscape quotes comma field");
    AssertEqual(CsvEscape(L"a\"b"), L"\"a\"\"b\"", "CsvEscape doubles quotes");
}

static void TestJsonEscapeEscapesControlCharacters() {
    AssertEqual(JsonEscape(L"a\"b\\c"), L"a\\\"b\\\\c", "JsonEscape escapes quote and backslash");
    AssertEqual(JsonEscape(L"a\nb"), L"a\\nb", "JsonEscape escapes newline");
}

static void TestNormalizeNewlinesForWindowsEditUsesCrLf() {
    AssertEqual(NormalizeNewlinesForWindowsEdit(L"a\nb\r\nc\rd"), L"a\r\nb\r\nc\r\nd", "NormalizeNewlinesForWindowsEdit converts all newlines to CRLF");
}

static void TestParseArgsDefaultsToConsole() {
    wchar_t exe[] = L"HardwareInfo.exe";
    wchar_t* argv[] = {exe};
    AppOptions options = ParseArgs(1, argv);

    AssertTrue(options.mode == OutputMode::Console, "ParseArgs defaults to console mode");
    AssertTrue(options.outputPath.empty(), "ParseArgs has no output path by default");
    AssertTrue(options.error.empty(), "ParseArgs has no error by default");
}

static void TestParseArgsAcceptsJsonOutput() {
    wchar_t exe[] = L"HardwareInfo.exe";
    wchar_t flag[] = L"--json";
    wchar_t path[] = L"report.json";
    wchar_t* argv[] = {exe, flag, path};
    AppOptions options = ParseArgs(3, argv);

    AssertTrue(options.mode == OutputMode::JsonFile, "ParseArgs selects JSON mode");
    AssertEqual(options.outputPath, L"report.json", "ParseArgs stores JSON output path");
    AssertTrue(options.error.empty(), "ParseArgs JSON has no error");
}

static void TestParseArgsRejectsMissingPath() {
    wchar_t exe[] = L"HardwareInfo.exe";
    wchar_t flag[] = L"--csv";
    wchar_t* argv[] = {exe, flag};
    AppOptions options = ParseArgs(2, argv);

    AssertTrue(!options.error.empty(), "ParseArgs rejects missing output path");
}

static void TestParseArgsAcceptsNoPause() {
    wchar_t exe[] = L"HardwareInfo.exe";
    wchar_t flag[] = L"--no-pause";
    wchar_t* argv[] = {exe, flag};
    AppOptions options = ParseArgs(2, argv);

    AssertTrue(options.mode == OutputMode::Console, "ParseArgs keeps console mode for no-pause");
    AssertTrue(!options.pauseOnConsoleExit, "ParseArgs disables console pause");
    AssertTrue(options.error.empty(), "ParseArgs no-pause has no error");
}

static void TestBuildDiskLineIncludesModelSizeAndSerial() {
    DiskInfo disk;
    disk.model = L"Samsung SSD";
    disk.interfaceType = L"SCSI";
    disk.serialNumber = L"ABC123";
    disk.sizeBytes = 1073741824ULL;

    AssertEqual(BuildDiskLine(disk), L"Samsung SSD | 1.00 GB | SCSI | ABC123", "BuildDiskLine includes model, size, interface and serial");
}

static void TestBuildSummaryFieldsIncludesCoreFields() {
    SystemReport report;
    report.compatibility.osCaption = L"Windows Server 2012 R2";
    report.computerName = L"SERVER01";
    report.bootTime = L"2026-05-31 10:00:00";
    report.manufacturer = L"Dell";
    report.model = L"PowerEdge";
    report.totalPhysicalMemoryBytes = 17179869184ULL;

    CpuInfo cpu;
    cpu.name = L"Intel Xeon";
    cpu.cores = 8;
    cpu.logicalProcessors = 16;
    report.cpus.push_back(cpu);

    std::vector<GuiField> fields = BuildSummaryFields(report);
    AssertTrue(fields.size() >= 6, "BuildSummaryFields returns core fields");
    AssertEqual(fields[0].label, L"操作系统", "BuildSummaryFields starts with OS label");
    AssertEqual(fields[0].value, L"Windows Server 2012 R2", "BuildSummaryFields starts with OS value");
}

int main() {
    TestTrimRemovesOuterWhitespace();
    TestFormatBytesUsesBinaryUnits();
    TestCsvEscapeQuotesSpecialFields();
    TestJsonEscapeEscapesControlCharacters();
    TestNormalizeNewlinesForWindowsEditUsesCrLf();
    TestParseArgsDefaultsToConsole();
    TestParseArgsAcceptsJsonOutput();
    TestParseArgsRejectsMissingPath();
    TestParseArgsAcceptsNoPause();
    TestBuildDiskLineIncludesModelSizeAndSerial();
    TestBuildSummaryFieldsIncludesCoreFields();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed" << std::endl;
        return 1;
    }

    std::cout << "All tests passed" << std::endl;
    return 0;
}
