/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/


#ifndef _GBUFFERSCHEMEHANDLER_H
#define _GBUFFERSCHEMEHANDLER_H

#include "OgreMaterialManager.h"

/** Class for handling materials who did not specify techniques for rendering
 *  themselves into the GBuffer. This class allows deferred shading to be used,
 *  without having to specify new techniques for all the objects in the scene.
 *  @note This does not support all the possible rendering techniques out there.
 *  in order to support more, either expand this class or specify the techniques
 *  in the materials.
 */
class GBufferSchemeHandler : public Ogre::MaterialManager::Listener
{
public:
    Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex,
        const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
        const Ogre::Renderable* rend) override;
};

#endif
