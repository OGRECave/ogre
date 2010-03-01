#!/bin/bash
# Call this file from MSYS!

# check if cmake is available
if ! which cmake.exe >/dev/null 2>&1; then
	echo "CMake could not be found. Please ensure that cmake.exe is in your \$PATH."
	exit 1
fi
# check if 7z is available
if ! which 7z.exe >/dev/null 2>&1; then
	echo "7z could not be found. Please ensure that 7z.exe is in your \$PATH."
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
make doc
make install

# get Ogre version
OGRE_VERSION=`cat version.txt`

# copy the BuildSamples* files
cd ../OgreSDK
cp ../../mingw_BuildSamples.bat BuildSamples.bat
cp ../../mingw_BuildSamples.txt BuildSamples.txt

# strip Release DLLs/EXEs
cd bin/release
strip --strip-all {Ogre,Plugin,RenderSystem}*.dll
strip --strip-all OIS.dll
strip --strip-all *.exe

# pack up SDK
cd ../../..
OGRESDK="OgreSDK_mingw_v${OGRE_VERSION}"
mv OgreSDK ${OGRESDK}
# create self-extracting 7zip archive
7z a -r -y -sfx ${OGRESDK}.exe ${OGRESDK}
