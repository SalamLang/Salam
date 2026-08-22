@echo off
rem Salam installer for Windows.
rem
rem This file is only a launcher: install.ps1 does the real work. When
rem install.ps1 is not sitting next to it - the usual case, someone
rem downloaded install.bat on its own - it is fetched first.
rem
rem Usage:
rem   install.bat
rem   install.bat --dir C:\Tools\Salam
rem   install.bat --version 1.2.3
rem   install.bat --no-modify-path
rem   install.bat --help
rem
rem Every option is passed straight through to install.ps1, so anything
rem that script learns works here the same day.
rem
rem Environment variables:
rem   SALAM_INSTALL_DIR    install directory, default %USERPROFILE%\.salam\bin
rem   SALAM_VERSION        exact version to install
rem   SALAM_NO_MODIFY_PATH set to 1 to leave PATH alone
rem   SALAM_NO_PAUSE       set to 1 to never wait for a key at the end
rem
rem Written without goto, labels or delayed expansion on purpose, so it
rem behaves the same from cmd.exe, from a double click, from PowerShell
rem and from a CI step, on everything from Windows 7 to Windows 11.
rem Parenthesised if blocks group the longer messages; nothing in here
rem reads a variable it set inside the same block, which is the one
rem thing that would need delayed expansion to work.
rem
rem cmd.exe eats exactly one space after "echo", so every message below
rem is written with one more leading space than it prints.

setlocal EnableExtensions

set "SALAM_REPO=SalamLang/Salam"
set "SALAM_PS1_URL=https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.ps1"
set "SALAM_RELEASES=https://github.com/SalamLang/Salam/releases"

rem Help is answered here rather than by install.ps1, so it still works
rem on a machine with no network at all.
set "SALAM_HELP="
if "%~1"=="/?" set "SALAM_HELP=1"
if "%~1"=="-h" set "SALAM_HELP=1"
if /i "%~1"=="--help" set "SALAM_HELP=1"
if /i "%~1"=="help" set "SALAM_HELP=1"

if defined SALAM_HELP (
    echo.
    echo   Salam Programming Language installer for Windows
    echo.
    echo   Usage:
    echo     install.bat [--dir DIR] [--version X.Y.Z] [--no-modify-path]
    echo.
    echo   Options:
    echo     --dir DIR          install into DIR
    echo     --version X.Y.Z    install this exact release instead of the latest
    echo     --no-modify-path   never touch the user PATH
    echo     --help             show this help
    echo.
    echo   Default directory:
    echo     %USERPROFILE%\.salam\bin
    echo.
    echo   Environment:
    echo     SALAM_INSTALL_DIR, SALAM_VERSION, SALAM_NO_MODIFY_PATH, SALAM_NO_PAUSE
    echo.
    echo   This launcher runs install.ps1, downloading it first when it is
    echo   not next to this file. Releases: %SALAM_RELEASES%
    echo.
    exit /b 0
)

echo.
echo   Salam Programming Language installer
echo   https://github.com/%SALAM_REPO%
echo   ------------------------------------

rem A window opened by double clicking closes the instant this script
rem ends, taking the result with it. timeout.exe rather than pause so an
rem automated run can never hang on it: with input redirected it returns
rem immediately, and otherwise it gives up on its own after a minute.
set "SALAM_WAIT="
set "SALAM_CMDLINE=%cmdcmdline%"
set "SALAM_CMDLINE=%SALAM_CMDLINE:"=%"
if not "%SALAM_CMDLINE%"=="%SALAM_CMDLINE:/c =%" set "SALAM_WAIT=1"
if defined SALAM_NO_PAUSE set "SALAM_WAIT="

rem Windows keeps PowerShell at a fixed path; going through PATH first
rem would pick up whatever else happens to be called powershell.exe.
set "SALAM_PS=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
set "SALAM_HAVE_PS="
if exist "%SALAM_PS%" set "SALAM_HAVE_PS=1"

rem Windows 7 and newer all ship PowerShell, but a stripped image or a
rem Server Core install can be missing the in-box copy.
if not defined SALAM_HAVE_PS where powershell.exe >nul 2>nul
if not defined SALAM_HAVE_PS if not errorlevel 1 set "SALAM_PS=powershell.exe"
if not defined SALAM_HAVE_PS if not errorlevel 1 set "SALAM_HAVE_PS=1"

if not defined SALAM_HAVE_PS (
    echo.
    echo   error: Windows PowerShell is not on this machine, and this
    echo          installer needs it to unpack and place the files.
    echo.
    echo   Download a release archive by hand instead:
    echo     %SALAM_RELEASES%
    echo.
    if defined SALAM_WAIT echo   Closing in 60 seconds - press a key to close now.
    if defined SALAM_WAIT if exist "%SystemRoot%\System32\timeout.exe" "%SystemRoot%\System32\timeout.exe" /t 60 >nul 2>nul
    exit /b 1
)

