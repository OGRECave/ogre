 %module(package="Ogre", directors="1") Ogre

%include "OgreSwigCommon.i"

/* ordered by dependency */
%include "OgreSwigCore.i"
%include "OgreSwigResources.i"
%include "OgreSwigMaterials.i"
%include "OgreSwigAnimation.i"
%include "OgreSwigScene.i"

// dont wrap: not useful in high level languages
// %include "OgreTimer.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
