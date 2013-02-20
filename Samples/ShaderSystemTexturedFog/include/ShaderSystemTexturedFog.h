#ifndef __ShaderSystemTexturedFog_H__
#define __ShaderSystemTexturedFog_H__

#include "SdkSample.h"
#include "RTShaderSRSTexturedFog.h"
#include "OgreControllerManager.h"

using namespace Ogre;
using namespace OgreBites;

const uint8 cPriorityMain = 50;
const String FOG_DISTANCE_SLIDER = "FogDistance";
const String ACTIVATE_FOG_BUTTON = "ActivateFog";
const String FOG_BACKGROUND_SLIDER = "FogBackground";
const String ACTIVATE_SKY_BUTTON = "ActivateSkyBox";


class _OgreSampleClassExport Sample_ShaderSystemTexturedFog : public SdkSample
{
public:

	Sample_ShaderSystemTexturedFog() :
		mSRSTextureFogFactory(NULL),
			mEntityNameGen("Head")
	{
		mInfo["Title"] = "Shader System - Textured Fog";
		mInfo["Description"] = "Shows a simple implementation of a RTSS sub-render state implementing a fog based on texture effect.";
		mInfo["Thumbnail"] = "thumb_texturedfog.png";
		mInfo["Category"] = "Lighting";
	}

	~Sample_ShaderSystemTexturedFog()
	{
		
	}

	virtual void _shutdown()
	{
		RTShader::RenderState* pMainRenderState = 
			RTShader::ShaderGenerator::getSingleton().createOrRetrieveRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME).first;
		pMainRenderState->reset();
		
		if (mSRSTextureFogFactory)
		{
			RTShader::ShaderGenerator::getSingleton().removeAllShaderBasedTechniques();
			RTShader::ShaderGenerator::getSingleton().removeSubRenderStateFactory(mSRSTextureFogFactory);
			delete mSRSTextureFogFactory;
			mSRSTextureFogFactory = NULL;
		}

		SdkSample::_shutdown();
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
    }

protected:

   void setupContent()
	{
		mTrayMgr->createCheckBox(TL_BOTTOM, ACTIVATE_FOG_BUTTON, "Fog Active")->setChecked(true,false);
		mTrayMgr->createCheckBox(TL_BOTTOM, ACTIVATE_SKY_BUTTON, "Sky Box Active")->setChecked(true,false);
		mTrayMgr->createThickSlider(TL_BOTTOM, FOG_DISTANCE_SLIDER, "Fog Distance", 240, 80, 20, 2000, 100)->setValue(1000, false);
		mTrayMgr->createThickSlider(TL_BOTTOM, FOG_BACKGROUND_SLIDER, "Background", 240, 80, 0, 3, 4)->setValue(0, false);

        setupShaderGenerator();

		mSceneMgr->setFog(FOG_LINEAR, ColourValue::White, 0, 500, 1000);
		
		mCamera->setPosition(0, -20, 470);

		mTrayMgr->showCursor();

		// create a floor mesh resource
		MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Vector3::UNIT_Z);

		// create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode()->attachObject(floor);
		
		addHead(Vector3(100,0,-400));
		addHead(Vector3(100,0,-200));
		addHead(Vector3(100,0,0));
		addHead(Vector3(100,0,200));
		
		addHead(Vector3(-100,0,-400));
		addHead(Vector3(-100,0,-200));
		addHead(Vector3(-100,0,0));
		addHead(Vector3(-100,0,200));

		//We will set the sky box far away so it will render with the color of the background
		mSceneMgr->setSkyBox(true,"BaseWhite",2000);
	}

   void addHead(const Vector3& pos)
   {

		// Create an ogre head and place it at the origin
		Entity* head = mSceneMgr->createEntity(mEntityNameGen.generate(), "ogrehead.mesh");
		head->setRenderQueueGroup(cPriorityMain);
		mSceneMgr->getRootSceneNode()->createChildSceneNode(pos)
			->attachObject(head);
   }

		
	void setupShaderGenerator()
	{
		RTShader::ShaderGenerator* mGen = RTShader::ShaderGenerator::getSingletonPtr();

		RTShader::RenderState* pMainRenderState = 
            mGen->createOrRetrieveRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME).first;
		pMainRenderState->reset();

		mSRSTextureFogFactory = new RTShaderSRSTexturedFogFactory;
		mGen->addSubRenderStateFactory(mSRSTextureFogFactory);
		pMainRenderState->addTemplateSubRenderState(
			mGen->createSubRenderState(RTShaderSRSTexturedFog::Type));	
		
		mSRSTextureFogFactory->setBackgroundTextureName("early_morning.jpg");

		
		
		mGen->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

		// Make this viewport work with shader generator scheme.
		mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
	}


	

	//--------------------------------------------------------------------------
	void sliderMoved(Slider* slider)
	{
		if (slider->getName() == FOG_DISTANCE_SLIDER)
		{
			Real fogDist = slider->getValue();
			mSceneMgr->setFog(mSceneMgr->getFogMode(), ColourValue::White, 0, fogDist * 0.5f, fogDist);
		}	
		if (slider->getName() == FOG_BACKGROUND_SLIDER)
		{
			String textureName;
			size_t back = (size_t)(slider->getValue() + 0.5);
			switch (back)
			{
			case 0: textureName = "early_morning.jpg"; break;
			case 1: textureName = "cloudy_noon.jpg"; break;
			case 2: textureName = "stormy.jpg"; break;
			default: textureName = "evening.jpg"; break;
			}
			mSRSTextureFogFactory->setBackgroundTextureName(textureName);
			RTShader::ShaderGenerator* gen = RTShader::ShaderGenerator::getSingletonPtr();
			gen->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
		}
	}
		
	void checkBoxToggled(CheckBox* box)
	{
		const String& cbName = box->getName();
		if (cbName == ACTIVATE_FOG_BUTTON)
		{
			//With our shader the 3 middle parameters don't really make a difference. Only the first and last do.
			FogMode mode = mSceneMgr->getFogMode() == FOG_NONE ? FOG_LINEAR : FOG_NONE;
			mSceneMgr->setFog(mode, ColourValue::White, 0, mSceneMgr->getFogStart(), mSceneMgr->getFogEnd());
		}
		if (cbName == ACTIVATE_SKY_BUTTON)
		{
			mSceneMgr->setSkyBox(!mSceneMgr->isSkyBoxEnabled(),"BaseWhite",2000);

		}
	}

	#if (OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS) && (OGRE_PLATFORM != OGRE_PLATFORM_ANDROID)

	//-----------------------------------------------------------------------
	bool mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
	{
		if (mTrayMgr->injectMouseDown(evt, id)) 
			return true;
		if (id == OIS::MB_Left) 	
			mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene			
	
		return true;
	}

	//-----------------------------------------------------------------------
	bool mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
	{
		if (mTrayMgr->injectMouseUp(evt, id)) 
			return true;
		if (id == OIS::MB_Left) 
			mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB

		return true;
	}

	//-----------------------------------------------------------------------
	bool mouseMoved( const OIS::MouseEvent& evt )
	{
		// only rotate the camera if cursor is hidden
		if (mTrayMgr->isCursorVisible()) 
			mTrayMgr->injectMouseMove(evt);
		else 
			mCameraMan->injectMouseMove(evt);


		return true;
	}
	#endif

	

private:
	RTShaderSRSTexturedFogFactory* mSRSTextureFogFactory;

	NameGenerator mEntityNameGen;

};

#endif
