#ifndef __ShaderSystemPBR_H__
#define __ShaderSystemPBR_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_ShaderSystemPBR : public SdkSample
{
public:

	Sample_ShaderSystemPBR()
    {
        mInfo["Title"] = "ShaderSystem - PBR";
        mInfo["Description"] = "Shows how to use PBR with the RTSS.";
        mInfo["Thumbnail"] = "thumb_hlms.png";
        mInfo["Category"] = "Lighting";
    }

private:
	std::vector<MaterialPtr> mMaterialList;
	Ogre::SceneNode* mLightNodes[3];
	Ogre::MeshPtr mFloor;

	RTShader::PerPixelLighting* mLightingState;

	const char* NORMALISED_BUTTON = "NORMALISED";
	const char* FRESNEL_BUTTON = "FRESNEL";
protected:

	void cleanupContent()
	{
		for (size_t i = 0; i < mMaterialList.size(); ++i)
		{
			mMaterialList[i]->unload();
			MaterialManager::getSingleton().remove(mMaterialList[i]);
		}
		mMaterialList.clear();
		mFloor->unload();
		MeshManager::getSingleton().remove(mFloor);
	}

    void checkBoxToggled(CheckBox* box)
    {
        return;
        if(box->getName() == NORMALISED_BUTTON)
            mLightingState->setNormaliseEnabled(box->isChecked());

        RTShader::ShaderGenerator::getSingleton().invalidateScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
    }

    void setupContent()
    {
        //auto schemRenderState =
        //    mShaderGenerator->getRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        //mLightingState = mShaderGenerator->createSubRenderState<RTShader::PerPixelLighting>();

        mTrayMgr->createCheckBox(TL_BOTTOM, NORMALISED_BUTTON, "Normalised")->setChecked(false);
        //mTrayMgr->createCheckBox(TL_BOTTOM, FRESNEL_BUTTON, "Fresnel")->setChecked(false);

        //schemRenderState->addTemplateSubRenderState(mLightingState);

        // Make this viewport work with shader generator scheme.
        mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        // setup some basic lighting for our scene
		Ogre::SceneNode *rootNode = mSceneMgr->getRootSceneNode();

		Ogre::Light *light = mSceneMgr->createLight();
		Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
		/*lightNode->attachObject(light);
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
		mLightNodes[0] = lightNode;

		light = mSceneMgr->createLight();
		lightNode = rootNode->createChildSceneNode();*/
		lightNode->attachObject(light);
		light->setDiffuseColour(0.8f, 0.4f, 0.2f); //Warm
		light->setSpecularColour(0.8f, 0.4f, 0.2f);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(-10.0f, 10.0f, 10.0f);
		light->setDirection(Ogre::Vector3(1, -1, -1).normalisedCopy());
		mLightNodes[1] = lightNode;

		/*light = mSceneMgr->createLight();
		lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setDiffuseColour(0.2f, 0.4f, 0.8f); //Cold
		light->setSpecularColour(0.2f, 0.4f, 0.8f);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(10.0f, 10.0f, -10.0f);
		light->setDirection(Ogre::Vector3(-1, -1, 1).normalisedCopy());
		mLightNodes[2] = lightNode;*/

		// create a floor mesh resource
		mFloor = MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, 0), 30, 30, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

		// create a floor entity, give it a material, and place it at the origin
		Entity* floor = mSceneMgr->createEntity(mFloor);
		floor->setMaterialName("Examples/Rockwall");
		floor->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->attachObject(floor);

		const int numX = 8;
		const int numZ = 8;

		const float armsLength = 1.0f;
		const float startX = (numX - 1) / 2.0f;
		const float startZ = (numZ - 1) / 2.0f;

		auto mat = MaterialManager::getSingleton().create("Base", RGN_DEFAULT);
		mat->setDiffuse(ColourValue::Green);
		mMaterialList.push_back(mat);

		for (int x = 0; x<numX; ++x)
		{
			for (int z = 0; z<numZ; ++z)
			{
				Entity *ent = mSceneMgr->createEntity("Sphere1000.mesh");
				SceneNode *sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				sceneNode->setPosition(Vector3(armsLength * x - startX,	1.0f, armsLength * z - startZ));
				sceneNode->attachObject(ent);

				float shininess = float(x + 1)/(numX)*128;
				float intensity = 0.1 + float(z)/(numZ-1)*0.9;
				auto clone = mat->clone(StringUtil::format("PBS_%d_%d", x, z));
				clone->setShininess(128.0 - shininess*0.6);
				clone->setSpecular(ColourValue::White*intensity);

				ent->setMaterial(clone);
				mMaterialList.push_back(clone);
			}
		}

		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();
		mCameraMan->setYawPitchDist(Degree(0), Degree(25), 20);
		mCameraMan->setTopSpeed(5);
		mCamera->setNearClipDistance(1);
    }
};

#endif
