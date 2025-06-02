@echo off
setlocal

REM Change to the project root directory
cd /d "%~dp0" && cd .. || exit /b 1
echo Current directory: %cd%

REM Default mode is hardware
set MODE=hardware

REM Process arguments
if "%1"=="--hardware" set MODE=hardware & goto :build
if "%1"=="--software" set MODE=software & goto :build
if "%1"=="--help" goto :usage
if "%1"=="-h" goto :usage

if not "%1"=="" (
    if "%1"=="--mode" (
        if "%2"=="hardware" set MODE=hardware & goto :build
        if "%2"=="software" set MODE=software & goto :build
        echo Error: Invalid mode '%2'
        echo Expected: hardware or software
        goto :usage
    ) else (
        echo Error: Invalid argument '%1'
        echo.
        goto :usage
    )
)

:build
echo Running in %MODE% mode

REM Create output directory
if not exist "output" mkdir output

if "%MODE%"=="hardware" (
    echo Building DLL for hardware mode...
    gcc -shared -o ./output/button.dll .\adapter_layer\button_adapter.c ..\bits_button.c
    if errorlevel 1 (
        echo DLL compilation failed! Switching to software mode.
        set MODE=software
    ) else (
        echo DLL compilation success, generate lib to output/!
    )
)

echo Starting simulator in %MODE% mode...
python .\python_simulator\advanced_v2_sim.py --mode %MODE%

exit /b 0

:usage
echo.
echo Usage: %~n0 [options]
echo.
echo Options:
echo   --hardware      Enable hardware mode
echo   --software      Enable software mode (default)
echo   --mode hardware Set hardware mode
echo   --mode software Set software mode
echo   --help          Show this help message
echo   -h              Show this help message
echo.
echo Examples:
echo   %~n0 --hardware
echo   %~n0 --mode hardware
echo   %~n0 --software
echo.
exit /b 1