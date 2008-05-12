# Alter config only if required (timestamp would get updated from sed)
if grep "OGRE_DEBUG_MEMORY_MANAGER 1" ../../OgreMain/include/OgreConfig.h
then
	echo "Disabling memory manager"
	sed -i -f disablememmgr.sed ../../OgreMain/include/OgreConfig.h
fi


