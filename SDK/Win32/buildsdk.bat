@echo off
if "%1" == "" goto paramErr
if "%VSINSTALLDIR%" == "" goto envErr

pushd ..\..
rem clean non-relevant targets to get their stuff out of the way
devenv %1 /clean "DebugStaticLib|Win32"
devenv %1 /clean "ReleaseStaticLib|Win32"

devenv %1 /build "Debug|Win32"
devenv %1 /build "Release|Win32"

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

