@echo off
if "%1" == "" goto paramErr
if "%VSINSTALLDIR%" == "" goto envErr

set COMPILER=%1

rem Determine CMake generator
if "%COMPILER%" == "vc71" set GENERATOR="Visual Studio 7 .NET 2003"
if "%COMPILER%" == "vc8" set GENERATOR="Visual Studio 8 2005"
if "%COMPILER%" == "vc8_x64" set GENERATOR="Visual Studio 8 2005 Win64"
if "%COMPILER%" == "vc9" set GENERATOR="Visual Studio 9 2008"
if "%COMPILER%" == "vc9_x64" set GENERATOR="Visual Studio 9 2008 Win64"
if "%COMPILER%" == "vc10" set GENERATOR="Visual Studio 10"
if "%COMPILER%" == "vc10_x64" set GENERATOR="Visual Studio 10 Win64"
if "%COMPILER%" == "vc11" set GENERATOR="Visual Studio 11"
if "%COMPILER%" == "vc11_x64" set GENERATOR="Visual Studio 11 Win64"

if %GENERATOR% == "" goto paramErr

rem check 7z and dot
7z > NUL
if errorlevel 1 goto 7zerror
dot -V > NUL
if errorlevel 1 goto doterror


set BUILD_DIR=%COMPILER%

if "%2" == "clean" rmdir /Q/S %BUILD_DIR%
mkdir %BUILD_DIR%
pushd %BUILD_DIR%
rem call CMake
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\SDK -G%GENERATOR% ..\..\..
if errorlevel 1 goto cmakeerror
rem call twice to ensure all variables are set properly
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\SDK -G%GENERATOR% ..\..\..
if errorlevel 1 goto cmakeerror

rem Read OGRE version
set /p OGREVERSION=<version.txt

rem Detect whether we're using full version of VStudio or Express
devenv /? > NUL

if errorlevel 1 goto tryexpress
set DEVENV=devenv
goto detecteddevenv
:tryexpress
set DEVENV=VCExpress
:detecteddevenv

rem build docs explicitly since INSTALL doesn't include it
%DEVENV% OGRE.sln /build "Release" /project "doc"

if errorlevel 1 goto msvcerror

rem Delete unnecessary doc files
pushd api\html
del /Q/F *.hhk *.hhc *.map *.md5 *.dot *.hhp *.plist
popd

rem Build main binaries
%DEVENV% OGRE.sln /build "Debug" /project "INSTALL"
%DEVENV% OGRE.sln /build "Release" /project "INSTALL"

rem call CMake in sdk 
pushd sdk
cmake -DBOOST_INCLUDEDIR:PATH=%cd%\boost -DBOOST_LIBRARYDIR=%cd%\boost\lib -DBoost_NO_SYSTEM_PATHS:BOOL=ON -G%GENERATOR% .\
if errorlevel 1 goto cmakeerror
rem delete cache (since it will include absolute paths)
del CMakeCache.txt
rmdir /S/Q CMakeFiles

rem Patch up absolute references to pdbs & debug directories
rem The former should be fixed in a future version of CMake, but the latter is because we configure these files in manually
dir /b /s *.vcproj *.vcproj.user *.vcxproj *.vcxproj.user  > filestopatch.txt
for /F "delims=" %%f in ('type filestopatch.txt') do (
cscript //nologo ..\..\removeabsolutepaths.vbs "%%f"
)
del /Q/F filestopatch.txt
popd

popd

rem Package up
set SDKNAME=OgreSDK_%COMPILER%_v%OGREVERSION%
rmdir /S/Q %SDKNAME%
move %BUILD_DIR%\sdk %SDKNAME%
del /Q/F %SDKNAME%.exe
rem create self-extracting 7zip archive
7z a -r -y -sfx7z.sfx %SDKNAME%.exe %SDKNAME%

echo Done! Test %SDKNAME%.exe and then release
goto end

:paramErr
echo Required: Build tool (vc71, vc8, vc8x64, vc9, vc9x64, vc10, vc10x64, vc11, vc11x64)
set errorlevel=1
goto end

:envErr
echo You need to run this script after running vcvars32.bat
set errorlevel=1
goto end

:cmakeerror
popd
echo CMake not found on your path or CMake error - see above and correct
goto end

:7zerror
echo 7z.exe not found on your path, please add
goto end

:doterror
echo dot.exe not found on your path, please add
goto end

:msvcerror
popd
echo Neither devenv.exe nor VCExpress are on your path, use vcvars32.bat
goto end

:end

