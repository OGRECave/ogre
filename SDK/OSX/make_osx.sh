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
cd ../../build
cmake -DOGRE_INSTALL_SAMPLES_SOURCE:BOOL=TRUE -DOGRE_INSTALL_MEDIA:BOOL=TRUE -DOGRE_INSTALL_DOCS:BOOL=TRUE -G Xcode ..

# Invoke Xcode build
xcodebuild -project OGRE.xcodeproj -target install -configuration Release -sdk macosx10.4 ARCHS=i386 GCC_VERSION=4.0 MACOSX_DEPLOYMENT_TARGET=10.4
# Just release mode, debug is too big
#xcodebuild -project OGRE.xcodeproj -target install -configuration Debug -sdk macosx10.4 ARCHS=i386 GCC_VERSION=4.0 MACOSX_DEPLOYMENT_TARGET=10.4

# Docs
echo Building API docs...

# Invoke doxygen to generate HTML docs
pushd ../Docs/src
doxygen ../../build/html.cfg
popd

cd api/html

# Delete unnecessary files
rm -f *.hhk *.map *.md5 ../*.tmp

# Build the Xcode docset and zip it up to save space
#make
#zip -9 -r org.ogre3d.documentation.Reference1_7.docset.zip org.ogre3d.documentation.Reference1_7.docset

mkdir $SDKBUILDDIR/sdk_contents
mkdir $SDKBUILDDIR/sdk_contents/docs

# Copy the docset to the disc image.  Disabled to reduce the size of the disc image.
#cp -R api $SDKBUILDDIR/sdk_contents/docs/
#cp org.ogre3d.documentation.Reference1_7.docset.zip $SDKBUILDDIR/sdk_contents/docs/

cd $SDKBUILDDIR

cp -R ../../Docs/manual sdk_contents/docs/
cp -R ../../Docs/licenses sdk_contents/docs/
cp ../../Docs/ReadMe.html sdk_contents/docs/
cp ../../Docs/style.css sdk_contents/docs/
cp -R ../../Docs/ChangeLog.html sdk_contents/docs/
cp -R ../../Docs/*.gif sdk_contents/docs/

echo API generation done.

# Frameworks
echo Copying frameworks...
mkdir sdk_contents/Dependencies

# Stuff we've built
ditto -arch $ARCH ../../build/lib/Release/Ogre.framework sdk_contents/Dependencies/Ogre.framework
ditto -arch $ARCH ../../build/lib/Release/*.dylib sdk_contents/Dependencies/

# Copy dependencies
mkdir sdk_contents/Dependencies/include
mkdir sdk_contents/Dependencies/lib
#mkdir sdk_contents/Dependencies/lib/Debug
mkdir sdk_contents/Dependencies/lib/Release

ditto -arch $ARCH ../../Dependencies/Cg.framework sdk_contents/Dependencies/Cg.framework
cp -R ../../Dependencies/include/* sdk_contents/Dependencies/include/
#cp ../../Dependencies/lib/Debug/*.a sdk_contents/Dependencies/lib/Debug/
ditto -arch $ARCH ../../Dependencies/lib/Release/*.a sdk_contents/Dependencies/lib/Release/

echo Frameworks copied.

# Do samples
echo Copying samples...
mkdir sdk_contents/Samples

# Copy project location
mkdir sdk_contents/Samples/Samples.xcodeproj
cp ../../CMake/Templates/XcodeSamplesOSX.pbxproj.in sdk_contents/Samples/Samples.xcodeproj/project.pbxproj

# Copy source
mkdir sdk_contents/Samples/src
mkdir sdk_contents/Samples/include

find ../../Samples -iname *.cpp -exec cp \{\} sdk_contents/Samples/src \;
find ../../Samples -iname *.mm -exec cp \{\} sdk_contents/Samples/src \;
find ../../Samples -iname *.h -exec cp \{\} sdk_contents/Samples/include \;

echo Samples copied.

echo Copying Media...

cp -R ../../Samples/Media sdk_contents/Samples/

# Copy config files
mkdir sdk_contents/Samples/config/
cp ../../build/bin/*.cfg sdk_contents/Samples/config/

# Fix up config files
sed -i -e "s/\=.*\(Samples\/Media.*\)/\=\1/g" sdk_contents/Samples/config/resources.cfg
sed -i -e "s/\=.*\(Samples\/Media.*\)/\=\1/g" sdk_contents/Samples/config/quakemap.cfg

echo Media copied.

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
