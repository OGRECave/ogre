#!/bin/bash

OGRE_VERSION="v1.7.0"
# Only build for i386, halves the size
ARCH="i386"
REMOVE_ARCH="ppc"

# invoke xcode build
xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Release
# Just release mode, debug is too big
#xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Debug

rm -rf sdk_contents 
mkdir sdk_contents

# frameworks
echo Copying frameworks...
mkdir sdk_contents/Dependencies

# Stuff we've built
ditto -arch $ARCH ../../Mac/build/Release/Ogre.framework sdk_contents/Dependencies/Ogre.framework

# dependencies
ditto -arch $ARCH ../../Dependencies/Cg.framework sdk_contents/Dependencies/Cg.framework
ditto -arch $ARCH ../../Dependencies/CEGUI.framework sdk_contents/Dependencies/CEGUI.framework

echo Frameworks copied.

# Docs
echo Building API docs...
mkdir sdk_contents/docs

# invoke doxygen
pushd ../../Docs/src
doxygen html.cfg
popd

cp -R ../../Docs/api sdk_contents/docs/
# delete unnecessary files
rm -f sdk_contents/docs/api/html/*.hhk
rm -f sdk_contents/docs/api/html/*.map
rm -f sdk_contents/docs/api/html/*.md5
cp -R ../../Docs/manual sdk_contents/docs/
cp -R ../../Docs/licenses sdk_contents/docs/
cp ReadMe.html sdk_contents/docs/
cp ../../Docs/style.css sdk_contents/docs/
cp -R ../../Docs/ChangeLog.html sdk_contents/docs/
cp -R ../../Docs/*.gif sdk_contents/docs/

echo API generation done.

# do samples
echo Copying samples...
mkdir sdk_contents/Samples

# Copy project location
ditto ../../Mac/Samples sdk_contents/Samples/
# copy source
mkdir sdk_contents/Samples/src
mkdir sdk_contents/Samples/include

find ../../samples -iname *.cpp -exec cp \{\} sdk_contents/Samples/src \;
find ../../samples -iname *.h -exec cp \{\} sdk_contents/Samples/include \;
cp ../../ReferenceApplication/BspCollision/src/*.cpp sdk_contents/Samples/src

# Copy dependencies
mkdir sdk_contents/Dependencies/include
mkdir sdk_contents/Dependencies/lib
#mkdir sdk_contents/Dependencies/lib/Debug
mkdir sdk_contents/Dependencies/lib/Release
cp -R ../../Dependencies/include/OIS sdk_contents/Dependencies/include
#cp ../../Dependencies/lib/Debug/libois.a sdk_contents/Dependencies/lib/Debug/
cp ../../Dependencies/lib/Release/libois.a sdk_contents/Dependencies/lib/Release/

# Fix up project references (2 stage rather than in-place since in-place only works for single sed commands)
sed -f editsamples.sed sdk_contents/Samples/Samples.xcodeproj/project.pbxproj > tmp.xcodeproj
mv tmp.xcodeproj sdk_contents/Samples/Samples.xcodeproj/project.pbxproj
# Fix up architecture
sed -i -e "s/$REMOVE_ARCH,//g" sdk_contents/Samples/Samples.xcodeproj/project.pbxproj

echo Samples copied.

echo Copying Media...

cp -R ../../Samples/Media sdk_contents/Samples/

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
rm OgreSDK_$OGRE_VERSION.dmg
hdiutil convert -format UDBZ  -o OgreSDK_$OGRE_VERSION.dmg template.dmg
rm -rf tmp_dmg
rm template.dmg

echo Done!
