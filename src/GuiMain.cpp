#include "Collector.h"
#include "GuiViewModel.h"
#include "Renderer.h"
#include "Utils.h"
#include "WmiClient.h"

#include <commdlg.h>
#include <sstream>
#include <windows.h>

#pragma comment(lib, "comdlg32.lib")

namespace {
const int IDC_REFRESH = 1001;
const int IDC_EXPORT_TXT = 1002;
const int IDC_EXPORT_JSON = 1003;
const int IDC_EXIT = 1004;
const int IDC_DETAIL = 1005;
const int IDC_COPY_BASE = 3000;

HINSTANCE g_instance = NULL;
HFONT g_font = NULL;
SystemReport g_report;
std::vector<GuiField> g_fields;
std::vector<HWND> g_valueEdits;
std::vector<HWND> g_copyButtons;
HWND g_detailEdit = NULL;
HWND g_statusText = NULL;

std::wstring ValueOrDash(const std::wstring& value) {
    return Trim(value).empty() ? L"未读取到" : value;
}

void SetWindowFont(HWND hwnd) {
    if (g_font && hwnd) {
        SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    }
}

HWND CreateLabel(HWND parent, const std::wstring& text, int x, int y, int w, int h) {
    HWND hwnd = CreateWindowExW(0, L"STATIC", text.c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y, w, h, parent, NULL, g_instance, NULL);
    SetWindowFont(hwnd);
    return hwnd;
}

HWND CreateValueEdit(HWND parent, const std::wstring& text, int x, int y, int w, int h) {
    HWND hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", text.c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
        x, y, w, h, parent, NULL, g_instance, NULL);
    SetWindowFont(hwnd);
    return hwnd;
}

HWND CreateButton(HWND parent, const std::wstring& text, int id, int x, int y, int w, int h) {
    HWND hwnd = CreateWindowExW(0, L"BUTTON", text.c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_instance, NULL);
    SetWindowFont(hwnd);
    return hwnd;
}

void CopyTextToClipboard(HWND hwnd, const std::wstring& text) {
    if (!OpenClipboard(hwnd)) {
        return;
    }
    EmptyClipboard();

    const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (memory) {
        void* target = GlobalLock(memory);
        if (target) {
            CopyMemory(target, text.c_str(), bytes);
            GlobalUnlock(memory);
            SetClipboardData(CF_UNICODETEXT, memory);
            memory = NULL;
        }
    }

    if (memory) {
        GlobalFree(memory);
    }
    CloseClipboard();
}

std::wstring BuildStatusText() {
    std::wostringstream stream;
    stream << L"管理员权限: " << (g_report.compatibility.isAdministrator ? L"是" : L"否")
           << L"    WMI: " << (g_report.compatibility.wmiAvailable ? L"正常" : L"异常");
    if (!g_report.warnings.empty()) {
        stream << L"    警告: " << g_report.warnings.size() << L"条";
    }
    return stream.str();
}

void LoadReport() {
    WmiClient wmi;
    std::wstring error;
    if (!wmi.Initialize(&error)) {
        g_report = SystemReport();
        g_report.compatibility.isAdministrator = IsRunningAsAdministrator();
        g_report.compatibility.wmiAvailable = false;
        g_report.warnings.push_back(error);
    } else {
        g_report = CollectSystemReport(wmi);
    }
    if (!error.empty() && g_report.warnings.empty()) {
        g_report.warnings.push_back(error);
    }
    g_fields = BuildSummaryFields(g_report);
}

void RefreshControls() {
    for (size_t i = 0; i < g_valueEdits.size() && i < g_fields.size(); ++i) {
        SetWindowTextW(g_valueEdits[i], g_fields[i].value.c_str());
    }

    std::wstring detail = NormalizeNewlinesForWindowsEdit(RenderTextReport(g_report));
    SetWindowTextW(g_detailEdit, detail.c_str());
    SetWindowTextW(g_statusText, BuildStatusText().c_str());
}

void CreateSummaryGrid(HWND hwnd) {
    const int left = 22;
    const int top = 16;
    const int colWidth = 390;
    const int rowHeight = 48;
    const int labelHeight = 18;
    const int editHeight = 24;
    const int copyWidth = 56;
    const int editWidth = 306;

    g_fields = BuildSummaryFields(g_report);
    for (size_t i = 0; i < g_fields.size(); ++i) {
        const int col = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        const int x = left + col * colWidth;
        const int y = top + row * rowHeight;

        CreateLabel(hwnd, g_fields[i].label, x, y, editWidth + copyWidth + 8, labelHeight);
        HWND edit = CreateValueEdit(hwnd, g_fields[i].value, x, y + labelHeight, editWidth, editHeight);
        HWND copy = CreateButton(hwnd, L"复制", IDC_COPY_BASE + static_cast<int>(i), x + editWidth + 8, y + labelHeight - 1, copyWidth, editHeight + 2);
        g_valueEdits.push_back(edit);
        g_copyButtons.push_back(copy);
    }
}

