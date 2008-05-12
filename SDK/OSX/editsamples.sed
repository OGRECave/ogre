s/ln -s -f \.\.\/\.\.\/\.\.\/Ogre\.framework/ln -s -f ..\/..\/..\/..\/..\/..\/..\/Dependencies\/Ogre.framework/
s/ln -s -f \.\.\//ln -s -f /g
s/\.\.\/\.\.\/Samples\/.*\/include/include/
s/\.\.\/\.\.\/Samples\/.*\/src/src/
s/\.\.\/\.\.\/Dependencies/..\/Dependencies/
s/\.\.\/Dependencies\/include\/CEGUI/..\/Dependencies\/CEGUI.framework\/Headers/
s/\.\.\/\.\.\/OgreMain\/include/..\/Dependencies\/Ogre.framework\/Headers/
s/path = Ogre\.framework\; sourceTree = BUILT_PRODUCTS_DIR/name = Ogre.framework\; path = ..\/Dependencies\/Ogre.framework\; sourceTree = SOURCE_ROOT/
