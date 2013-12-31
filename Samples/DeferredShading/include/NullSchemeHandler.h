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

#ifndef _NULLSCHEMEHANDLER_H
#define _NULLSCHEMEHANDLER_H

#include <OgreMaterialManager.h>

/** Class for skipping materials which do not have the scheme defined
 */
class NullSchemeHandler : public Ogre::MaterialManager::Listener
{
public:
	/** @copydoc MaterialManager::Listener::handleSchemeNotFound */
	virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
		const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
		const Ogre::Renderable* rend)
	{
		//Creating a technique so the handler only gets called once per material
		Ogre::Technique* emptyTech = originalMaterial->createTechnique();
		emptyTech->removeAllPasses();
		emptyTech->setSchemeName(schemeName);
		return emptyTech;
	}
};

#endif
