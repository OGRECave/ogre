#ifndef __HLMS_H__
#define __HLMS_H__

#include "SdkSample.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsPbsMaterial.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_HLMS : public SdkSample
{
public:

	Sample_HLMS() : mHlmsManager(NULL)
    {
        mInfo["Title"] = "HLMS";
        mInfo["Description"] = "Shows how to use hlms.";
        mInfo["Thumbnail"] = "thumb_hlms.png";
        mInfo["Category"] = "Lighting";
    }

private:
	Ogre::HlmsManager* mHlmsManager;
	std::vector<Ogre::PbsMaterial*> mPBSMaterialList;
	std::vector<String> mMaterialList;
	Ogre::SceneNode* mLightNodes[3];
	Ogre::MeshPtr mFloor;

protected:

	void cleanupContent()
	{
		for (size_t i = 0; i < mPBSMaterialList.size(); ++i)
		{
			delete mPBSMaterialList[i];
		}

		for (size_t i = 0; i < mMaterialList.size(); ++i)
		{
			MaterialManager::getSingleton().unload(mMaterialList[i], ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
			MaterialManager::getSingleton().remove(mMaterialList[i], ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		}
		mMaterialList.clear();
		mFloor->unload();
		MeshManager::getSingleton().remove(mFloor);
		mFloor.reset();

		mHlmsManager->unbindAll("pbs");
		delete mHlmsManager;
	}

    void setupContent()
    {
		mHlmsManager = new Ogre::HlmsManager(mSceneMgr);

        // setup some basic lighting for our scene
		Ogre::SceneNode *rootNode = mSceneMgr->getRootSceneNode();

		Ogre::Light *light = mSceneMgr->createLight();
		Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setPowerScale(1.0f);
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		lightNode->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
		mLightNodes[0] = lightNode;

		mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f));

		light = mSceneMgr->createLight();
		lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setDiffuseColour(0.8f, 0.4f, 0.2f); //Warm
		light->setSpecularColour(0.8f, 0.4f, 0.2f);
		light->setPowerScale(Ogre::Math::PI);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(-10.0f, 10.0f, 10.0f);
		lightNode->setDirection(Ogre::Vector3(1, -1, -1).normalisedCopy());
		mLightNodes[1] = lightNode;

		light = mSceneMgr->createLight();
		lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setDiffuseColour(0.2f, 0.4f, 0.8f); //Cold
		light->setSpecularColour(0.2f, 0.4f, 0.8f);
		light->setPowerScale(Ogre::Math::PI);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(10.0f, 10.0f, -10.0f);
		lightNode->setDirection(Ogre::Vector3(-1, -1, 1).normalisedCopy());
		mLightNodes[2] = lightNode;


		mViewport->setBackgroundColour(ColourValue(0.8f, 1.0f, 1.0f));
		mSceneMgr->setFog(Ogre::FOG_LINEAR, ColourValue(0.8f, 1.0f, 1.0f), 0, 15, 100);

		// create a floor mesh resource
		mFloor = MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, 0), 100, 100, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

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
		
		TexturePtr cubeMap = Ogre::TextureManager::getSingletonPtr()->load("cubescene.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_CUBE_MAP);
		//TexturePtr anisoTex = Ogre::TextureManager::getSingletonPtr()->load("hlms_aniso_baked.png", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		//
		//{			
		//	MeshPtr sphere = MeshManager::getSingleton().load("SphereManualTangents.mesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//	ensureHasTangents(sphere);

		//	Ogre::Entity *ent = mSceneMgr->createEntity(sphere);
		//	Ogre::SceneNode *sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		//	sceneNode->setPosition(0,	3.0f, 0);
		//	sceneNode->setScale(Vector3(0.5));
		//	sceneNode->attachObject(ent);

		//	PbsMaterial* pbsMaterial = new PbsMaterial();
		//	pbsMaterial->setAlbedo(ColourValue::Green);

		//	pbsMaterial->setRoughness(0.32);
		//	pbsMaterial->setLightRoughnessOffset(0.2);
		//	pbsMaterial->setF0(Ogre::ColourValue(0.9, 0.9, 0.9));
		//	pbsMaterial->setEnvironmentMap(cubeMap);
		//	pbsMaterial->setNormalrTexture(PbsMaterial::MS_MAIN, anisoTex, PbsMaterial::TextureAddressing(), 1.0, 1.0);
		//	createHLMSMaterial(ent, pbsMaterial, "PBS_ANISO");
		//}

		for (int x = 0; x<numX; ++x)
		{
			for (int z = 0; z<numZ; ++z)
			{
				Entity *ent = mSceneMgr->createEntity("Sphere1000.mesh");
				SceneNode *sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				sceneNode->setPosition(Vector3(armsLength * x - startX,	1.0f, armsLength * z - startZ));
				sceneNode->attachObject(ent);

				PbsMaterial* pbsMaterial = new PbsMaterial();
				pbsMaterial->setAlbedo(ColourValue::Green);

				pbsMaterial->setRoughness(std::max<float>(0.02f, x / std::max<float>(1, (float)(numX - 1)))  * 0.6 );
				pbsMaterial->setLightRoughnessOffset(0.2);

				float f0 = (z / std::max<float>(1, (float)(numZ - 1))) * 0.9 + 0.1;
				pbsMaterial->setF0(Ogre::ColourValue(f0, f0, f0));
				pbsMaterial->setEnvironmentMap(cubeMap);
				
				createHLMSMaterial(ent, pbsMaterial, "PBS_" + Ogre::StringConverter::toString(x) + "_" + Ogre::StringConverter::toString(z));
			}
		}

		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();
		mCameraMan->setYawPitchDist(Degree(0), Degree(25), 20);
		mCameraMan->setTopSpeed(5);
		mCamera->setNearClipDistance(1);
    }


	void createHLMSMaterial(Ogre::Entity *ent, PbsMaterial* pbsMaterial, String matName)
	{
		size_t count = ent->getNumSubEntities();
		for (size_t i = 0; i < count; i++)
		{
			Ogre::SubEntity* subEnt = ent->getSubEntity(i);
			Ogre::MaterialPtr newMat = subEnt->getMaterial()->clone(matName + "_" + Ogre::StringConverter::toString(i));
			newMat->removeAllTechniques();
			Ogre::Pass* pass = newMat->createTechnique()->createPass();
#if defined(INCLUDE_RTSHADER_SYSTEM)
			// ensures that this material does not get picked up by the RTSS
			newMat->getTechnique(0)->setSchemeName(mViewport->getMaterialScheme());
#endif
			pass->setName("pbs");
			subEnt->setMaterial(newMat);
			mMaterialList.push_back(newMat->getName());
			mHlmsManager->bind(subEnt, pbsMaterial, "pbs");
		}
	}

	static void ensureHasTangents(Ogre::MeshPtr mesh)
	{
		bool generateTangents = false;
		if (mesh->sharedVertexData)
		{
			Ogre::VertexDeclaration *vertexDecl = mesh->sharedVertexData->vertexDeclaration;
			generateTangents |= hasNoTangentsAndCanGenerate(vertexDecl);
		}

		for (ushort i = 0; i<mesh->getNumSubMeshes(); ++i)
		{
			Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
			if (subMesh->vertexData)
			{
				Ogre::VertexDeclaration *vertexDecl = subMesh->vertexData->vertexDeclaration;
				generateTangents |= hasNoTangentsAndCanGenerate(vertexDecl);
			}
		}

		try
		{
			if (generateTangents)
			{
				mesh->buildTangentVectors();
			}


		}
		catch (...) {}
	}

	static bool hasNoTangentsAndCanGenerate(Ogre::VertexDeclaration *vertexDecl)
	{
		bool hasTangents = false;
		bool hasUVs = false;
		const Ogre::VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
		Ogre::VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
		Ogre::VertexDeclaration::VertexElementList::const_iterator end = elementList.end();

		while (itor != end && !hasTangents)
		{
			const Ogre::VertexElement &vertexElem = *itor;
			if (vertexElem.getSemantic() == Ogre::VES_TANGENT)
				hasTangents = true;
			if (vertexElem.getSemantic() == Ogre::VES_TEXTURE_COORDINATES)
				hasUVs = true;

			++itor;
		}

		return !hasTangents && hasUVs;
	}
};

#endif
