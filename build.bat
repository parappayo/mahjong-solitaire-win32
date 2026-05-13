@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

where clang >nul 2>nul
if errorlevel 1 (
    echo clang not found in PATH. Install LLVM or MSYS2 mingw-w64 and add bin to PATH.
    exit /b 1
)

where windres >nul 2>nul
if errorlevel 1 (
    echo windres not found in PATH. Add MSYS2 mingw-w64 bin ^(e.g. ucrt64\bin^) to PATH to compile resources.
    exit /b 1
)

windres -I include -o res\app.o res\app.rc
if errorlevel 1 exit /b 1

clang -std=c17 -Wall -Wextra -O2 -Iinclude -c -o src\main.o src\main.c
if errorlevel 1 exit /b 1

clang -std=c17 -Wall -Wextra -O2 -Iinclude -c -o src\load_tiles.o src\load_tiles.c
if errorlevel 1 exit /b 1

clang -o mahjong.exe src\main.o src\load_tiles.o res\app.o -mwindows -lgdiplus -lole32 -luuid
if errorlevel 1 exit /b 1

echo Built mahjong.exe
exit /b 0

rem MSVC linker style, if MinGW flags fail:
rem clang -std=c17 -Wall -Wextra -O2 -Iinclude -c -o src\main.o src\main.c
rem clang -std=c17 -Wall -Wextra -O2 -Iinclude -c -o src\load_tiles.o src\load_tiles.c
rem clang -o mahjong.exe src\main.o src\load_tiles.o res\app.o user32.lib gdi32.lib ole32.lib uuid.lib gdiplus.lib -Xlinker /SUBSYSTEM:WINDOWS