echo.
echo   Preparing:
echo       powershell : %SALAM_PS%

set "SALAM_PS1=%~dp0install.ps1"
set "SALAM_PS1_TEMP="

if not exist "%SALAM_PS1%" set "SALAM_PS1_TEMP=%TEMP%\salam-install-%RANDOM%%RANDOM%.ps1"
if defined SALAM_PS1_TEMP set "SALAM_PS1=%SALAM_PS1_TEMP%"

if not defined SALAM_PS1_TEMP echo       installer  : using the install.ps1 next to this file
if defined SALAM_PS1_TEMP echo       installer  : downloading install.ps1 from raw.githubusercontent.com

rem TLS 1.2 has been mandatory on github.com since 2018, and .NET only
rem picks it by default from 4.7 on. The literal 3072 is Tls12: naming
rem the enum member instead would break on the older runtimes this has
rem to keep working with.
if defined SALAM_PS1_TEMP "%SALAM_PS%" -NoProfile -ExecutionPolicy Bypass -Command "try { [Net.ServicePointManager]::SecurityProtocol = 3072 -bor 768 -bor 192 } catch { }; (New-Object Net.WebClient).DownloadFile('%SALAM_PS1_URL%', '%SALAM_PS1%')" >nul 2>nul

rem certutil goes through WinHTTP, so it still works where PowerShell is
rem too old to negotiate TLS 1.2 at all - the stock Windows 7 case.
if defined SALAM_PS1_TEMP if not exist "%SALAM_PS1%" echo       installer  : PowerShell could not fetch it, trying certutil
if defined SALAM_PS1_TEMP if not exist "%SALAM_PS1%" certutil -urlcache -split -f "%SALAM_PS1_URL%" "%SALAM_PS1%" >nul 2>nul

rem The URL cache keeps a copy of everything it fetches; drop it so a
rem later run cannot silently reuse a stale script.
if defined SALAM_PS1_TEMP if exist "%SALAM_PS1%" certutil -urlcache -f "%SALAM_PS1_URL%" delete >nul 2>nul

rem A captive portal or a proxy error page arrives as a perfectly real
rem file, so check that what landed is the script we asked for. Only the
rem downloaded copy is checked: a local install.ps1 is the user's own.
set "SALAM_PS1_OK="
if not defined SALAM_PS1_TEMP set "SALAM_PS1_OK=1"
if defined SALAM_PS1_TEMP if exist "%SALAM_PS1%" findstr /c:"# Salam installer for Windows." "%SALAM_PS1%" >nul 2>nul
if defined SALAM_PS1_TEMP if exist "%SALAM_PS1%" if not errorlevel 1 set "SALAM_PS1_OK=1"

set "SALAM_PS1_SIZE="
if defined SALAM_PS1_TEMP if defined SALAM_PS1_OK for %%A in ("%SALAM_PS1%") do set "SALAM_PS1_SIZE=%%~zA"
if defined SALAM_PS1_SIZE echo       installer  : got %SALAM_PS1_SIZE% bytes, saved to %SALAM_PS1%

if not defined SALAM_PS1_OK (
    echo.
    echo   error: could not fetch a usable installer script from
    echo          %SALAM_PS1_URL%
    echo.
    echo   Check your internet connection and any proxy settings. If this
    echo   machine sits behind a filtering proxy, download a release
    echo   archive by hand instead:
    echo     %SALAM_RELEASES%
    echo.
    if defined SALAM_PS1_TEMP if exist "%SALAM_PS1_TEMP%" del /f /q "%SALAM_PS1_TEMP%" >nul 2>nul
    if defined SALAM_WAIT echo   Closing in 60 seconds - press a key to close now.
    if defined SALAM_WAIT if exist "%SystemRoot%\System32\timeout.exe" "%SystemRoot%\System32\timeout.exe" /t 60 >nul 2>nul
    exit /b 1
)

echo       starting install.ps1, which reports every step from here on

set "SALAM_RC=1"
"%SALAM_PS%" -NoProfile -ExecutionPolicy Bypass -File "%SALAM_PS1%" %*
set "SALAM_RC=%ERRORLEVEL%"

if defined SALAM_PS1_TEMP if exist "%SALAM_PS1_TEMP%" del /f /q "%SALAM_PS1_TEMP%" >nul 2>nul

if not "%SALAM_RC%"=="0" (
    echo   The installer stopped with exit code %SALAM_RC%.
    echo   If nothing was reported above it, this machine blocks PowerShell
    echo   scripts; take a release archive from %SALAM_RELEASES% instead.
    echo.
)

if defined SALAM_WAIT echo   Closing in 60 seconds - press a key to close now.
if defined SALAM_WAIT if exist "%SystemRoot%\System32\timeout.exe" "%SystemRoot%\System32\timeout.exe" /t 60 >nul 2>nul

exit /b %SALAM_RC%
