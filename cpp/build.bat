@echo off
REM Build M++ C++ interpreter
REM Requires: g++ (MinGW) or cl (MSVC)

set SRC=.
set OUT=mpp.exe

REM Try g++ first
g++ -std=c++17 -O2 -o %OUT% ^
    %SRC%\main.cpp ^
    %SRC%\lexer.cpp ^
    %SRC%\parser.cpp ^
    %SRC%\value.cpp ^
    %SRC%\interpreter.cpp ^
    2>nul

if %ERRORLEVEL% EQU 0 (
    echo Built %OUT% successfully.
    exit /b 0
)

REM Try cl (MSVC)
cl /EHsc /std:c++17 /O2 /Fe:%OUT% ^
    %SRC%\main.cpp ^
    %SRC%\lexer.cpp ^
    %SRC%\parser.cpp ^
    %SRC%\value.cpp ^
    %SRC%\interpreter.cpp ^
    2>nul

if %ERRORLEVEL% EQU 0 (
    echo Built %OUT% successfully.
    del *.obj 2>nul
    exit /b 0
)

echo Error: No C++ compiler found. Install MinGW (g++) or Visual Studio (cl).
exit /b 1
