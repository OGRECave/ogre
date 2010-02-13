@echo off
if "%1" == "" goto paramErr
if "%VSINSTALLDIR%" == "" goto envErr

rem Determine CMake generator
if "%1" == "vc71" set GENERATOR="Visual Studio 7 .NET 2003"
if "%1" == "vc8" set GENERATOR="Visual Studio 8 2005"
if "%1" == "vc8x64" set GENERATOR="Visual Studio 8 2005 Win64"
if "%1" == "vc9" set GENERATOR="Visual Studio 9 2008"
if "%1" == "vc9x64" set GENERATOR="Visual Studio 9 2008 Win64"
if "%1" == "vc10" set GENERATOR="Visual Studio 10"
if "%1" == "vc10x64" set GENERATOR="Visual Studio 10 Win64"

if %GENERATOR% == "" goto paramErr

set BUILD_DIR=%1

rmdir /Q/S %BUILD_DIR%
mkdir %BUILD_DIR%
pushd %BUILD_DIR%
rem call CMake
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_MEDIA:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -G%GENERATOR% ..\..\..
if errorlevel 1 goto cmakeerror

rem Detect whether we're using full version of VStudio or Express
devenv /?

if errorlevel 1 goto tryexpress
set DEVENV=devenv
goto detecteddevenv
:tryexpress
set DEVENV=VCExpress
:detecteddevenv

%DEVENV% OGRE.sln /build "Debug|Win32"
%DEVENV% OGRE.sln /build "Release|Win32"

rem TODO - go into sdk directory, call CMake again, then zip it up

popd

goto end

:paramErr
echo Required: Build tool (vc71, vc8, vc8x64, vc9, vc9x64, vc10, vc10x64)
set errorlevel=1
goto end

:envErr
echo You need to run this script after running vcvars32.bat
set errorlevel=1
goto end

:cmakeerror
popd
echo CMake not found on your path or CMake error - see above and correct
:end

