
#include "Tutorial_SSAOGameState.h"
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
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

#include "OgreResourceGroupManager.h"
#include "OgreTextureManager.h"
#include "Ogre.h"

using namespace Demo;

namespace Demo
{
	Tutorial_SSAOGameState::Tutorial_SSAOGameState(const Ogre::String &helpDescription) :
		TutorialGameState(helpDescription),
		mAnimateObjects(true),
		mNumSpheres(0),
		mPowerScale(1.5f),
		mKernelRadius(1.0f)
	{
		memset(mSceneNode, 0, sizeof(mSceneNode));
	}
	//-----------------------------------------------------------------------------------
	void Tutorial_SSAOGameState::createScene01(void)
	{
		Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

		//Lets make some environment to look at
		//We use the same code as PBS demo to create meshes

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


		{
			mNumSpheres = 0;
			Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
			Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

			assert(dynamic_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS)));

			Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

			const int numX = 8;
			const int numZ = 8;

			const float armsLength = 2.0f;
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

					std::string meshName;
					float meshScale = 1.0f;
					if (x == z)
					{
						meshName = "Sphere1000.mesh";
						meshScale = 2.0f;
					}
					else
					{
						meshName = "Sphere1000.mesh";
						meshScale = 3.0f;
					}

					Ogre::Item *item = sceneManager->createItem(meshName,
						Ogre::ResourceGroupManager::
						AUTODETECT_RESOURCE_GROUP_NAME,
						Ogre::SCENE_DYNAMIC);
					item->setDatablock(datablock);
					item->setVisibilityFlags(0x000000002);

					if(x != z) item->setDatablock("Marble");

					Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
						createChildSceneNode(Ogre::SCENE_DYNAMIC);
					sceneNode->setPosition(Ogre::Vector3(armsLength * x - startX,
						0.0f + (numZ-z)*0.5f,
						armsLength * z - startZ));
					sceneNode->setScale(Ogre::Vector3(meshScale, meshScale, meshScale));
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

		//---------------------------------------------------------------------------------

		//We need to create SSAO kernel samples and noise texture
		//Generate kernel samples first
		Ogre::Vector3 kernelSamples[64];
		for (int i = 0; i < 64; i++)
		{
			Ogre::Vector3 sample = Ogre::Vector3(Ogre::Math::RangeRandom(-1.0f, 1.0f), Ogre::Math::RangeRandom(-1.0f, 1.0f), Ogre::Math::RangeRandom(0.0f, 1.0f));

			sample.normalise();

			float scale = (float)i / 64.0f;
			scale = Ogre::Math::lerp(0.3f, 1.0f, scale*scale);
			sample = sample * scale;

			sample.x = (sample.x + 1.0f) * 0.5f;
			sample.y = (sample.y + 1.0f) * 0.5f;

			kernelSamples[i] = sample;
		}

		//Generate sample texture
		Ogre::TexturePtr sampleTexture = Ogre::TextureManager::getSingleton().createManual(
			"sampleTexture",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TextureType::TEX_TYPE_2D, 8, 8, 0,
			Ogre::PF_R8G8B8,
			Ogre::TextureUsage::TU_DYNAMIC_WRITE_ONLY);

		Ogre::v1::HardwarePixelBufferSharedPtr pixelBuffer = sampleTexture->getBuffer();
		const Ogre::PixelBox& pixelBox = pixelBuffer->lock(Ogre::Box(0, 0, sampleTexture->getWidth(), sampleTexture->getHeight()), Ogre::v1::HardwareBuffer::HBL_DISCARD);

		unsigned int sampleIter = 0;
		for (size_t j = 0; j < pixelBox.getWidth(); j++)
		{
			for (size_t i = 0; i < pixelBox.getHeight(); i++)
			{
				Ogre::PixelBox& pb = Ogre::PixelBox(pixelBox);
				pb.setColourAt(Ogre::ColourValue(kernelSamples[sampleIter].x, kernelSamples[sampleIter].y, kernelSamples[sampleIter].z, 1.0f), j, i, 0);
				sampleIter++;
			}
		}
		pixelBuffer->unlock();

