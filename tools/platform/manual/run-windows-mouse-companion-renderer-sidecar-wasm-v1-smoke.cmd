@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%run-windows-mouse-companion-render-proof.ps1" -Preset renderer-sidecar-wasm-v1-smoke %*
exit /b %ERRORLEVEL%
