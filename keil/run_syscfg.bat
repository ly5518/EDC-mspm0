@echo off
setlocal

set "PROJ_DIR=%~dp0"
set "SYSCFG_FILE=%~1"

set "TOOLS_DIR=%PROJ_DIR%..\..\tools\keil"

call "%TOOLS_DIR%\syscfg.bat" "%PROJ_DIR%" "%SYSCFG_FILE%"