std::wstring BuildListText() {
    std::wostringstream stream;
    stream << L"硬盘序列号\n";
    for (size_t i = 0; i < g_report.disks.size(); ++i) {
        stream << i + 1 << L". " << BuildDiskLine(g_report.disks[i]) << L"\r\n";
    }
    if (g_report.disks.empty()) {
        stream << L"未读取到\r\n";
    }

    stream << L"\r\n内存信息\r\n";
    for (size_t i = 0; i < g_report.memoryModules.size(); ++i) {
        stream << i + 1 << L". " << BuildMemoryLine(g_report.memoryModules[i]) << L"\r\n";
    }
    if (g_report.memoryModules.empty()) {
        stream << L"未读取到\r\n";
    }
    return stream.str();
}

void CreateDetailArea(HWND hwnd) {
    CreateLabel(hwnd, L"详细信息", 22, 404, 754, 18);
    g_detailEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
        22, 426, 754, 220, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_DETAIL)), g_instance, NULL);
    SetWindowFont(g_detailEdit);
}

bool SelectSavePath(HWND hwnd, const wchar_t* filter, const wchar_t* defaultName, std::wstring* path) {
    wchar_t fileName[MAX_PATH] = {0};
    lstrcpynW(fileName, defaultName, MAX_PATH);

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileNameW(&ofn)) {
        return false;
    }

    *path = fileName;
    return true;
}

void ExportReport(HWND hwnd, bool json) {
    std::wstring path;
    if (!SelectSavePath(hwnd,
            json ? L"JSON 文件 (*.json)\0*.json\0所有文件 (*.*)\0*.*\0" : L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0",
            json ? L"hardware-info.json" : L"hardware-info.txt",
            &path)) {
        return;
    }

    std::wstring error;
    const std::wstring content = json ? RenderJsonReport(g_report) : RenderTextReport(g_report);
    if (!WriteUtf8File(path, content, &error)) {
        MessageBoxW(hwnd, error.c_str(), L"导出失败", MB_ICONERROR | MB_OK);
        return;
    }
    MessageBoxW(hwnd, L"导出完成。", L"HardwareInfoGUI", MB_ICONINFORMATION | MB_OK);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        LoadReport();
        CreateSummaryGrid(hwnd);
        CreateDetailArea(hwnd);
        g_statusText = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
            22, 656, 754, 20, hwnd, NULL, g_instance, NULL);
        SetWindowFont(g_statusText);
        CreateButton(hwnd, L"刷新", IDC_REFRESH, 180, 686, 104, 34);
        CreateButton(hwnd, L"导出TXT", IDC_EXPORT_TXT, 308, 686, 104, 34);
        CreateButton(hwnd, L"导出JSON", IDC_EXPORT_JSON, 436, 686, 104, 34);
        CreateButton(hwnd, L"退出", IDC_EXIT, 564, 686, 104, 34);
        RefreshControls();
        return 0;

    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        if (id >= IDC_COPY_BASE && id < IDC_COPY_BASE + static_cast<int>(g_fields.size())) {
            CopyTextToClipboard(hwnd, g_fields[id - IDC_COPY_BASE].value);
            return 0;
        }
        if (id == IDC_REFRESH) {
            LoadReport();
            RefreshControls();
            return 0;
        }
        if (id == IDC_EXPORT_TXT) {
            ExportReport(hwnd, false);
            return 0;
        }
        if (id == IDC_EXPORT_JSON) {
            ExportReport(hwnd, true);
            return 0;
        }
        if (id == IDC_EXIT) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    g_instance = instance;
    g_font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    if (!g_font) {
        g_font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    }

    const wchar_t* className = L"HardwareInfoGuiWindow";
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = className;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"窗口注册失败。", L"HardwareInfoGUI", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND hwnd = CreateWindowExW(0, className, L"HardwareInfoGUI v1.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 770,
        NULL, NULL, instance, NULL);

    if (!hwnd) {
        MessageBoxW(NULL, L"窗口创建失败。", L"HardwareInfoGUI", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG message;
    while (GetMessageW(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (g_font && g_font != GetStockObject(DEFAULT_GUI_FONT)) {
        DeleteObject(g_font);
    }
    return static_cast<int>(message.wParam);
}
