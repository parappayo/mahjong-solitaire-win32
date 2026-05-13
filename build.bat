@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

where clang >nul 2>nul
if errorlevel 1 (
    echo clang not found in PATH. Install LLVM or MSYS2 mingw-w64 and add bin to PATH.
    exit /b 1
)

rem MinGW-w64 style (LLVM from winlibs, MSYS2 UCRT64, etc.)
clang -std=c17 -Wall -Wextra -O2 -o mahjong.exe src\main.c -mwindows
if errorlevel 1 exit /b 1

echo Built mahjong.exe
exit /b 0

rem MSVC linker style (Visual Studio + LLVM, or clang with lld-link), if MinGW flags fail:
rem clang -std=c17 -Wall -Wextra -O2 -o mahjong.exe src\main.c user32.lib gdi32.lib -Xlinker /SUBSYSTEM:WINDOWS
