@echo off
cd /d "%~dp0"
if not exist contact_server.exe (
  echo Server executable not found. Run build.bat first.
  pause
  exit /b 1
)
contact_server.exe
pause
