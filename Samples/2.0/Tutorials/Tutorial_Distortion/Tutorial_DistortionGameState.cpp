
#include "Tutorial_DistortionGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

using namespace Demo;

namespace Demo
{
	DistortionGameState::DistortionGameState(const Ogre::String &helpDescription) :
		TutorialGameState(helpDescription),
		mAnimateObjects(true),
		mAnimateDistortion(true),
		mDistortionStrenght(1.0f),
		mNumSpheres(0),
		mTransparencyMode(Ogre::HlmsPbsDatablock::None),
		mTransparencyValue(1.0f)
	{
		memset(mSceneNode, 0, sizeof(mSceneNode));
	}
	//-----------------------------------------------------------------------------------
	void DistortionGameState::createScene01(void)
	{
		/*
		Lets create a scene to look at.
		We will be using standard PBS example as demo scene
		*/
		Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

		const float armsLength = 2.5f;

		Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane("Plane v1",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::Plane(Ogre::Vector3::UNIT_Y, 1.0f), 50.0f, 50.0f,
			1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z,
			Ogre::v1::HardwareBuffer::HBU_STATIC,
			Ogre::v1::HardwareBuffer::HBU_STATIC);

		Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
			"Plane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		planeMesh->importV1(planeMeshV1.get(), true, true, true);

		{
			Ogre::Item *item = sceneManager->createItem(planeMesh, Ogre::SCENE_DYNAMIC);
			item->setDatablock("Marble");
			Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
				createChildSceneNode(Ogre::SCENE_DYNAMIC);
			sceneNode->setPosition(0, -1, 0);
			sceneNode->attachObject(item);

			//Change the addressing mode of the roughness map to wrap via code.
			//Detail maps default to wrap, but the rest to clamp.
			assert(dynamic_cast<Ogre::HlmsPbsDatablock*>(item->getSubItem(0)->getDatablock()));
			Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
				item->getSubItem(0)->getDatablock());
			//Make a hard copy of the sampler block
			Ogre::HlmsSamplerblock samplerblock(*datablock->getSamplerblock(Ogre::PBSM_ROUGHNESS));
			samplerblock.mU = Ogre::TAM_WRAP;
			samplerblock.mV = Ogre::TAM_WRAP;
			samplerblock.mW = Ogre::TAM_WRAP;
			//Set the new samplerblock. The Hlms system will
			//automatically create the API block if necessary
			datablock->setSamplerblock(Ogre::PBSM_ROUGHNESS, samplerblock);
		}

		for (int i = 0; i<4; ++i)
		{
			for (int j = 0; j<4; ++j)
			{
				Ogre::String meshName;

				if (i == j)
					meshName = "Sphere1000.mesh";
				else
					meshName = "Cube_d.mesh";

				Ogre::Item *item = sceneManager->createItem(meshName,
					Ogre::ResourceGroupManager::
					AUTODETECT_RESOURCE_GROUP_NAME,
					Ogre::SCENE_DYNAMIC);
				if (i % 2 == 0)
					item->setDatablock("Rocks");
				else
					item->setDatablock("Marble");

				item->setVisibilityFlags(0x000000001);

				size_t idx = i * 4 + j;

				mSceneNode[idx] = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
					createChildSceneNode(Ogre::SCENE_DYNAMIC);

				mSceneNode[idx]->setPosition((i - 1.5f) * armsLength,
					2.0f,
					(j - 1.5f) * armsLength);
				mSceneNode[idx]->setScale(0.65f, 0.65f, 0.65f);

				mSceneNode[idx]->roll(Ogre::Radian((Ogre::Real)idx));

				mSceneNode[idx]->attachObject(item);
			}
		}

