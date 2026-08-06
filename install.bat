rem Salam installer for Windows.
rem
rem Usage:
rem   powershell -c "irm https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.bat -OutFile install.bat"; .\install.bat
rem   install.bat --dir .
rem

@echo off
setlocal EnableDelayedExpansion

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

set "ARCH=%PROCESSOR_ARCHITECTURE%"
if /I "%PROCESSOR_ARCHITEW6432%" NEQ "" set "ARCH=%PROCESSOR_ARCHITEW6432%"

if /I "%ARCH%"=="AMD64" (
  set "PLATFORM=windows"
) else if /I "%ARCH%"=="x86" (
  set "PLATFORM=windows-i686"
) else if /I "%ARCH%"=="ARM64" (
  for /f "tokens=4-5 delims=. " %%a in ('ver') do set "WINVER=%%a.%%b"
  if "!WINVER!"=="10.0" (
    for /f "tokens=3" %%b in ('reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v CurrentBuildNumber 2^>nul') do set "BUILD=%%b"
    if !BUILD! GEQ 22000 (set "PLATFORM=windows") else (set "PLATFORM=windows-i686")
  ) else (
    set "PLATFORM=windows-i686"
  )
) else (
  echo error: unsupported architecture: %ARCH% 1>&2
  echo Salam publishes windows ^(x86_64^) and windows-i686 ^(32-bit^) builds only. 1>&2
  exit /b 1
)

set "WORKDIR=%TEMP%\salam-install-%RANDOM%"
mkdir "%WORKDIR%" >nul 2>nul

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

if "%INSTALL_DIR%"=="" set "INSTALL_DIR=%USERPROFILE%\.salam\bin"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%" >nul 2>nul
copy /y "%BINARY%" "%INSTALL_DIR%\salam.exe" >nul

if exist "%EXTRACT_DIR%\salam-%PLATFORM%\tcc" (
  xcopy /e /i /y /q "%EXTRACT_DIR%\salam-%PLATFORM%\tcc" "%INSTALL_DIR%\tcc" >nul
)

echo Installed: %INSTALL_DIR%\salam.exe
"%INSTALL_DIR%\salam.exe" version 2>nul

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
