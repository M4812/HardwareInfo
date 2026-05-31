#include "Args.h"
#include "Collector.h"
#include "Renderer.h"
#include "Utils.h"
#include "WmiClient.h"

#include <windows.h>

namespace {
void ConfigureConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void WriteText(DWORD handleId, const std::wstring& text) {
    HANDLE handle = GetStdHandle(handleId);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        return;
    }

    DWORD mode = 0;
    if (GetConsoleMode(handle, &mode)) {
        DWORD written = 0;
        WriteConsoleW(handle, text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
        return;
    }

    const std::string utf8 = WideToUtf8(text);
    if (!utf8.empty()) {
        DWORD written = 0;
        WriteFile(handle, utf8.data(), static_cast<DWORD>(utf8.size()), &written, NULL);
    }
}

void WriteOut(const std::wstring& text) {
    WriteText(STD_OUTPUT_HANDLE, text);
}

void WriteErr(const std::wstring& text) {
    WriteText(STD_ERROR_HANDLE, text);
}

bool IsInteractiveConsole(DWORD handleId) {
    HANDLE handle = GetStdHandle(handleId);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        return false;
    }

    DWORD mode = 0;
    return GetConsoleMode(handle, &mode) != 0;
}

void PauseBeforeExitIfNeeded(const AppOptions& options) {
    if (!options.pauseOnConsoleExit || options.mode != OutputMode::Console || !IsInteractiveConsole(STD_INPUT_HANDLE)) {
        return;
    }

    WriteOut(L"\n按回车键退出...");
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    wchar_t buffer[2] = {0};
    DWORD read = 0;
    ReadConsoleW(input, buffer, 1, &read, NULL);
}
}

int wmain(int argc, wchar_t* argv[]) {
    ConfigureConsole();

    AppOptions options = ParseArgs(argc, argv);
    if (!options.error.empty()) {
        WriteErr(options.error + L"\n\n");
        WriteErr(BuildUsageText(argc > 0 ? argv[0] : L"HardwareInfo.exe"));
        return 2;
    }

    if (options.showHelp) {
        WriteOut(BuildUsageText(argc > 0 ? argv[0] : L"HardwareInfo.exe"));
        return 0;
    }

    WmiClient wmi;
    std::wstring wmiError;
    if (!wmi.Initialize(&wmiError)) {
        WriteErr(L"WMI 初始化异常: " + wmiError + L"\n");
        WriteErr(L"程序会继续运行，但硬件信息可能不完整。\n\n");
    }

    SystemReport report = CollectSystemReport(wmi);
    if (!wmiError.empty()) {
        report.warnings.push_back(wmiError);
    }

    std::wstring content;
    if (options.mode == OutputMode::JsonFile) {
        content = RenderJsonReport(report);
    } else if (options.mode == OutputMode::CsvFile) {
        content = RenderCsvReport(report);
    } else {
        content = RenderTextReport(report);
    }

    if (options.mode == OutputMode::Console) {
        WriteOut(content);
        PauseBeforeExitIfNeeded(options);
        return 0;
    }

    std::wstring writeError;
    if (!WriteUtf8File(options.outputPath, content, &writeError)) {
        WriteErr(writeError + L"\n");
        return 1;
    }

    WriteOut(L"已写入: " + options.outputPath + L"\n");
    return 0;
}
