@echo off
setlocal

rem Run from a "Developer Command Prompt for VS" so cl.exe is available.
set ROOT=%~dp0..
set OUT=%ROOT%\dist
set OBJ=%ROOT%\build\obj

if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OBJ%" mkdir "%OBJ%"

cl /nologo /EHsc /W4 /O2 /MT /utf-8 /DUNICODE /D_UNICODE ^
  /Fo"%OBJ%\\" ^
  "%ROOT%\src\Main.cpp" ^
  "%ROOT%\src\Args.cpp" ^
  "%ROOT%\src\Collector.cpp" ^
  "%ROOT%\src\Renderer.cpp" ^
  "%ROOT%\src\Utils.cpp" ^
  "%ROOT%\src\WmiClient.cpp" ^
  /link /OUT:"%OUT%\HardwareInfo.exe" /SUBSYSTEM:CONSOLE,6.02 wbemuuid.lib advapi32.lib

if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo Built "%OUT%\HardwareInfo.exe"

cl /nologo /EHsc /W4 /O2 /MT /utf-8 /DUNICODE /D_UNICODE ^
  /Fo"%OBJ%\\" ^
  "%ROOT%\src\GuiMain.cpp" ^
  "%ROOT%\src\Collector.cpp" ^
  "%ROOT%\src\GuiViewModel.cpp" ^
  "%ROOT%\src\Renderer.cpp" ^
  "%ROOT%\src\Utils.cpp" ^
  "%ROOT%\src\WmiClient.cpp" ^
  /link /OUT:"%OUT%\HardwareInfoGUI.exe" /SUBSYSTEM:WINDOWS,6.02 wbemuuid.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib

if errorlevel 1 (
  echo GUI build failed.
  exit /b 1
)

echo Built "%OUT%\HardwareInfoGUI.exe"
