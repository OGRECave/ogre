#ifndef __TextureArray_H__
#define __TextureArray_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

//
// fragment program to look up in texture array
//
static const char *fprog_code = 
"!!NVfp4.0                                        \n"
"TEMP texcoord;                                   \n"
"MOV texcoord, fragment.texcoord[0];              \n"
"FLR texcoord.z, texcoord;                        \n"
"TEX result.color, texcoord, texture[0], ARRAY2D; \n"
"END";

//
// interpolate between two nearest layers in array
//
static const char *lerp_fprog_code = 
"!!NVfp4.0                                        \n"
"TEMP texcoord, c0, c1, frac;                     \n"
"MOV texcoord, fragment.texcoord[0];              \n"
"FLR texcoord.z, texcoord;                        \n"
"TEX c0, texcoord, texture[0], ARRAY2D;           \n"
"ADD texcoord.z, texcoord, { 0, 0, 1, 0 };        \n"
"TEX c1, texcoord, texture[0], ARRAY2D;           \n"
"FRC frac.x, fragment.texcoord[0].z;              \n"
"LRP result.color, frac.x, c1, c0;                \n"
"END";


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
            512, 512, texNames.size(), 
            0, 
            PF_X8R8G8B8 );


        // add all the textures to a 2d texture array
		for (size_t i = 0; i < texNames.size(); i++)
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
		MaterialPtr newMat = matMgr.create("TextureArray", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		newMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        Pass * pass = newMat->getTechnique(0)->getPass(0);
		TextureUnitState* pState = pass->createTextureUnitState();
        pState->setTextureName(tex->getName());

        // the the shader (you must use a shader with texture array - this doesn't work with the fixed func)
        GpuProgramPtr fragProg = GpuProgramManager::getSingleton().createProgram("TexArrayFrag", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            "",
            GPT_FRAGMENT_PROGRAM,
            "arbfp1");
        fragProg->setSource(lerp_fprog_code);
        fragProg->load();
        pass->setFragmentProgram(fragProg->getName());

        // create a plain with float3 tex cord - the third value will be the texture index in our case
	    ManualObject* textureArrayObject = mSceneMgr->createManualObject("TextureAtlasObject");
        
        // create a quad that uses our material 
        int quadSize = 100;
	    textureArrayObject->begin(newMat->getName(), RenderOperation::OT_TRIANGLE_LIST);
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
};

#endif
