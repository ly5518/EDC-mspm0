@echo off
REM SysConfig CLI script for EIDE pre-build
REM Uses --quiet to avoid opening GUI

setlocal

set SYSCONFIG_CLI=D:\ti\sysconfig_1.21.1\sysconfig_cli.bat
set PROJECT_DIR=%~dp0..
set SYSCFG_FILE=%PROJECT_DIR%\empty.syscfg
set OUTPUT_DIR=%PROJECT_DIR%
set SDK_PATH=D:\ti\mspm0_sdk_2_02_00_05\.metadata\product.json

REM Check if files exist
if not exist "%SYSCONFIG_CLI%" (
    echo [SysConfig] ERROR: CLI not found
    exit /b 1
)

if not exist "%SYSCFG_FILE%" (
    echo [SysConfig] ERROR: Config file not found
    exit /b 1
)

echo [SysConfig] Generating config files...

REM Run SysConfig CLI
"%SYSCONFIG_CLI%" --device "MSPM0G350X" --quiet -o "%OUTPUT_DIR%" -s "%SDK_PATH%" "%SYSCFG_FILE%"

if %ERRORLEVEL% == 0 (
    echo [SysConfig] Success: ti_msp_dl_config.c/.h generated
) else (
    echo [SysConfig] Failed! Error code: %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

exit /b 0
