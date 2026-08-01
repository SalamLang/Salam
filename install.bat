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
rem tagged with zero assets by a failed build matrix.
rem
rem This is deliberately NOT run through `for /f` with the command inline
rem (`for /f ... in (\`powershell ...\`) do ...`): for /f's backquote-command
rem parser has a much smaller internal buffer than a normal cmd line and
rem silently mangles/truncates long inline commands (confirmed: the exact
rem same call works standalone but breaks under for /f). So the resolver
rem runs directly with output redirected to a file, and only that short
rem file *path* goes through for /f. The script itself is passed via
rem -EncodedCommand (Base64 of UTF-16LE) rather than -Command, since cmd.exe
rem treats |, &, <, > as special even inside double quotes, and REPO/
rem PLATFORM/WORKDIR/VERSION are read from the environment (already real
rem process env vars via `set`) rather than interpolated into the script
rem text, so nothing about this call depends on cmd's quoting behavior.
powershell -NoProfile -NonInteractive -EncodedCommand JABFAHIAcgBvAHIAQQBjAHQAaQBvAG4AUAByAGUAZgBlAHIAZQBuAGMAZQAgAD0AIAAnAFMAdABvAHAAJwAKACQAUAByAG8AZwByAGUAcwBzAFAAcgBlAGYAZQByAGUAbgBjAGUAIAA9ACAAJwBTAGkAbABlAG4AdABsAHkAQwBvAG4AdABpAG4AdQBlACcACgAkAHMAYwByAGkAcAB0ADoAcgBlAHMAdQBsAHQATABpAG4AZQAgAD0AIAAkAG4AdQBsAGwACgBmAHUAbgBjAHQAaQBvAG4AIABUAHIAeQBPAG4AZQAoACQAdgApACAAewAKACAAIAAkAGEAcwBzAGUAdAAgAD0AIAAnAHMAYQBsAGEAbQAtACcAIAArACAAJAB2ACAAKwAgACcALQAnACAAKwAgACQAZQBuAHYAOgBQAEwAQQBUAEYATwBSAE0AIAArACAAJwAuAHoAaQBwACcACgAgACAAJAB1AHIAbAAgAD0AIAAnAGgAdAB0AHAAcwA6AC8ALwBnAGkAdABoAHUAYgAuAGMAbwBtAC8AJwAgACsAIAAkAGUAbgB2ADoAUgBFAFAATwAgACsAIAAnAC8AcgBlAGwAZQBhAHMAZQBzAC8AZABvAHcAbgBsAG8AYQBkAC8AdgAnACAAKwAgACQAdgAgACsAIAAnAC8AJwAgACsAIAAkAGEAcwBzAGUAdAAKACAAIAAkAG8AdQB0ACAAPQAgAEoAbwBpAG4ALQBQAGEAdABoACAAJABlAG4AdgA6AFcATwBSAEsARABJAFIAIAAkAGEAcwBzAGUAdAAKACAAIAB0AHIAeQAgAHsACgAgACAAIAAgAEkAbgB2AG8AawBlAC0AVwBlAGIAUgBlAHEAdQBlAHMAdAAgAC0AVQBzAGUAQgBhAHMAaQBjAFAAYQByAHMAaQBuAGcAIAAtAFUAcgBpACAAJAB1AHIAbAAgAC0ATwB1AHQARgBpAGwAZQAgACQAbwB1AHQAIAAtAEUAcgByAG8AcgBBAGMAdABpAG8AbgAgAFMAdABvAHAACgAgACAAIAAgACQAcwBjAHIAaQBwAHQAOgByAGUAcwB1AGwAdABMAGkAbgBlACAAPQAgACQAdgAgACsAIAAnACwAJwAgACsAIAAkAGEAcwBzAGUAdAAgACsAIAAnACwAJwAgACsAIAAkAG8AdQB0AAoAIAAgACAAIAByAGUAdAB1AHIAbgAgACQAdAByAHUAZQAKACAAIAB9ACAAYwBhAHQAYwBoACAAewAKACAAIAAgACAAcgBlAHQAdQByAG4AIAAkAGYAYQBsAHMAZQAKACAAIAB9AAoAfQAKAGkAZgAgACgAJABlAG4AdgA6AFYARQBSAFMASQBPAE4AIAAtAGEAbgBkACAAJABlAG4AdgA6AFYARQBSAFMASQBPAE4AIAAtAG4AZQAgACcAJwApACAAewAKACAAIABpAGYAIAAoAFQAcgB5AE8AbgBlACAAJABlAG4AdgA6AFYARQBSAFMASQBPAE4AKQAgAHsAIABXAHIAaQB0AGUALQBPAHUAdABwAHUAdAAgACQAcgBlAHMAdQBsAHQATABpAG4AZQA7ACAAZQB4AGkAdAAgADAAIAB9ACAAZQBsAHMAZQAgAHsAIABlAHgAaQB0ACAAMQAgAH0ACgB9AAoAdAByAHkAIAB7AAoAIAAgACQAcgBlAGwAcwAgAD0AIABJAG4AdgBvAGsAZQAtAFIAZQBzAHQATQBlAHQAaABvAGQAIAAtAFUAcwBlAEIAYQBzAGkAYwBQAGEAcgBzAGkAbgBnACAALQBVAHIAaQAgACgAJwBoAHQAdABwAHMAOgAvAC8AYQBwAGkALgBnAGkAdABoAHUAYgAuAGMAbwBtAC8AcgBlAHAAbwBzAC8AJwAgACsAIAAkAGUAbgB2ADoAUgBFAFAATwAgACsAIAAnAC8AcgBlAGwAZQBhAHMAZQBzAD8AcABlAHIAXwBwAGEAZwBlAD0AMQAwACcAKQAKAH0AIABjAGEAdABjAGgAIAB7AAoAIAAgAGUAeABpAHQAIAAxAAoAfQAKAGYAbwByAGUAYQBjAGgAIAAoACQAcgAgAGkAbgAgACQAcgBlAGwAcwApACAAewAKACAAIAAkAHYAZQByACAAPQAgACQAcgAuAHQAYQBnAF8AbgBhAG0AZQAgAC0AcgBlAHAAbABhAGMAZQAgACcAXgB2ACcALAAgACcAJwAKACAAIABpAGYAIAAoAFQAcgB5AE8AbgBlACAAJAB2AGUAcgApACAAewAgAFcAcgBpAHQAZQAtAE8AdQB0AHAAdQB0ACAAJAByAGUAcwB1AGwAdABMAGkAbgBlADsAIABlAHgAaQB0ACAAMAAgAH0ACgB9AAoAZQB4AGkAdAAgADEA > "%WORKDIR%\resolve.txt" 2>nul

set "VERSION="
set "ASSET="
set "ARCHIVE="
for /f "usebackq tokens=1-3 delims=," %%A in ("%WORKDIR%\resolve.txt") do (
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
