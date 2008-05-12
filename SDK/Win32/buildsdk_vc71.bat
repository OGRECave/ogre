@echo off
if "%VSINSTALLDIR%" == "" goto envErr

pushd ..\..
rem clean non-relevant targets to get their stuff out of the way
devenv OgreStatic.sln /clean "DebugStaticLib"
devenv OgreStatic.sln /clean "ReleaseStaticLib"

devenv Ogre.sln /build "Debug"
devenv Ogre.sln /build "Release"

popd

goto end

:envErr
echo You need to run this script after running vcvars32.bat
set errorlevel=1

:end

