#!/bin/bash
# Call this file from MSYS!

# check if cmake is available
if ! which cmake.exe >/dev/null 2>&1; then
	echo "CMake could not be found. Please ensure that cmake.exe is in your \$PATH."
	exit 1
fi

# check if OGRE_DEPENDENCIES_DIR is set
if [ -z "${OGRE_DEPENDENCIES_DIR}" ]; then
	echo "OGRE_DEPENDENCIES_DIR is not set."
	echo "You should set this to the path of the Ogre dependency libs."
	echo "Press ENTER to continue, CTRL-C to abort."
	read
fi

# check if BOOST_ROOT is set
if [ -z "${BOOST_ROOT}" ]; then
	echo "BOOST_ROOT is not set."
	echo "You should set this to the path of your Boost installation."
	echo "Press ENTER to continue, CTRL-C to abort."
	read
fi


ROOT_DIR=`pwd`

# prepare output directories
mkdir -p mingw/debug
mkdir -p mingw/release
mkdir -p mingw/OgreSDK

# do Debug build first
cd mingw/debug
cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX="${ROOT_DIR}/mingw/OgreSDK" -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES_SOURCE=TRUE ../../../../
make $@
make install

# now do Release build
cd ../release
cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX="${ROOT_DIR}/mingw/OgreSDK" -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES_SOURCE=TRUE -DOGRE_INSTALL_DOCS=TRUE ../../../../
make $@
# build API docs
#make doc
make install

# for the Samples build, we currently need everything in bin in a unified folder
# this is easier to do manually from here (might change in future)
cd ../OgreSDK/bin
mv release/* .
mv debug/* .
rm -rf release debug

# finally, copy the BuildSamples* files
cd ..
cp ../../mingw_BuildSamples.bat BuildSamples.bat
cp ../../mingw_BuildSamples.txt BuildSamples.txt
