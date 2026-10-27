@echo off
setlocal
where gcc >nul 2>nul
if errorlevel 1 (
  echo GCC was not found.
  echo Install MSYS2 and the UCRT64 GCC package, then add C:\msys64\ucrt64\bin to PATH.
  echo See README.md for exact steps.
  pause
  exit /b 1
)
cd /d "%~dp0"
gcc -std=c11 backend\main.c -o contact_server.exe -lws2_32
if errorlevel 1 (
  echo Build failed. Read the compiler errors above.
  pause
  exit /b 1
)
echo Build successful: contact_server.exe
pause
