@echo off
setlocal

rem Run from a "Developer Command Prompt for VS" so cl.exe is available.
set ROOT=%~dp0..
set OUT=%ROOT%\dist
set OBJ=%ROOT%\build\obj

if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OBJ%" mkdir "%OBJ%"

cl /nologo /EHsc /W4 /Od /MTd /utf-8 /DUNICODE /D_UNICODE ^
  /Fo"%OBJ%\\" ^
  "%ROOT%\tests\TestMain.cpp" ^
  "%ROOT%\src\Args.cpp" ^
  "%ROOT%\src\GuiViewModel.cpp" ^
  "%ROOT%\src\Utils.cpp" ^
  /link /OUT:"%ROOT%\build\HardwareInfo.Tests.exe" /SUBSYSTEM:CONSOLE,6.02

if errorlevel 1 (
  echo Test build failed.
  exit /b 1
)

"%ROOT%\build\HardwareInfo.Tests.exe"
exit /b %errorlevel%
