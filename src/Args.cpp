#include "Args.h"

#include "Utils.h"

#include <sstream>

AppOptions ParseArgs(int argc, wchar_t* argv[]) {
    AppOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::wstring arg = argv[i] ? argv[i] : L"";
        const std::wstring lower = ToLower(arg);

        if (lower == L"--help" || lower == L"-h" || lower == L"/?") {
            options.mode = OutputMode::Help;
            options.showHelp = true;
            return options;
        }

        if (lower == L"--no-pause") {
            options.pauseOnConsoleExit = false;
            continue;
        }

        if (lower == L"--txt" || lower == L"--json" || lower == L"--csv") {
            if (i + 1 >= argc) {
                options.error = L"参数 " + arg + L" 后面需要跟输出文件路径。";
                return options;
            }

            const std::wstring path = argv[++i] ? argv[i] : L"";
            if (Trim(path).empty()) {
                options.error = L"输出文件路径不能为空。";
                return options;
            }

            if (lower == L"--txt") {
                options.mode = OutputMode::TextFile;
            } else if (lower == L"--json") {
                options.mode = OutputMode::JsonFile;
            } else {
                options.mode = OutputMode::CsvFile;
            }
            options.outputPath = path;
            continue;
        }

        options.error = L"未知参数: " + arg;
        return options;
    }

    return options;
}

std::wstring BuildUsageText(const std::wstring& exeName) {
    std::wostringstream stream;
    stream << L"用法:\n";
    stream << L"  " << exeName << L"\n";
    stream << L"  " << exeName << L" --txt report.txt\n";
    stream << L"  " << exeName << L" --json report.json\n";
    stream << L"  " << exeName << L" --csv report.csv\n";
    stream << L"  " << exeName << L" --no-pause\n";
    stream << L"  " << exeName << L" --help\n";
    return stream.str();
}
