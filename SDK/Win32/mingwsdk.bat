@echo off

rem check MinGW make availability
mingw32-make --version > NUL
if errorlevel 1 goto mingwerror
rem check 7z and dot
7z > NUL
if errorlevel 1 goto 7zerror
dot -V > NUL
if errorlevel 1 goto doterror


set BUILD_DIR=mingw
set GENERATOR="MinGW Makefiles"

if "%1" == "clean" rmdir /Q/S %BUILD_DIR%
mkdir %BUILD_DIR%
pushd %BUILD_DIR%

rem Build release binaries and docs
mkdir release
pushd release
rem call CMake
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\..\SDK -DCMAKE_BUILD_TYPE="Release" -G%GENERATOR% ..\..\..\..
if errorlevel 1 goto cmakeerror
rem call twice to ensure all variables are set properly
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\..\SDK -DCMAKE_BUILD_TYPE="Release" -G%GENERATOR% ..\..\..\..
if errorlevel 1 goto cmakeerror
rem Read OGRE version
set /p OGREVERSION=<version.txt

rem build docs explicitly since INSTALL doesn't include it
mingw32-make doc
if errorlevel 1 goto docserror
rem Delete unnecessary doc files
pushd api\html
del /Q/F *.hhk *.hhc *.map *.md5 *.dot *.hhp *.plist
popd

rem Build release binaries
mingw32-make install -j
popd

rem Build debug binaries
mkdir debug
pushd debug
rem call CMake
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\..\SDK -DCMAKE_BUILD_TYPE="Debug" -G%GENERATOR% ..\..\..\..
if errorlevel 1 goto cmakeerror
rem call twice to ensure all variables are set properly
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -DOGRE_INSTALL_DEPENDENCIES:BOOL=TRUE -DCMAKE_INSTALL_PREFIX:PATH=%cd%\..\SDK -DCMAKE_BUILD_TYPE="Debug" -G%GENERATOR% ..\..\..\..
if errorlevel 1 goto cmakeerror
mingw32-make install -j
popd

rem Copy BuildSamples files
pushd SDK
copy ..\..\mingw_BuildSamples.bat BuildSamples.bat
copy ..\..\mingw_BuildSamples.txt BuildSamples.txt

rem strip Release DLLs/EXEs
pushd bin\Release
strip --strip-all Ogre*.dll
strip --strip-all Plugin*.dll
strip --strip-all RenderSystem*.dll
strip --strip-all OIS.dll
strip --strip-all *.exe
popd

popd

popd

rem Package up
set SDKNAME=OgreSDK_MinGW_v%OGREVERSION%
rmdir /S/Q %SDKNAME%
move %BUILD_DIR%\sdk %SDKNAME%
del /Q/F %SDKNAME%.exe
rem create self-extracting 7zip archive
7z a -r -y -sfx7z.sfx %SDKNAME%.exe %SDKNAME%

echo Done! Test %SDKNAME%.exe and then release
goto end

:mingwerror
echo MinGW make not found on your path, please add
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

:docserror
popd
echo Could not create docs - missing doxygen?
goto end

:end

