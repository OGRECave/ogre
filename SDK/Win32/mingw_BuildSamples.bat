@echo off
rem Specify the path to your CMake installation
set CMAKE=C:\Program Files\CMake 2.8
rem For x64 the following might work for you:
rem set CMAKE=C:\Program Files (x86)\CMake 2.8

rem Specify the path to your MinGW installation
set MINGW=C:\mingw

rem Choose a build type (debug, release);
rem uncomment either of the following:
set BUILD=RelWithDebInfo
rem set BUILD=Release
rem set BUILD=Debug


rem ---------------------------------------------------------
rem Preparing environment...
set PATH=%PATH%;%CMAKE%\bin;%MINGW%\bin
rem Calling CMake...
cmake.exe -G "MinGW Makefiles" . -DCMAKE_BUILD_TYPE="%BUILD%"
rem Compile
mingw32-make.exe

echo 
echo All done.
pause
