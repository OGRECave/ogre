#!/bin/bash

OGRE_VERSION="v1.7.0"
# Only build for i386, halves the size
ARCH="i386"
SDKBUILDDIR=`pwd`

# Clean up files from previous builds
echo Cleaning previous builds...
rm -rf ../../build
rm -rf $SDKBUILDDIR/sdk_contents 
rm OgreSDK_$OGRE_VERSION.dmg

# Configure with CMake
mkdir ../../build
pushd ../../build
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_MEDIA:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -G Xcode ..

echo Building API docs...

# Build docs explicitly since INSTALL doesn't include it
xcodebuild -project OGRE.xcodeproj -target doc -configuration Release -sdk macosx10.4 ARCHS=i386 GCC_VERSION=4.0 MACOSX_DEPLOYMENT_TARGET=10.4

pushd api/html

# Delete unnecessary files
rm -f *.hhk *.hhc *.map *.md5 *.dot *.hhp *.plist ../*.tmp
popd

# Build the Xcode docset and zip it up to save space
#make
#zip -9 -r org.ogre3d.documentation.Reference1_7.docset.zip org.ogre3d.documentation.Reference1_7.docset

# Copy the docset to the disc image.  Disabled to reduce the size of the disc image.
#cp -R api $SDKBUILDDIR/sdk_contents/docs/
#cp org.ogre3d.documentation.Reference1_7.docset.zip $SDKBUILDDIR/sdk_contents/docs/

echo API generation done.

# Invoke Xcode build
xcodebuild -project OGRE.xcodeproj -target install -configuration Release -sdk macosx10.4 ARCHS=i386 GCC_VERSION=4.0 MACOSX_DEPLOYMENT_TARGET=10.4
# Just release mode, debug is too big
#xcodebuild -project OGRE.xcodeproj -target install -configuration Debug -sdk macosx10.4 ARCHS=i386 GCC_VERSION=4.0 MACOSX_DEPLOYMENT_TARGET=10.4

echo Generating Samples Project...

pushd sdk
cmake -DOIS_HOME=../../Dependencies -G Xcode .
rm CMakeCache.txt
rm -rf CMakeFiles
popd

echo End Generating Samples Project

echo Copying SDK...

mkdir $SDKBUILDDIR/sdk_contents
ditto sdk $SDKBUILDDIR/sdk_contents
popd

echo End Copying SDK

# Remove SVN files to avoid increasing the size of the SDK with duplicates
find sdk_contents -iname .svn -exec rm -rf \{\} \;

echo Building DMG...

# Note that our template DMG has already been set up with images, folders and links
# and has already had 'bless -folder blah -openfolder blah' run on it
# to make it auto-open on mounting.

bunzip2 -k -f template.dmg.bz2
mkdir tmp_dmg
hdiutil attach template.dmg -noautoopen -quiet -mountpoint tmp_dmg
ditto sdk_contents tmp_dmg/OgreSDK
hdiutil detach tmp_dmg
hdiutil convert -format UDBZ -o OgreSDK_$OGRE_VERSION.dmg template.dmg
rm -rf tmp_dmg
rm template.dmg

echo Done!