		//Next generate noise texture
		Ogre::TexturePtr noiseTexture = Ogre::TextureManager::getSingleton().createManual(
			"noiseTexture",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TextureType::TEX_TYPE_2D, 2, 2, 0,
			Ogre::PixelFormat::PF_R8G8B8,
			Ogre::TextureUsage::TU_DYNAMIC_WRITE_ONLY);

		
		Ogre::v1::HardwarePixelBufferSharedPtr pixelBuffer2 = noiseTexture->getBuffer();
		const Ogre::PixelBox& pixelBox2 = pixelBuffer2->lock(Ogre::Box(0, 0, noiseTexture->getWidth(), noiseTexture->getHeight()), Ogre::v1::HardwareBuffer::HBL_DISCARD);

		for (size_t j = 0; j < pixelBox2.getWidth(); j++)
		{
			for (size_t i = 0; i < pixelBox2.getHeight(); i++)
			{
				Ogre::Vector3 noise = Ogre::Vector3(Ogre::Math::RangeRandom(-1.0f, 1.0f), Ogre::Math::RangeRandom(-1.0f, 1.0f), 0.0f);
				noise.normalise();

				//Range values to [0.0 - 1.0] we will re-range them later in shader to the orginal ones
				noise.x = (noise.x + 1.0f) * 0.5f;
				noise.y = (noise.y + 1.0f) * 0.5f;

				Ogre::PixelBox& pb = Ogre::PixelBox(pixelBox2);
				pb.setColourAt(Ogre::ColourValue(noise.x, noise.y, noise.z, 1.0f), j, i, 0);
			}
		}
		pixelBuffer2->unlock();

