@echo off
setlocal
rem ---------------------------------------------------------------------------
rem Build Notepad.exe with MSVC.
rem Run this from a "Developer Command Prompt for VS" (cl.exe + rc.exe on PATH).
rem
rem For MinGW-W64 builds use `mingw32-make` instead; on Linux use `make`
rem with the mingw-w64 cross toolchain.
rem ---------------------------------------------------------------------------
where cl >nul 2>nul
if errorlevel 1 (
    echo cl.exe not found. Please run this from a Visual Studio Developer Command Prompt.
    exit /b 1
)

where rc >nul 2>nul
if errorlevel 1 (
    echo rc.exe not found - building without the embedded manifest.
    cl /nologo /W4 /EHsc /O2 /D_CRT_SECURE_NO_WARNINGS main.cpp ^
       /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comdlg32.lib comctl32.lib shell32.lib advapi32.lib
    exit /b %errorlevel%
)

rc /nologo app.rc
if errorlevel 1 exit /b 1

rem /MANIFEST:NO because app.res already carries resource id 1.
cl /nologo /W4 /EHsc /O2 /D_CRT_SECURE_NO_WARNINGS main.cpp app.res ^
   /link /MANIFEST:NO /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comdlg32.lib comctl32.lib shell32.lib advapi32.lib
exit /b %errorlevel%
