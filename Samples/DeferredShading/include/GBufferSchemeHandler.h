/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/


#ifndef _GBUFFERSCHEMEHANDLER_H
#define _GBUFFERSCHEMEHANDLER_H

#include <OgreMaterialManager.h>
#include "GBufferMaterialGenerator.h"

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
	/** @copydoc MaterialManager::Listener::handleSchemeNotFound */
	virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
		const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
		const Ogre::Renderable* rend);
protected:
	//The material generator
	GBufferMaterialGenerator mMaterialGenerator;
	
	//The string that will be checked in textures to determine whether they are normal maps
	static const Ogre::String NORMAL_MAP_PATTERN;

	//A structure for containing the properties of a material, relevant to GBuffer rendering
	//You might need to expand this class to support more options
	struct PassProperties 
	{
		PassProperties() : normalMap(0), isSkinned(false), isTransparent(false) {}
		Ogre::vector<Ogre::TextureUnitState*>::type regularTextures;
		Ogre::TextureUnitState* normalMap;
		bool isSkinned;
        bool hasDiffuseColour;
		bool isTransparent;

		//Example of possible extension : vertex colours
		//Ogre::TrackVertexColourType vertexColourType;
	};

	//Inspect a technique and return its relevant properties
	PassProperties inspectPass(Ogre::Pass* pass, 
		unsigned short lodIndex, const Ogre::Renderable* rend);

	//Get the permutation of material flags that fit a certain property sheet
	MaterialGenerator::Perm getPermutation(const PassProperties& props);

	//Fill a pass with the specific data from the pass it is based on
	void fillPass(Ogre::Pass* gBufferPass, Ogre::Pass* originalPass, const PassProperties& props);

	//Check if a texture is a normal map, and fill property sheet accordingly
	bool checkNormalMap(Ogre::TextureUnitState* tus, PassProperties& props);
};

#endif
