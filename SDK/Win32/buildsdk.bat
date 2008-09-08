@echo off
if "%1" == "" goto paramErr
if "%VSINSTALLDIR%" == "" goto envErr

rem Detect whether we're using full version of VStudio or Express
devenv /?

if errorlevel 1 goto tryexpress
set DEVENV=devenv
goto detecteddevenv
:tryexpress
set DEVENV=VCExpress
:detecteddevenv

pushd ..\..
rem clean non-relevant targets to get their stuff out of the way
%DEVENV% %1 /clean "DebugStaticLib|Win32"
%DEVENV% %1 /clean "ReleaseStaticLib|Win32"

%DEVENV% %1 /build "Debug|Win32"
%DEVENV% %1 /build "Release|Win32"

popd

goto end

:paramErr
echo Required: Solution file parameter
set errorlevel=1
goto end

:envErr
echo You need to run this script after running vcvars32.bat
set errorlevel=1
goto end

:end

