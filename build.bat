@echo off
vamos smake
if %ERRORLEVEL% equ 0 (
    echo Build successful.
    type nul > run.flag
) else (
    echo Build failed.
)
REM call run.bat
