Instructions for building the iPhone SDK
---------------------------------------

System Path
-----------
You need the following available on your system path:
- CMake 2.8+
- Xcode 3.1
- iPhone SDK 3.0 or later
- Doxygen
- Graphviz 'dot' command

Environment
-----------

The following environment variables must be defined:
- Unless Boost is installed globally, BOOST_ROOT must point to a place where boost::thread and boost::datetime are available & built

Building the SDK
----------------

Open a Terminal window and navigate to the OGRESDK/SDK/iPhone directory.  Then run ./make_iphone.sh . You can optionally supply the parameter "clean" to remove any previous build results.