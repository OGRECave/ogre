Instructions for building the Win32 SDK
---------------------------------------

System Path
-----------
You need the following available on your system path:
- CMake 2.8+
- MSVC (run vcvars32.bat from the version of your choice in your command prompt)
- Doxygen

Environment
-----------

The following environment variables must be defined:
- BOOST_ROOT must point to a place where boost::thread and boost::datetime are available & built
- DXSDK_DIR must point to your DirectX SDK of choice (the DirectX SDK installer usually does this for you, but bear in mind it will point to the latest)

Building the SDK
----------------

Run buildsdk.bat with one of the following parameters to pick the compiler of your choice:
 - vc71
 - vc8
 - vc8_x64
 - vc9
 - vc9_x64
 - vc10
 - vc10_x64
 
You can also add "clean" to the parameters to delete any existing build folder (or do it manually).
 
Packaging the SDK
-----------------

Zip up the contents of %compiler%/sdk. 