@echo off
setlocal ENABLEDELAYEDEXPANSION
cd /d "%~dp0"

set PORT=8000
set URL=http://localhost:%PORT%

echo ===============================
echo  Local Dev Server Launcher
echo ===============================
echo Folder: %CD%
echo URL:    %URL%
echo.

REM ----------------------------
REM Try Python 3
REM ----------------------------
where python >nul 2>nul
if %ERRORLEVEL%==0 (
    echo [OK] Python found. Starting python http.server...
    echo.
    start "" %URL%
    python -m http.server %PORT%
    goto :EOF
)

REM ----------------------------
REM Try Node.js (npx serve)
REM ----------------------------
where node >nul 2>nul
if %ERRORLEVEL%==0 (
    where npx >nul 2>nul
    if %ERRORLEVEL%==0 (
        echo [OK] Node.js found. Starting npx serve...
        echo.
        start "" %URL%
        npx serve . -l %PORT%
        goto :EOF
    )
)

REM ----------------------------
REM Nothing found
REM ----------------------------
echo [ERROR] No supported server found.
echo.
echo Install one of the following:
echo  - Python 3: https://www.python.org/
echo  - Node.js:  https://nodejs.org/
echo.
pause
exit /b 1
