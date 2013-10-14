#ifndef __TextureArray_H__
#define __TextureArray_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_TextureArray : public SdkSample
{
public:

	Sample_TextureArray()
	{
		mInfo["Title"] = "Texture Array";
		mInfo["Description"] = "Demonstrates texture array support.";
		mInfo["Thumbnail"] = "thumb_texarray.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
	}

protected:

    StringVector getRequiredPlugins()
	{
		StringVector names;
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsles") && !GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
            names.push_back("Cg Program Manager");
		return names;
	}

    void testCapabilities( const RenderSystemCapabilities* caps )
    {
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and "
                        "fragment programs, so you cannot run this sample. Sorry!",
                        "TextureArray::testCapabilities");
        }

        if (!GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") && 
            !GpuProgramManager::getSingleton().isSyntaxSupported("glsl150") &&
#if OGRE_NO_GLES3_SUPPORT == 0
            !GpuProgramManager::getSingleton().isSyntaxSupported("glsles") &&
#endif
            !GpuProgramManager::getSingleton().isSyntaxSupported("gp4fp"))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model needed for this sample, "
                        "so you cannot run this sample. Sorry!", "TextureArray::testCapabilities");
        }
    }

	void setupContent()
	{
		mSceneMgr->setSkyBox(true, "Examples/TrippySkyBox");

		// set our camera to orbit around the origin and show cursor
		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();

		// the names of the textures we will use (all need to be the same size: 512*512 in our case)
		vector<String>::type texNames;
        texNames.push_back("BeachStones.jpg");
        texNames.push_back("BumpyMetal.jpg");
        texNames.push_back("egyptrockyfull.jpg");
        texNames.push_back("frost.png");
        texNames.push_back("MtlPlat2.jpg");
        texNames.push_back("nskingr.jpg");
        texNames.push_back("Panels_Diffuse.png");
        texNames.push_back("Panels_reflection.png");
        texNames.push_back("RustedMetal.jpg");
        texNames.push_back("spacesky.jpg");
        texNames.push_back("terrain_texture.jpg");
        texNames.push_back("texmap2.jpg");
        texNames.push_back("Water01.jpg");
        texNames.push_back("Water02.jpg");
        texNames.push_back("body.jpg");
        texNames.push_back("stone1.jpg");
        texNames.push_back("wall3.jpg");
        texNames.push_back("sinbad_body.tga");
        texNames.push_back("sinbad_clothes.tga");
        texNames.push_back("stevecube_BK.jpg");

        // create the 2d texture array (the depth is the size of the array - number of textures)
        TexturePtr tex = TextureManager::getSingleton().createManual("TextureArrayTex", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            TEX_TYPE_2D_ARRAY, 
            512, 512, (uint)texNames.size(),
            0, 
            PF_X8R8G8B8 );


        // add all the textures to a 2d texture array
		for (uint32 i = 0; i < static_cast<uint32>(texNames.size()); i++)
		{
            Image terrainTex;
            terrainTex.load(texNames[i], ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            HardwarePixelBufferSharedPtr pixelBufferBuf = tex->getBuffer(0);
            const PixelBox&  currImage = pixelBufferBuf->lock(Box(0,0,i,terrainTex.getHeight(), terrainTex.getHeight(), i+1), HardwareBuffer::HBL_DISCARD);
            PixelUtil::bulkPixelConversion(terrainTex.getPixelBox(), currImage);
            pixelBufferBuf->unlock();
        }

        // create material and set the texture unit to our texture
        MaterialManager& matMgr = MaterialManager::getSingleton();
        MaterialPtr texArrayMat = matMgr.createOrRetrieve("Examples/TextureArray", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first.staticCast<Material>();
        texArrayMat->compile();
        Pass * pass = texArrayMat->getBestTechnique()->getPass(0);
        pass->setLightingEnabled(false);
		TextureUnitState* pState = pass->createTextureUnitState();
        pState->setTextureName(tex->getName(), TEX_TYPE_2D_ARRAY);

        // create a plane with float3 tex coord - the third value will be the texture index in our case
	    ManualObject* textureArrayObject = mSceneMgr->createManualObject("TextureAtlasObject");
        
        // create a quad that uses our material 
        int quadSize = 100;
	    textureArrayObject->begin(texArrayMat->getName(), RenderOperation::OT_TRIANGLE_LIST);
	    // triangle 0 of the quad
	    textureArrayObject->position(0, 0, 0);
    	textureArrayObject->textureCoord(0, 0, 0);
	    textureArrayObject->position(quadSize, 0, 0);
    	textureArrayObject->textureCoord(1, 0, 0);
	    textureArrayObject->position(quadSize, quadSize, 0);
    	textureArrayObject->textureCoord(1, 1, texNames.size());

	    // triangle 1 of the quad
	    textureArrayObject->position(0, 0, 0);
    	textureArrayObject->textureCoord(0, 0, 0); 
	    textureArrayObject->position(quadSize, quadSize, 0); 
    	textureArrayObject->textureCoord(1, 1, texNames.size()); 
	    textureArrayObject->position(0, quadSize, 0);
    	textureArrayObject->textureCoord(0, 1, texNames.size());

	    textureArrayObject->end();

		// attach it to a node and position appropriately
		SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		node->setPosition(-quadSize / 2, -quadSize / 2, 0);
		node->attachObject(textureArrayObject);
	}
    
    void cleanupContent()
    {
        TextureManager::getSingleton().remove("TextureArrayTex");
    }
};

#endif