		//---------------------------------------------------------------------------------
		//Get GpuProgramParametersSharedPtr to set uniforms that we need
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
			"SSAO/HS",
			Ogre::ResourceGroupManager::
			AUTODETECT_RESOURCE_GROUP_NAME).staticCast<Ogre::Material>();

		Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
		mSSAOPass = pass;
		Ogre::GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

		//Lets set uniforms for shader
		//Set texture uniform for noise
		Ogre::TextureUnitState* noiseTextureState = pass->getTextureUnitState("noiseTexture");
		noiseTextureState->setTexture(noiseTexture);

		//Set texture uniform for sampler
		Ogre::TextureUnitState* sampleTextureState = pass->getTextureUnitState("sampleTexture");
		sampleTextureState->setTexture(sampleTexture);

		//Reconstruct position from depth. Position is needed in SSAO
		//We need to set the parameters based on camera to the
		//shader so that the un-projection works as expected
		Ogre::Camera *camera = mGraphicsSystem->getCamera();
		Ogre::Real projectionA = camera->getFarClipDistance() /
			(camera->getFarClipDistance() - camera->getNearClipDistance());
		Ogre::Real projectionB = (-camera->getFarClipDistance() * camera->getNearClipDistance()) /
			(camera->getFarClipDistance() - camera->getNearClipDistance());
		//The division will keep "linearDepth" in the shader in the [0; 1] range.
		projectionB /= camera->getFarClipDistance();
		psParams->setNamedConstant("projectionParams", Ogre::Vector2(projectionA, projectionB));

		//Set other uniforms
		psParams->setNamedConstant("kernelRadius", 0.35f);
		psParams->setNamedConstant("noiseScale", Ogre::Vector2(((float)mGraphicsSystem->getRenderWindow()->getWidth()*0.5f)/2.0f, ((float)mGraphicsSystem->getRenderWindow()->getHeight()*0.5f)/2.0f));
		psParams->setNamedConstant("kernelSize", 64);

		//Set blur shader uniforms
		Ogre::MaterialPtr materialBlurH = Ogre::MaterialManager::getSingleton().load(
			"SSAO/BlurH",
			Ogre::ResourceGroupManager::
			AUTODETECT_RESOURCE_GROUP_NAME).staticCast<Ogre::Material>();

		Ogre::Pass *passBlurH = materialBlurH->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr psParamsBlurH = passBlurH->getFragmentProgramParameters();
		psParamsBlurH->setNamedConstant("texelSize", Ogre::Vector2( 1.0f/(mGraphicsSystem->getRenderWindow()->getWidth()), 1.0f/(mGraphicsSystem->getRenderWindow()->getHeight()) ));
		psParamsBlurH->setNamedConstant("projectionParams", Ogre::Vector2(projectionA, projectionB));

		Ogre::MaterialPtr materialBlurV = Ogre::MaterialManager::getSingleton().load(
			"SSAO/BlurV",
			Ogre::ResourceGroupManager::
			AUTODETECT_RESOURCE_GROUP_NAME).staticCast<Ogre::Material>();

		Ogre::Pass *passBlurV = materialBlurV->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr psParamsBlurV = passBlurV->getFragmentProgramParameters();
		psParamsBlurV->setNamedConstant("texelSize", Ogre::Vector2(1.0f / (mGraphicsSystem->getRenderWindow()->getWidth()), 1.0f / (mGraphicsSystem->getRenderWindow()->getHeight())));
		psParamsBlurV->setNamedConstant("projectionParams", Ogre::Vector2(projectionA, projectionB));

		//Set apply shader uniforms
		Ogre::MaterialPtr materialApply = Ogre::MaterialManager::getSingleton().load(
			"SSAO/Apply",
			Ogre::ResourceGroupManager::
			AUTODETECT_RESOURCE_GROUP_NAME).staticCast<Ogre::Material>();

		Ogre::Pass *passApply = materialApply->getTechnique(0)->getPass(0);
		mApplyPass = passApply;
		Ogre::GpuProgramParametersSharedPtr psParamsApply = passApply->getFragmentProgramParameters();
		psParamsApply->setNamedConstant("powerScale", (float)1.5f);

		TutorialGameState::createScene01();
	}
	//-----------------------------------------------------------------------------------
	void Tutorial_SSAOGameState::update(float timeSinceLast)
	{

		Ogre::GpuProgramParametersSharedPtr psParams = mSSAOPass->getFragmentProgramParameters();
		psParams->setNamedConstant("projection", mGraphicsSystem->getCamera()->getProjectionMatrix());
		psParams->setNamedConstant("kernelRadius", mKernelRadius);

		Ogre::GpuProgramParametersSharedPtr psParamsApply = mApplyPass->getFragmentProgramParameters();
		psParamsApply->setNamedConstant("powerScale", mPowerScale);

		TutorialGameState::update(timeSinceLast);
	}
	//-----------------------------------------------------------------------------------
	void Tutorial_SSAOGameState::generateDebugText(float timeSinceLast, Ogre::String &outText)
	{
		TutorialGameState::generateDebugText(timeSinceLast, outText);

		if (mDisplayHelpMode == 1)
		{
			outText += "\nPress F5/F6 to increase/reduce kernel radius. ";
			outText += "[" + Ogre::StringConverter::toString(mKernelRadius) + "]";
			outText += "\nPress F7/F8 to increase/reduce power scale. ";
			outText += "[" + Ogre::StringConverter::toString(mPowerScale) + "]";
		}
	}
	//-----------------------------------------------------------------------------------
	void Tutorial_SSAOGameState::keyReleased(const SDL_KeyboardEvent &arg)
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
		else if (arg.keysym.sym == SDLK_F5)
		{
			mKernelRadius = mKernelRadius - 0.1f;
			if (mKernelRadius < 0.05f) mKernelRadius = 0.05f;
		}
		else if (arg.keysym.sym == SDLK_F6)
		{
			mKernelRadius = mKernelRadius + 0.1f;
			if (mKernelRadius > 10.0f) mKernelRadius = 10.0f;
		}
		else if (arg.keysym.sym == SDLK_F7)
		{
			mPowerScale = mPowerScale - 0.25f;
		}
		else if (arg.keysym.sym == SDLK_F8)
		{
			mPowerScale = mPowerScale + 0.25f;
		}
		else
		{
			TutorialGameState::keyReleased(arg);
		}
	}
}
