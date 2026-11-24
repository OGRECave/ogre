 %module(package="Ogre", directors="1") Ogre

%include "OgreSwigCommon.i"

/* ordered by dependency */
// Python builds each part as a separate module, so they compile in parallel (see Components/Python):
// OGRE_SWIG_PART=<n> selects the part to wrap (numbered in this order), the others are only imported
#if defined(OGRE_SWIG_PART) && OGRE_SWIG_PART != 1
%import "OgreSwigCore.i"
#else
%include "OgreSwigCore.i"
#endif
#if defined(OGRE_SWIG_PART) && OGRE_SWIG_PART != 2
%import "OgreSwigResources.i"
#else
%include "OgreSwigResources.i"
#endif
#if defined(OGRE_SWIG_PART) && OGRE_SWIG_PART != 3
%import "OgreSwigMaterials.i"
#else
%include "OgreSwigMaterials.i"
#endif
#if defined(OGRE_SWIG_PART) && OGRE_SWIG_PART != 4
%import "OgreSwigAnimation.i"
#else
%include "OgreSwigAnimation.i"
#endif
#if defined(OGRE_SWIG_PART) && OGRE_SWIG_PART != 5
%import "OgreSwigScene.i"
#else
%include "OgreSwigScene.i"
#endif

// dont wrap: not useful in high level languages
// %include "OgreTimer.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
