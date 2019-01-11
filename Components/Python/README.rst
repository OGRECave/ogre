Python for Ogre
===============

This is a work-in-progress (WIP) to develop and automated wheel build process 
for the Python bindings for the Ogre 3D rendering engine.

For instructions on building for Windows, see `BUILD_WIN.txt`.


TODO List
=========

- Copy the structure in `Components/Python` into the build directory provided 
  by CMake.
- Have CMake set environment variables that can be passed into `setup.py`.
- Ensure that `setup.py` has all the files it needs, including possibly 
  default media files?
  