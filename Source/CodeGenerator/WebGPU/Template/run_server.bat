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
    start "" python -m http.server %PORT%
    call :WAIT_FOR_SERVER
    start "" %URL%
    echo [OK] Server is ready! Browser opened.
    echo Press Ctrl+C or close this window to stop.
    pause >nul
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
        start "" npx serve . -l %PORT%
        call :WAIT_FOR_SERVER
        start "" %URL%
        echo [OK] Server is ready! Browser opened.
        echo Press Ctrl+C or close this window to stop.
        pause >nul
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


REM ----------------------------
REM Subroutine: wait until the
REM server responds on PORT
REM ----------------------------
:WAIT_FOR_SERVER
echo Waiting for server to start...
set TRIES=0
:RETRY
set /a TRIES+=1
if %TRIES% GTR 30 (
    echo [WARN] Server did not respond after 15s, opening browser anyway...
    goto :EOF
)
powershell -NoProfile -Command "try { $r = Invoke-WebRequest -Uri '%URL%' -UseBasicParsing -TimeoutSec 1; exit 0 } catch { exit 1 }" >nul 2>nul
if %ERRORLEVEL%==0 goto :EOF
timeout /t 1 /nobreak >nul
goto :RETRY