@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%run-windows-mouse-companion-renderer-lane-matrix.ps1" %*
exit /b %ERRORLEVEL%
