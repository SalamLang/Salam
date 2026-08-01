@echo off
setlocal EnableDelayedExpansion
rem Salam installer for Windows.
rem
rem Usage:
rem   powershell -c "irm https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.bat -OutFile install.bat"; .\install.bat
rem   install.bat --dir .
rem
rem Env overrides:
rem   SALAM_INSTALL_DIR   directory to place salam.exe in (default: %USERPROFILE%\.salam\bin)
rem   SALAM_VERSION       version to install, e.g. "0.2.7" (default: newest release that ships a
rem                       matching asset - a "latest" release can exist with zero assets if its
rem                       build matrix failed, so this is not simply /releases/latest)
rem
rem All networking, GitHub API/JSON handling and zip extraction is done via PowerShell
rem (bundled with every supported Windows release), so this script has no other dependency.

set "REPO=SalamLang/Salam"
set "INSTALL_DIR=%SALAM_INSTALL_DIR%"
set "VERSION=%SALAM_VERSION%"

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="--dir" (
  set "INSTALL_DIR=%~2"
  shift & shift
  goto parse_args
)
if /I "%~1"=="--version" (
  set "VERSION=%~2"
  shift & shift
  goto parse_args
)
if /I "%~1"=="-h" goto usage
if /I "%~1"=="--help" goto usage
echo error: unknown option: %~1 1>&2
exit /b 1
:usage
echo Usage: install.bat [--dir DIR] [--version X.Y.Z]
exit /b 0
:args_done

rem --- detect architecture -------------------------------------------------
set "ARCH=%PROCESSOR_ARCHITECTURE%"
if /I "%PROCESSOR_ARCHITEW6432%" NEQ "" set "ARCH=%PROCESSOR_ARCHITEW6432%"

if /I "%ARCH%"=="AMD64" (
  set "PLATFORM=windows"
) else if /I "%ARCH%"=="x86" (
  set "PLATFORM=windows-i686"
) else (
  echo error: unsupported architecture: %ARCH% 1>&2
  echo Salam publishes windows ^(x86_64^) and windows-i686 ^(32-bit^) builds only. 1>&2
  exit /b 1
)

set "WORKDIR=%TEMP%\salam-install-%RANDOM%"
mkdir "%WORKDIR%" >nul 2>nul

rem --- resolve version and download (PowerShell: HTTP + JSON + retry loop) --
rem Newest-first walk of releases, downloading the first one that actually
rem has a %PLATFORM% asset - guards against a "latest" release that was
rem tagged with zero assets by a failed build matrix. On success prints
rem "VERSION|ASSET|ARCHIVEPATH" to stdout for the batch script to capture.
set "PS_OUT="
for /f "usebackq tokens=1-3 delims=|" %%A in (`powershell -NoProfile -Command "$ProgressPreference='SilentlyContinue'; $ErrorActionPreference='Stop'; function TryOne($v){ $asset='salam-'+$v+'-%PLATFORM%.zip'; $url='https://github.com/%REPO%/releases/download/v'+$v+'/'+$asset; $out='%WORKDIR%\'+$asset; try { Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $out -ErrorAction Stop; Write-Output ($v+'|'+$asset+'|'+$out); return $true } catch { return $false } }; if ('%VERSION%' -ne '') { if (TryOne '%VERSION%') { exit 0 } else { exit 1 } }; try { $rels = Invoke-RestMethod -UseBasicParsing -Uri 'https://api.github.com/repos/%REPO%/releases?per_page=10' } catch { exit 1 }; foreach ($r in $rels) { $ver = $r.tag_name -replace '^v',''; if (TryOne $ver) { exit 0 } }; exit 1"`) do (
  set "VERSION=%%A"
  set "ASSET=%%B"
  set "ARCHIVE=%%C"
)

if not exist "%ARCHIVE%" (
  echo error: could not find a Salam release publishing a %PLATFORM% asset. 1>&2
  echo Check https://github.com/%REPO%/releases manually. 1>&2
  rd /s /q "%WORKDIR%" >nul 2>nul
  exit /b 1
)

echo Installing Salam %VERSION% (%PLATFORM%) from:
echo   https://github.com/%REPO%/releases/download/v%VERSION%/%ASSET%

set "EXTRACT_DIR=%WORKDIR%\extracted"
powershell -NoProfile -Command "$ProgressPreference='SilentlyContinue'; Expand-Archive -LiteralPath '%ARCHIVE%' -DestinationPath '%EXTRACT_DIR%' -Force"
if %ERRORLEVEL% NEQ 0 (
  echo error: could not extract %ASSET% 1>&2
  rd /s /q "%WORKDIR%" >nul 2>nul
  exit /b 1
)

set "BINARY=%EXTRACT_DIR%\salam-%PLATFORM%\salam.exe"
if not exist "%BINARY%" (
  for /r "%EXTRACT_DIR%" %%F in (salam.exe) do if exist "%%F" set "BINARY=%%F"
)
if not exist "%BINARY%" (
  echo error: could not find salam.exe inside %ASSET% 1>&2
  rd /s /q "%WORKDIR%" >nul 2>nul
  exit /b 1
)

rem --- install ---------------------------------------------------------------
if "%INSTALL_DIR%"=="" set "INSTALL_DIR=%USERPROFILE%\.salam\bin"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%" >nul 2>nul
copy /y "%BINARY%" "%INSTALL_DIR%\salam.exe" >nul

rem Ship the C backend's bundled tcc/ alongside salam.exe when present, since
rem `salam build` (the non-LLVM backend) resolves it relative to the exe.
if exist "%EXTRACT_DIR%\salam-%PLATFORM%\tcc" (
  xcopy /e /i /y /q "%EXTRACT_DIR%\salam-%PLATFORM%\tcc" "%INSTALL_DIR%\tcc" >nul
)

echo Installed: %INSTALL_DIR%\salam.exe
"%INSTALL_DIR%\salam.exe" version 2>nul

rem Only touch the user PATH for the default, non-CI install location; a
rem caller-specified --dir (e.g. a build pipeline dropping .\salam.exe next
rem to a script that expects it there) means "just put it here".
if /I "%INSTALL_DIR%"=="%USERPROFILE%\.salam\bin" (
  echo ";%PATH%;" | find /I ";%INSTALL_DIR%;" >nul
  if errorlevel 1 (
    for /f "usebackq tokens=2,*" %%A in (`reg query "HKCU\Environment" /v Path 2^>nul`) do set "USERPATH=%%B"
    setx PATH "%USERPATH%;%INSTALL_DIR%" >nul
    echo Added %INSTALL_DIR% to your user PATH. Open a new terminal for it to take effect.
  )
)

rd /s /q "%WORKDIR%" >nul 2>nul
echo Done. Run 'salam version' to verify (open a new terminal first if PATH was just updated^).