		{
			mNumSpheres = 0;
			Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
			Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

			assert(dynamic_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS)));

			Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

			const int numX = 8;
			const int numZ = 8;

			const float armsLength = 1.0f;
			const float startX = (numX - 1) / 2.0f;
			const float startZ = (numZ - 1) / 2.0f;

			for (int x = 0; x<numX; ++x)
			{
				for (int z = 0; z<numZ; ++z)
				{
					Ogre::String datablockName = "Test" + Ogre::StringConverter::toString(mNumSpheres++);
					Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
						hlmsPbs->createDatablock(datablockName,
							datablockName,
							Ogre::HlmsMacroblock(),
							Ogre::HlmsBlendblock(),
							Ogre::HlmsParamVec()));

					Ogre::HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
						createOrRetrieveTexture("SaintPetersBasilica.dds",
							Ogre::HlmsTextureManager::TEXTURE_TYPE_ENV_MAP);

					datablock->setTexture(Ogre::PBSM_REFLECTION, texLocation.xIdx, texLocation.texture);
					datablock->setDiffuse(Ogre::Vector3(0.0f, 1.0f, 0.0f));

					datablock->setRoughness(std::max(0.02f, x / Ogre::max(1, (float)(numX - 1))));
					datablock->setFresnel(Ogre::Vector3(z / Ogre::max(1, (float)(numZ - 1))), false);

					Ogre::Item *item = sceneManager->createItem("Sphere1000.mesh",
						Ogre::ResourceGroupManager::
						AUTODETECT_RESOURCE_GROUP_NAME,
						Ogre::SCENE_DYNAMIC);
					item->setDatablock(datablock);
					item->setVisibilityFlags(0x000000002);

					Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
						createChildSceneNode(Ogre::SCENE_DYNAMIC);
					sceneNode->setPosition(Ogre::Vector3(armsLength * x - startX,
						1.0f,
						armsLength * z - startZ));
					sceneNode->attachObject(item);
				}
			}
		}

		Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

		Ogre::Light *light = sceneManager->createLight();
		Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setPowerScale(1.0f);
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

		mLightNodes[0] = lightNode;

		sceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f,
			Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
			-light->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f);

		light = sceneManager->createLight();
		lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setDiffuseColour(0.8f, 0.4f, 0.2f); //Warm
		light->setSpecularColour(0.8f, 0.4f, 0.2f);
		light->setPowerScale(Ogre::Math::PI);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(-10.0f, 10.0f, 10.0f);
		light->setDirection(Ogre::Vector3(1, -1, -1).normalisedCopy());
		light->setAttenuationBasedOnRadius(10.0f, 0.01f);

		mLightNodes[1] = lightNode;

		light = sceneManager->createLight();
		lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject(light);
		light->setDiffuseColour(0.2f, 0.4f, 0.8f); //Cold
		light->setSpecularColour(0.2f, 0.4f, 0.8f);
		light->setPowerScale(Ogre::Math::PI);
		light->setType(Ogre::Light::LT_SPOTLIGHT);
		lightNode->setPosition(10.0f, 10.0f, -10.0f);
		light->setDirection(Ogre::Vector3(-1, -1, 1).normalisedCopy());
		light->setAttenuationBasedOnRadius(10.0f, 0.01f);

		mLightNodes[2] = lightNode;

		mCameraController = new CameraController(mGraphicsSystem, false);

		//----------------------------------------------------
		//----------------- DISTORTION PART ---------------------------------
		//----------------------------------------------------
		/*
		We will create varous items that are used to distort the scene in postprocessing. You can use whatever objects you want to (fe. particle effect billboards)
		but we are going to create just simple spheres.
		*/
		{
			//Lets setup a new render queue for distortion pass. Set ID 6 to be our distortion queue
            mGraphicsSystem->getSceneManager()->getRenderQueue()->setRenderQueueMode(6, Ogre::RenderQueue::FAST);

			Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
			Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

			assert(dynamic_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT)));

			Ogre::HlmsUnlit *hlmsUnlit = static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

			for (int i = 0; i < 10; ++i)
			{
				/*
				Next we need to setup materials and items:
				- We will be using unlit material with displacement texture.
				- Distortion objects will be rendered in their own render queue to a separate texture.
				- See distortion compositor and shaders for more information
				*/

				//Create proper blend block for distortion objects
				Ogre::HlmsBlendblock blendBlock = Ogre::HlmsBlendblock();
				blendBlock.mIsTransparent = true;
                blendBlock.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
                blendBlock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;

				//Create macro block to disable depth write
				Ogre::HlmsMacroblock macroBlock = Ogre::HlmsMacroblock();
				macroBlock.mDepthWrite = false;
				macroBlock.mDepthCheck = true;

				Ogre::String datablockName = "DistMat" + Ogre::StringConverter::toString(i);
				Ogre::HlmsUnlitDatablock *datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
					hlmsUnlit->createDatablock(datablockName,
						datablockName,
						macroBlock,
						blendBlock,
						Ogre::HlmsParamVec()));

				//Use non-color data as texture type because distortion is stored as x,y vectors to texture.
				Ogre::HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
					createOrRetrieveTexture("distort_deriv.png",
						Ogre::HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA);

				datablock->setTexture(0, texLocation.xIdx, texLocation.texture);

				//Set material to use vertex colors. Vertex colors are used to control distortion intensity (alpha value)
				datablock->setUseColour(true);
				//Random alpha value for objects. Alpha value is multiplier for distortion strenght
				datablock->setColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, Ogre::Math::RangeRandom(0.25f, 0.5f)));

				Ogre::String meshName = "Sphere1000.mesh";

				Ogre::Item *item = sceneManager->createItem(meshName,
					Ogre::ResourceGroupManager::
					AUTODETECT_RESOURCE_GROUP_NAME,
					Ogre::SCENE_DYNAMIC);

				item->setDatablock(datablock);

				//Set item to be rendered in distortion queue pass (ID 6)
				item->setRenderQueueGroup(6);

				mDistortionSceneNode[i] = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
					createChildSceneNode(Ogre::SCENE_DYNAMIC);

				//Lets stack distortion objects to same position with little variation
				mDistortionSceneNode[i]->setPosition(
					Ogre::Math::RangeRandom(-3.0f, 3.0f),
					Ogre::Math::RangeRandom(0.85f, 4.15f),
					Ogre::Math::RangeRandom(-3.0f, 3.0f));

				float scale = Ogre::Math::RangeRandom(1.5f, 5.5f);
				mDistortionSceneNode[i]->setScale(scale, scale, scale);

				mDistortionSceneNode[i]->roll(Ogre::Radian((Ogre::Real)i));

				mDistortionSceneNode[i]->attachObject(item);
			}

			//Receive distortion material and set strenght uniform
			Ogre::MaterialPtr materialDistortion = Ogre::MaterialManager::getSingleton().load(
				"Distortion/Quad",
				Ogre::ResourceGroupManager::
				AUTODETECT_RESOURCE_GROUP_NAME).staticCast<Ogre::Material>();

			Ogre::Pass *passDist = materialDistortion->getTechnique(0)->getPass(0);
			mDistortionPass = passDist;
			Ogre::GpuProgramParametersSharedPtr psParams = passDist->getFragmentProgramParameters();
			psParams->setNamedConstant("u_DistortionStrenght", mDistortionStrenght);
		}

		TutorialGameState::createScene01();
	}
	//-----------------------------------------------------------------------------------
	void DistortionGameState::update(float timeSinceLast)
	{
		if (mAnimateObjects)
		{
			for (int i = 0; i<16; ++i)
				mSceneNode[i]->yaw(Ogre::Radian(timeSinceLast * i * 0.125f));
		}

		if (mAnimateDistortion)
		{
			for (int i = 0; i<10; ++i)
				mDistortionSceneNode[i]->yaw(Ogre::Radian(timeSinceLast * (i + 1.0f) * 0.825f));
		}

		//Update distortion uniform
		Ogre::GpuProgramParametersSharedPtr psParams = mDistortionPass->getFragmentProgramParameters();
		psParams->setNamedConstant("u_DistortionStrenght", mDistortionStrenght);

		TutorialGameState::update(timeSinceLast);
	}
	//-----------------------------------------------------------------------------------
	void DistortionGameState::generateDebugText(float timeSinceLast, Ogre::String &outText)
	{
		Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();

		TutorialGameState::generateDebugText(timeSinceLast, outText);
		outText += "\nPress F2 to toggle animation. ";
		outText += mAnimateObjects ? "[On]" : "[Off]";
		outText += "\nPress F3 to show/hide animated objects. ";
		outText += (visibilityMask & 0x000000001) ? "[On]" : "[Off]";
		outText += "\nPress F4 to show/hide palette of spheres. ";
		outText += (visibilityMask & 0x000000002) ? "[On]" : "[Off]";
		outText += "\nPress F5 to toggle transparency mode. ";
		outText += mTransparencyMode == Ogre::HlmsPbsDatablock::Fade ? "[Fade]" : "[Transparent]";
		outText += "\n+/- to change distortion strenght. [";
		outText += Ogre::StringConverter::toString(mDistortionStrenght) + "]";

	}
	//-----------------------------------------------------------------------------------
	void DistortionGameState::setTransparencyToMaterials(void)
	{
		Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();

		assert(dynamic_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS)));

		Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

		Ogre::HlmsPbsDatablock::TransparencyModes mode =
			static_cast<Ogre::HlmsPbsDatablock::TransparencyModes>(mTransparencyMode);

		if (mTransparencyValue >= 1.0f)
			mode = Ogre::HlmsPbsDatablock::None;

		if (mTransparencyMode < 1.0f && mode == Ogre::HlmsPbsDatablock::None)
			mode = Ogre::HlmsPbsDatablock::Transparent;

		for (size_t i = 0; i<mNumSpheres; ++i)
		{
			Ogre::String datablockName = "Test" + Ogre::StringConverter::toString(i);
			Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
				hlmsPbs->getDatablock(datablockName));

			datablock->setTransparency(mTransparencyValue, mode);
		}
	}
	//-----------------------------------------------------------------------------------
	void DistortionGameState::keyReleased(const SDL_KeyboardEvent &arg)
	{
		if ((arg.keysym.mod & ~(KMOD_NUM | KMOD_CAPS)) != 0)
		{
			TutorialGameState::keyReleased(arg);
			return;
		}

		if (arg.keysym.sym == SDLK_F2)
		{
			mAnimateObjects = !mAnimateObjects;
		}
		else if (arg.keysym.sym == SDLK_F3)
		{
			Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
			bool showMovingObjects = (visibilityMask & 0x00000001);
			showMovingObjects = !showMovingObjects;
			visibilityMask &= ~0x00000001;
			visibilityMask |= (Ogre::uint32)showMovingObjects;
			mGraphicsSystem->getSceneManager()->setVisibilityMask(visibilityMask);
		}
		else if (arg.keysym.sym == SDLK_F4)
		{
			Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
			bool showPalette = (visibilityMask & 0x00000002) != 0;
			showPalette = !showPalette;
			visibilityMask &= ~0x00000002;
			visibilityMask |= (Ogre::uint32)(showPalette) << 1;
			mGraphicsSystem->getSceneManager()->setVisibilityMask(visibilityMask);
		}
		else if (arg.keysym.sym == SDLK_F5)
		{
			mTransparencyMode = mTransparencyMode == Ogre::HlmsPbsDatablock::Fade ?
				Ogre::HlmsPbsDatablock::Transparent :
				Ogre::HlmsPbsDatablock::Fade;
			if (mTransparencyValue != 1.0f)
				setTransparencyToMaterials();
		}
		else if (arg.keysym.scancode == SDL_SCANCODE_KP_PLUS)
		{
			mDistortionStrenght += 0.05f;
		}
		else if (arg.keysym.scancode == SDL_SCANCODE_MINUS ||
			arg.keysym.scancode == SDL_SCANCODE_KP_MINUS)
		{
			mDistortionStrenght -= 0.05f;
		}
		else
		{
			TutorialGameState::keyReleased(arg);
		}
	}
}
