Instructions for building the OS X SDK
---------------------------------------

System Path
-----------
You need the following available on your system path:
- CMake 2.8+
- Xcode 2.5
- Doxygen
- Graphviz 'dot' command

Environment
-----------

The following environment variables must be defined:
- Unless Boost is installed globally, BOOST_ROOT must point to a place where boost::thread and boost::datetime are available & built

Building the SDK
----------------

Open a Terminal window and navigate to the OGRESDK/SDK/OSX directory.  Then run ./make_osx.sh . You can optionally supply the parameter "clean" to remove any previous build results.