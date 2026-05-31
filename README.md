# HardwareInfo

HardwareInfo 是一个面向内网服务器的离线硬件信息查看工具。它使用原生 Win32/C++ 编写，目标是在 Windows Server 上直接运行，不需要安装 .NET、Go、Java、Python、Node.js 或数据库。

项目提供两个程序：

- `HardwareInfo.exe`：控制台版本，适合命令行查看和导出。
- `HardwareInfoGUI.exe`：图形界面版本，适合双击查看和现场复制信息。

## 特性

- 本机离线采集，不联网，不上传。
- 支持 Windows Server 2012 / 2012 R2 及以上 x64 系统。
- 使用 WMI/COM 读取本机硬件和系统信息。
- 使用 `/MT` 静态链接 C/C++ 运行库，降低目标服务器缺少 VC++ Runtime 导致无法启动的风险。
- 图形界面支持逐项复制、刷新、导出 TXT、导出 JSON。
- 控制台版本支持 TXT、JSON、CSV 导出。

## 可查看的信息

- 操作系统：系统名称、版本、架构、启动时间
- 服务器：主机名、当前用户、域/工作组、厂商、型号
- CPU：型号、厂商、处理器 ID、核心数、线程数、最大频率
- 硬盘：型号、接口、介质、容量、序列号
- 逻辑磁盘：盘符、卷标、文件系统、容量、剩余空间
- 主板：厂商、型号、序列号
- BIOS：厂商、名称、版本、序列号、发布日期
- 内存：插槽、厂商、序列号、容量、频率
- 网卡：描述、MAC、IP、网关、DNS、DHCP
- 兼容性检查：管理员权限、WMI 状态

## 使用方式

### 图形界面

双击运行：

```bat
HardwareInfoGUI.exe
```

图形界面适合现场人员使用。常用字段右侧有“复制”按钮，下方详细信息区会显示完整硬件报告。

### 控制台

```bat
HardwareInfo.exe
HardwareInfo.exe --no-pause
HardwareInfo.exe --txt report.txt
HardwareInfo.exe --json report.json
HardwareInfo.exe --csv report.csv
HardwareInfo.exe --help
```

说明：

- 直接运行 `HardwareInfo.exe` 会在输出后等待回车，避免双击时窗口一闪而过。
- 脚本调用建议使用 `--no-pause`。
- 导出文件使用 UTF-8 with BOM，方便在 Windows 记事本和 Excel 中打开。

## 构建

构建机器需要安装 Visual Studio C++ Build Tools。打开 “Developer Command Prompt for VS”，进入项目目录后执行：

```bat
build\test-msvc.bat
build\build-msvc.bat
```

生成结果：

```text
dist\HardwareInfo.exe
dist\HardwareInfoGUI.exe
```

构建脚本使用的关键参数：

```text
/MT
/SUBSYSTEM:CONSOLE,6.02
/SUBSYSTEM:WINDOWS,6.02
```

其中 `6.02` 对应 Windows 8 / Windows Server 2012 级别，便于兼容 Windows Server 2012 / 2012 R2。

## 运行环境说明

目标服务器不需要安装开发环境或运行时。构建后的 EXE 依赖 Windows 自带系统 DLL。

当前验证中，`HardwareInfoGUI.exe` 的依赖为：

```text
ADVAPI32.dll
USER32.dll
GDI32.dll
COMDLG32.dll
ole32.dll
OLEAUT32.dll
KERNEL32.dll
```

`HardwareInfo.exe` 的依赖为：

```text
ADVAPI32.dll
ole32.dll
OLEAUT32.dll
KERNEL32.dll
```

未依赖：

- .NET Framework / .NET Runtime
- Go Runtime
- Java Runtime
- Python
- Node.js
- VC++ Runtime DLL

## 权限建议

建议右键“以管理员身份运行”。普通用户通常能读取大部分信息，但以下字段在部分服务器上可能需要管理员权限：

- 硬盘序列号
- 主板序列号
- BIOS 序列号
- 部分内存条序列号

## 已知边界

- 虚拟机、云服务器、RAID 控制器或厂商驱动可能不暴露真实硬盘序列号。
- 部分服务器只返回虚拟磁盘序列号，而不是物理硬盘序列号。
- 如果 WMI 服务损坏、禁用或权限不足，硬件信息会不完整。
- 本工具只读取当前机器信息，不做远程扫描。

## 项目结构

```text
src/
  Args.*            控制台参数解析
  Collector.*       WMI 硬件信息采集
  GuiMain.cpp       Win32 图形界面入口
  GuiViewModel.*    GUI 展示字段整理
  Main.cpp          控制台入口
  Models.h          数据模型
  Renderer.*        TXT/JSON/CSV 输出
  Utils.*           字符串、编码、格式化工具
  WmiClient.*       WMI/COM 查询封装
tests/
  TestMain.cpp      轻量级单元测试
build/
  test-msvc.bat     构建并运行测试
  build-msvc.bat    构建正式 EXE
```

## 安全说明

硬件序列号、MAC 地址、主机名等属于资产敏感信息。导出的报告请按内部资产信息管理要求保存和传输。
