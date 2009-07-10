#!/bin/bash

# Build and package the SDK for iPhone
# Assumes that you are in the build directory

OGRE_VERSION="v1.7.0"
ARCH="i386"
LIPO=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/lipo

# invoke xcode build for device and simulator
xcodebuild -project ../../build/OGRE.xcodeproj -alltargets -configuration Release -sdk iphoneos3.0
xcodebuild -project ../../build/OGRE.xcodeproj -alltargets -configuration Release -sdk iphonesimulator3.0
# Just release mode, debug is too big
# xcodebuild -project ../../build/OGRE.xcodeproj -alltargets -configuration Debug -sdk iphoneos3.0
# xcodebuild -project ../../build/OGRE.xcodeproj -alltargets -configuration Debug -sdk iphonesimulator3.0

rm -rf sdk_contents 
mkdir sdk_contents

# frameworks
echo Copying frameworks...
mkdir sdk_contents/iPhoneDependencies

# Stuff we've built
# Cram them together so we have a 'fat' library for device and simulator
LIBNAME=libOgreMainStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libRenderSystem_GLESStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libPlugin_OctreeSceneManagerStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libPlugin_BSPSceneManagerStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libPlugin_ParticleFXStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libPlugin_PCZSceneManagerStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libPlugin_OctreeZoneStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libOgrePagingStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

LIBNAME=libOgreTerrainStatic.a
$LIPO ../../build/lib/Release-iphoneos/$LIBNAME -arch i386 ../../build/lib/Release-iphonesimulator/$LIBNAME -create -output sdk_contents/iPhoneDependencies/$LIBNAME

echo Frameworks copied.

# Docs
echo Building API docs...
mkdir sdk_contents/docs

# invoke doxygen
pushd ../Docs/src
doxygen html.cfg
popd

cp -R api sdk_contents/docs/

# delete unnecessary files
rm -f sdk_contents/docs/api/html/*.hhk
rm -f sdk_contents/docs/api/html/*.map
rm -f sdk_contents/docs/api/html/*.md5
cp -R ../Docs/manual sdk_contents/docs/
cp -R ../Docs/licenses sdk_contents/docs/
cp ReadMe.html sdk_contents/docs/
cp ../Docs/style.css sdk_contents/docs/
cp -R ../Docs/ChangeLog.html sdk_contents/docs/
cp -R ../Docs/*.gif sdk_contents/docs/

echo API generation done.

# do samples
echo Copying samples...
mkdir sdk_contents/Samples

# Copy project location
#ditto bin/*.app sdk_contents/Samples/
# copy source
mkdir sdk_contents/Samples/src
mkdir sdk_contents/Samples/include

find ../Samples -iname *.cpp -exec cp \{\} sdk_contents/Samples/src \;
find ../Samples -iname *.h -exec cp \{\} sdk_contents/Samples/include \;
cp ../ReferenceApplication/BspCollision/src/*.cpp sdk_contents/Samples/src

# Copy dependencies
mkdir sdk_contents/iPhoneDependencies/include
mkdir sdk_contents/iPhoneDependencies/lib
#mkdir sdk_contents/iPhoneDependencies/lib/Debug
mkdir sdk_contents/iPhoneDependencies/lib/Release
cp -R ../iPhoneDependencies/include/ sdk_contents/iPhoneDependencies/
#cp ../iPhoneDependencies/lib/Debug/libois.a sdk_contents/iPhoneDependencies/lib/Debug/
cp ../iPhoneDependencies/lib/Release/*.a sdk_contents/iPhoneDependencies/lib/Release/

# Fix up project references (2 stage rather than in-place since in-place only works for single sed commands)
sed -f editsamples.sed sdk_contents/Samples/Samples.xcodeproj/project.pbxproj > tmp.xcodeproj
mv tmp.xcodeproj sdk_contents/Samples/Samples.xcodeproj/project.pbxproj

echo Samples copied.

echo Copying Media...

cp -R ../Samples/Media sdk_contents/Samples/

# Fix up config files
sed -i -e "s/\.\.\/\.\.\/\.\.\/\.\.\/Samples/..\/..\/..\/Samples/g" sdk_contents/samples/config/resources.cfg

echo Media copied.

#remove SVN files to avoid increasing the size of the SDK with duplicates
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
rm OgreSDK_iPhone_$OGRE_VERSION.dmg
hdiutil convert -format UDBZ  -o OgreSDK_iPhone_$OGRE_VERSION.dmg template.dmg
rm -rf tmp_dmg
rm template.dmg

echo Done!
