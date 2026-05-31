#pragma once

#include <string>

enum class OutputMode {
    Console,
    TextFile,
    JsonFile,
    CsvFile,
    Help
};

struct AppOptions {
    OutputMode mode;
    std::wstring outputPath;
    bool showHelp;
    bool pauseOnConsoleExit;
    std::wstring error;

    AppOptions() : mode(OutputMode::Console), showHelp(false), pauseOnConsoleExit(true) {}
};

AppOptions ParseArgs(int argc, wchar_t* argv[]);
std::wstring BuildUsageText(const std::wstring& exeName);
