@echo off
setlocal enabledelayedexpansion
rem Rebuild compiler\salam from source (self-hosted: compiler\main.salam,
rem compiled by the CURRENT compiler\salam binary). There is no C source
rem anymore -- this is not a from-scratch bootstrap, it always needs a
rem working compiler\salam to start from (the one already tracked in git).
rem Windows cmd.exe equivalent of tools/bash/build-compiler.sh.
rem
rem NOTE: compiler\salam is a Linux ELF binary (built via WSL); on native
rem Windows you need a Windows-built salam.exe on PATH or at
rem compiler\salam.exe instead. Set SALAM_BIN to override.

cd /d "%~dp0..\.."

if "%SALAM_BIN%"=="" (
    if exist "salam.exe" (
        set "SALAM_BIN=salam.exe"
    ) else (
        set "SALAM_BIN=salam"
    )
)

if not exist "%SALAM_BIN%" (
    echo error: %SALAM_BIN% not found -- need a working bootstrap compiler
    echo        to self-host a new one from.
    exit /b 1
)

if "%SALAM_STD%"=="" set "SALAM_STD=%~dp0..\..\.."

echo ^>^> self-hosting: %SALAM_BIN% build main.salam --output=salam.new.exe
"%SALAM_BIN%" build main.salam --output=salam.new.exe
if errorlevel 1 (
    echo error: self-hosted build failed
    del /q salam.new.exe >nul 2>nul
    exit /b 1
)

salam.new.exe version >nul 2>nul
if errorlevel 1 (
    echo error: freshly built salam.new.exe failed "salam version" -- not replacing salam.exe
    del /q salam.new.exe >nul 2>nul
    exit /b 1
)

move /y salam.new.exe salam.exe >nul
echo built compiler\salam.exe (self-hosted)
