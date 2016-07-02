#!/bin/sh

OGRE_SDK=../../installed_em

# copy together a minimal media subset
cp ../Media/packs/SdkTrays.zip media/
cp ../Media/packs/Sinbad.zip media/
cp -R ../Media/RTShaderLib/GLSLES media/RTShaderLib/

INCLUDES="-I$OGRE_SDK/include/OGRE/Bites -I$OGRE_SDK/include/OGRE/RTShaderSystem -I$OGRE_SDK/include/OGRE/RenderSystems/GLES2 -I$OGRE_SDK/include/OGRE/Overlay -I$OGRE_SDK/include/OGRE"
LIBRARIES="-L$OGRE_SDK/lib/ -lz -lOgreMain -lOgreOverlay -lOgreRTShaderSystem -lOgreBites -lOGRE/RenderSystem_GLES2"

em++ -std=c++11 -O3 Main.cpp Sample.cpp $INCLUDES $LIBRARIES -o release.html --preload-file media/@. -s EXPORTED_FUNCTIONS="['_passAssetAsArrayBuffer', '_clearScene', '_main']" -s ALLOW_MEMORY_GROWTH=1 --closure 1
