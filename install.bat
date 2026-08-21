@echo off
rem Salam installer for Windows.
rem
rem This file only launches install.ps1, which does the real work. When
rem install.ps1 is not sitting next to it - the usual case, someone
rem fetched install.bat on its own - it is downloaded first.
rem
rem Usage:
rem   install.bat
rem   install.bat --dir C:\Tools\Salam
rem   install.bat --version 1.2.3
rem   install.bat --no-modify-path
rem
rem Environment variables:
rem   SALAM_INSTALL_DIR    install directory (default %USERPROFILE%\.salam\bin)
rem   SALAM_VERSION        exact version to install
rem   SALAM_NO_MODIFY_PATH set to 1 to leave PATH alone
rem   SALAM_NO_PAUSE       set to 1 to never wait for a key at the end
rem
rem Written without goto, labels or delayed expansion on purpose, so it
rem behaves the same from cmd.exe, from a double click, from PowerShell
rem and from a CI step, on everything from Windows 7 to Windows 11.

setlocal EnableExtensions

set "SALAM_PS1_URL=https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.ps1"
set "SALAM_RELEASES=https://github.com/SalamLang/Salam/releases"

rem Windows keeps PowerShell at a fixed path; going through PATH would
rem pick up whatever else happens to be called powershell.exe.
set "SALAM_PS=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%SALAM_PS%" set "SALAM_PS=powershell.exe"

rem A window opened by double clicking closes the instant this script
rem ends, taking the result with it. timeout.exe rather than pause so an
rem automated run can never hang on it: with input redirected it returns
rem immediately, and otherwise it gives up on its own after a minute.
set "SALAM_WAIT="
set "SALAM_CMDLINE=%cmdcmdline%"
set "SALAM_CMDLINE=%SALAM_CMDLINE:"=%"
if not "%SALAM_CMDLINE%"=="%SALAM_CMDLINE:/c =%" set "SALAM_WAIT=1"
if defined SALAM_NO_PAUSE set "SALAM_WAIT="

rem Windows 7 and newer all ship PowerShell, but a stripped image or a
rem Server Core install can be missing it.
set "SALAM_HAVE_PS="
if exist "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" set "SALAM_HAVE_PS=1"
if not defined SALAM_HAVE_PS where powershell.exe >nul 2>nul
if not defined SALAM_HAVE_PS if not errorlevel 1 set "SALAM_HAVE_PS=1"

if not defined SALAM_HAVE_PS echo.
if not defined SALAM_HAVE_PS echo error: Windows PowerShell was not found on this machine, and the
if not defined SALAM_HAVE_PS echo        Salam installer needs it. Download a release archive by hand:
if not defined SALAM_HAVE_PS echo        %SALAM_RELEASES%
if not defined SALAM_HAVE_PS echo.
if not defined SALAM_HAVE_PS if defined SALAM_WAIT if exist "%SystemRoot%\System32\timeout.exe" "%SystemRoot%\System32\timeout.exe" /t 60 >nul 2>nul
if not defined SALAM_HAVE_PS exit /b 1

set "SALAM_PS1=%~dp0install.ps1"
set "SALAM_PS1_TEMP="

if not exist "%SALAM_PS1%" set "SALAM_PS1_TEMP=%TEMP%\salam-install-%RANDOM%%RANDOM%.ps1"
if defined SALAM_PS1_TEMP set "SALAM_PS1=%SALAM_PS1_TEMP%"
if defined SALAM_PS1_TEMP echo Fetching the Salam installer...

rem TLS 1.2 has been mandatory on github.com since 2018, and .NET only
rem picks it by default from 4.7 on. The literal 3072 is Tls12: naming
rem the enum member instead would break on the older runtimes this has
rem to keep working with.
if defined SALAM_PS1_TEMP "%SALAM_PS%" -NoProfile -ExecutionPolicy Bypass -Command "try { [Net.ServicePointManager]::SecurityProtocol = 3072 -bor 768 -bor 192 } catch { }; (New-Object Net.WebClient).DownloadFile('%SALAM_PS1_URL%', '%SALAM_PS1%')" >nul 2>nul

rem certutil goes through WinHTTP, so it still works where PowerShell is
rem too old to negotiate TLS 1.2 at all - the stock Windows 7 case.
if defined SALAM_PS1_TEMP if not exist "%SALAM_PS1%" certutil -urlcache -split -f "%SALAM_PS1_URL%" "%SALAM_PS1%" >nul 2>nul
if defined SALAM_PS1_TEMP if exist "%SALAM_PS1%" certutil -urlcache -f "%SALAM_PS1_URL%" delete >nul 2>nul

set "SALAM_RC=1"

if exist "%SALAM_PS1%" "%SALAM_PS%" -NoProfile -ExecutionPolicy Bypass -File "%SALAM_PS1%" %*
if exist "%SALAM_PS1%" set "SALAM_RC=%ERRORLEVEL%"

if not exist "%SALAM_PS1%" echo.
if not exist "%SALAM_PS1%" echo error: could not download the Salam installer from
if not exist "%SALAM_PS1%" echo        %SALAM_PS1_URL%
if not exist "%SALAM_PS1%" echo.
if not exist "%SALAM_PS1%" echo Check your internet connection and proxy settings, or download a
if not exist "%SALAM_PS1%" echo release archive by hand from %SALAM_RELEASES%
if not exist "%SALAM_PS1%" echo.

if defined SALAM_PS1_TEMP if exist "%SALAM_PS1_TEMP%" del /f /q "%SALAM_PS1_TEMP%" >nul 2>nul

if defined SALAM_WAIT echo Closing in 60 seconds - press a key to close now.
if defined SALAM_WAIT if exist "%SystemRoot%\System32\timeout.exe" "%SystemRoot%\System32\timeout.exe" /t 60 >nul 2>nul

exit /b %SALAM_RC%
