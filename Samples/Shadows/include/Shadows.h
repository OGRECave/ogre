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

/**
    \file 
        Shadows.cpp
    \brief
        Shows a few ways to use Ogre's shadowing techniques
*/

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;


Entity* mAthene;
AnimationState* mAnimState = 0;
Entity* pPlaneEnt;
vector<Entity*>::type pColumns;
Light* mLight;
Light* mSunLight;
SceneNode* mLightNode = 0;
AnimationState* mLightAnimationState = 0;
ColourValue mMinLightColour(0.2, 0.1, 0.0);
ColourValue mMaxLightColour(0.5, 0.3, 0.1);
Real mMinFlareSize = 40;
Real mMaxFlareSize = 80;
Controller<Real>* mController = 0;

// New depth shadowmapping
String CUSTOM_ROCKWALL_MATERIAL("Ogre/DepthShadowmap/Receiver/RockWall");
String CUSTOM_CASTER_MATERIAL("Ogre/DepthShadowmap/Caster/Float");
String CUSTOM_RECEIVER_MATERIAL("Ogre/DepthShadowmap/Receiver/Float");
String CUSTOM_ATHENE_MATERIAL("Ogre/DepthShadowmap/Receiver/Athene");

String BASIC_ROCKWALL_MATERIAL("Examples/Rockwall");
String BASIC_ATHENE_MATERIAL("Examples/Athene/NormalMapped");

/** This class 'wibbles' the light and billboard */
class LightWibbler : public ControllerValue<Real>
{
protected:
    Light* mLight;
    Billboard* mBillboard;
    ColourValue mColourRange;
    ColourValue mMinColour;
    Real mMinSize;
    Real mSizeRange;
    Real intensity;
public:
    LightWibbler(Light* light, Billboard* billboard, const ColourValue& minColour, 
        const ColourValue& maxColour, Real minSize, Real maxSize)
    {
        mLight = light;
        mBillboard = billboard;
        mMinColour = minColour;
        mColourRange.r = maxColour.r - minColour.r;
        mColourRange.g = maxColour.g - minColour.g;
        mColourRange.b = maxColour.b - minColour.b;
        mMinSize = minSize;
        mSizeRange = maxSize - minSize;
		
    }

    virtual Real  getValue (void) const
    {
        return intensity;
    }

    virtual void  setValue (Real value)
    {
        intensity = value;

        ColourValue newColour;

        // Attenuate the brightness of the light
        newColour.r = mMinColour.r + (mColourRange.r * intensity);
        newColour.g = mMinColour.g + (mColourRange.g * intensity);
        newColour.b = mMinColour.b + (mColourRange.b * intensity);

        mLight->setDiffuseColour(newColour);
        mBillboard->setColour(newColour);
        // set billboard size
        Real newSize = mMinSize + (intensity * mSizeRange);
        mBillboard->setDimensions(newSize, newSize);

    }
};

Real timeDelay = 0;


class _OgreSampleClassExport Sample_Shadows : public SdkSample
{
protected:
	enum ShadowProjection
	{
		UNIFORM,
		UNIFORM_FOCUSED,
		LISPSM,
		PLANE_OPTIMAL
	};

	enum ShadowMaterial
	{
		MAT_STANDARD,
		MAT_DEPTH_FLOAT,
		MAT_DEPTH_FLOAT_PCF
	};
	
	ShadowTechnique mCurrentShadowTechnique;
	ShadowProjection mCurrentProjection;
	ShadowMaterial mCurrentMaterial;

	GpuProgramParametersSharedPtr mCustomRockwallVparams;
	GpuProgramParametersSharedPtr mCustomRockwallFparams;
	GpuProgramParametersSharedPtr mCustomAtheneVparams;
	GpuProgramParametersSharedPtr mCustomAtheneFparams;

	ShadowCameraSetupPtr mCurrentShadowCameraSetup;
	/// Plane that defines plane-optimal shadow mapping basis
	MovablePlane* mPlane;
	// transient pointer to LiSPSM setup if present
	LiSPSMShadowCameraSetup* mLiSPSMSetup;

	bool mIsOpenGL;

public:

	bool frameEnded(const FrameEvent& evt)
    {
		if (mAnimState)
				mAnimState->addTime(evt.timeSinceLastFrame);
		return SdkSample::frameEnded(evt);
	}

	Sample_Shadows() : 
	  mPlane(0)
	{
		mInfo["Title"] = "Shadows";
		mInfo["Description"] = "A demonstration of ogre's various shadowing techniques.";
		mInfo["Thumbnail"] = "thumb_shadows.png";
		mInfo["Category"] = "Lighting";

    }

    ~Sample_Shadows() 
    {
		//mDescWindow = 0;
		
		delete mPlane;
    }
protected:

	// Override this to ensure FPU mode
	//bool configure(void)
	//{
	//	// Show the configuration dialog and initialise the system
	//	// You can skip this and use root.restoreConfig() to load configuration
	//	// settings if you were sure there are valid ones saved in ogre.cfg
	//	if(mRoot->showConfigDialog())
	//	{
	//		// Custom option - to use PlaneOptimalShadowCameraSetup we must have
	//		// double-precision. Thus, set the D3D floating point mode if present, 
	//		// no matter what was chosen
	//		ConfigOptionMap& optMap = mRoot->getRenderSystem()->getConfigOptions();
	//		ConfigOptionMap::iterator i = optMap.find("Floating-point mode");
	//		if (i != optMap.end())
	//		{
	//			if (i->second.currentValue != "Consistent")
	//			{
	//				i->second.currentValue = "Consistent";
	//				LogManager::getSingleton().logMessage("Demo_Shadows: overriding "
	//					"D3D floating point mode to 'Consistent' to ensure precision "
	//					"for plane-optimal camera setup option");

	//			}
	//		}

	//		// If returned true, user clicked OK so initialise
	//		// Here we choose to let the system create a default rendering window by passing 'true'
	//		mWindow = mRoot->initialise(true);
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}
	
    // Just override the mandatory create scene method
    void setupContent(void)
    {
		// Need to detect D3D or GL for best depth shadowmapping
		mIsOpenGL = Root::getSingleton().getRenderSystem()->getName().find("GL") != String::npos;

		// do this first so we generate edge lists
		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_HWSTENCIL))
        {
            mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
            mCurrentShadowTechnique = SHADOWTYPE_STENCIL_ADDITIVE;
        }
        else
        {
            mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
            mCurrentShadowTechnique = SHADOWTYPE_TEXTURE_MODULATIVE;
        }
        // Set ambient light off
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

        // Fixed light, dim
        mSunLight = mSceneMgr->createLight("SunLight");
        mSunLight->setType(Light::LT_SPOTLIGHT);
        mSunLight->setPosition(1500,1750,1300);
        mSunLight->setSpotlightRange(Degree(30), Degree(50));
        Vector3 dir;
        dir = -mSunLight->getPosition();
        dir.normalise();
        mSunLight->setDirection(dir);
        mSunLight->setDiffuseColour(0.35, 0.35, 0.38);
        mSunLight->setSpecularColour(0.9, 0.9, 1);

        // Point light, movable, reddish
        mLight = mSceneMgr->createLight("Light2");
        mLight->setDiffuseColour(mMinLightColour);
        mLight->setSpecularColour(1, 1, 1);
        mLight->setAttenuation(8000,1,0.0005,0);

        // Create light node
        mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(
            "MovingLightNode");
        mLightNode->attachObject(mLight);
        // create billboard set
        BillboardSet* bbs = mSceneMgr->createBillboardSet("lightbbs", 1);
        bbs->setMaterialName("Examples/Flare");
        Billboard* bb = bbs->createBillboard(0,0,0,mMinLightColour);
        // attach
        mLightNode->attachObject(bbs);

        // create controller, after this is will get updated on its own
        ControllerFunctionRealPtr func = ControllerFunctionRealPtr(
            new WaveformControllerFunction(Ogre::WFT_SINE, 0.75, 0.5));
        ControllerManager& contMgr = ControllerManager::getSingleton();
        ControllerValueRealPtr val = ControllerValueRealPtr(
            new LightWibbler(mLight, bb, mMinLightColour, mMaxLightColour, 
            mMinFlareSize, mMaxFlareSize));
        mController = contMgr.createController(
            contMgr.getFrameTimeSource(), val, func);
		
        //mLight->setPosition(Vector3(300,250,-300));
        mLightNode->setPosition(Vector3(300,1750,-700));


        // Create a track for the light
        Animation* anim = mSceneMgr->createAnimation("LightTrack", 20);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, mLightNode);
        // Setup keyframes
        TransformKeyFrame* key = track->createNodeKeyFrame(0); // A startposition
        key->setTranslate(Vector3(300,750,-700));
        key = track->createNodeKeyFrame(2);//B
        key->setTranslate(Vector3(150,800,-250));
        key = track->createNodeKeyFrame(4);//C
        key->setTranslate(Vector3(-150,850,-100));
        key = track->createNodeKeyFrame(6);//D
        key->setTranslate(Vector3(-400,700,-200));
        key = track->createNodeKeyFrame(8);//E
        key->setTranslate(Vector3(-200,700,-400));
        key = track->createNodeKeyFrame(10);//F
        key->setTranslate(Vector3(-100,850,-200));
        key = track->createNodeKeyFrame(12);//G
        key->setTranslate(Vector3(-100,575,180));
        key = track->createNodeKeyFrame(14);//H
        key->setTranslate(Vector3(0,750,300));
        key = track->createNodeKeyFrame(16);//I
        key->setTranslate(Vector3(100,850,100));
        key = track->createNodeKeyFrame(18);//J
        key->setTranslate(Vector3(250,800,0));
        key = track->createNodeKeyFrame(20);//K == A
        key->setTranslate(Vector3(300,750,-700));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("LightTrack");
        mAnimState->setEnabled(true);
        // Make light node look at origin, this is for when we
        // change the moving light to a spotlight
        mLightNode->setAutoTracking(true, mSceneMgr->getRootSceneNode());

        // Prepare athene mesh for normalmapping
        MeshPtr pAthene = MeshManager::getSingleton().load("athene.mesh", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        unsigned short src, dest;
        if (!pAthene->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
        {
            pAthene->buildTangentVectors(VES_TANGENT, src, dest);
        }

        SceneNode* node;
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mAthene = mSceneMgr->createEntity( "athene", "athene.mesh" );
        mAthene->setMaterialName(BASIC_ATHENE_MATERIAL);
        node->attachObject( mAthene );
        node->translate(0,-27, 0);
        node->yaw(Degree(90));

        Entity* pEnt;
		// Columns
		for (int x = -2; x <= 2; ++x)
		{
			for (int z = -2; z <= 2; ++z)
			{
				if (x != 0 || z != 0)
				{
					StringUtil::StrStreamType str;
					str << "col" << x << "_" << z;
					node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					pEnt = mSceneMgr->createEntity( str.str(), "column.mesh" );
					pEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
					pColumns.push_back(pEnt);
					node->attachObject( pEnt );
					node->translate(x*300,0, z*300);

				}
			}

		}


        // Skybox
        mSceneMgr->setSkyBox(true, "Examples/StormySkyBox");

        // Floor plane (use POSM plane def)
		mPlane = new MovablePlane("*mPlane");
        mPlane->normal = Vector3::UNIT_Y;
        mPlane->d = 107;
        MeshManager::getSingleton().createPlane("Myplane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *mPlane,
            1500,1500,50,50,true,1,5,5,Vector3::UNIT_Z);
        pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_HWRENDER_TO_TEXTURE))
        {
            // In D3D, use a 1024x1024 shadow texture
            mSceneMgr->setShadowTextureSettings(1024, 2);
        }
        else
        {
            // Use 512x512 texture in GL since we can't go higher than the window res
            mSceneMgr->setShadowTextureSettings(512, 2);
        }

        mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
        //mSceneMgr->setShowDebugShadows(true);

		setupGUI();
		setDragLook(true);
    }

	virtual void setupView()
	{
		SdkSample::setupView();

        // incase infinite far distance is not supported
        mCamera->setFarClipDistance(100000);

		mCamera->setPosition(250, 20, 400);
		mCamera->lookAt(0, 10, 0);
	}
	
	virtual void cleanupContent()
	{
		ControllerManager::getSingleton().destroyController(mController);
	}

	/// Change basic shadow technique 
	void changeShadowTechnique(ShadowTechnique newTech)
	{
		mSceneMgr->setShadowTechnique(newTech);

		// Below is for projection
		//configureShadowCameras(mCurrentShadowTechnique, newTech);

		configureLights(newTech);

		// Advanced modes - materials / compositors
		//configureCompositors(mCurrentShadowTechnique, newTech);
		//configureTextures(mCurrentShadowTechnique, newTech);
		//configureShadowCasterReceiverMaterials(mCurrentShadowTechnique, newTech);

		updateGUI(newTech);

		mCurrentShadowTechnique = newTech;
	}

	void configureLights(ShadowTechnique newTech)
	{
		Vector3 dir;
		switch (newTech)
		{
		case SHADOWTYPE_STENCIL_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(true);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);

			break;
		case SHADOWTYPE_STENCIL_MODULATIVE:
			// Multiple lights cause obvious silhouette edges in modulative mode
			// So turn off shadows on the direct light
			// Fixed light, dim
			mSunLight->setCastShadows(false);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			break;
		case SHADOWTYPE_TEXTURE_MODULATIVE:
		case SHADOWTYPE_TEXTURE_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(true);

			// Change moving light to spotlight
			// Point light, movable, reddish
			mLight->setType(Light::LT_SPOTLIGHT);
			mLight->setDirection(Vector3::NEGATIVE_UNIT_Z);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			mLight->setSpotlightRange(Degree(80),Degree(90));


			break;
		default:
			break;
		};

	}

	SelectMenu* mTechniqueMenu;
	SelectMenu* mLightingMenu;
	SelectMenu* mProjectionMenu;
	SelectMenu* mMaterialMenu;
	
	Slider* mFixedBiasSlider;
	Slider* mSlopedBiasSlider;
	Slider* mClampSlider;

	void setupGUI()
	{
		mTechniqueMenu = mTrayMgr->createLongSelectMenu(
			TL_TOPLEFT, "TechniqueSelectMenu", "Technique", 300, 200, 5);
		mTechniqueMenu->addItem("Stencil");
		mTechniqueMenu->addItem("Texture");

		mLightingMenu = mTrayMgr->createLongSelectMenu(
			TL_TOPLEFT, "LightingSelectMenu", "Lighting", 300, 200, 5);
		mLightingMenu->addItem("Additive");
		mLightingMenu->addItem("Modulative");

		//These values are synchronized with ShadowProjection enum
		mProjectionMenu = mTrayMgr->createLongSelectMenu(
			TL_TOPLEFT, "ProjectionSelectMenu", "Projection", 300, 200, 5);
		mProjectionMenu->addItem("Uniform");
		mProjectionMenu->addItem("Uniform Focused");
		mProjectionMenu->addItem("LiSPSM");
		mProjectionMenu->addItem("Plane Optimal");
		
		mMaterialMenu = mTrayMgr->createLongSelectMenu(
			TL_TOPLEFT, "MaterialSelectMenu", "Material", 300, 200, 5);
		mMaterialMenu->addItem("Standard");
		mMaterialMenu->addItem("Depth Shadowmap");
		mMaterialMenu->addItem("Depth Shadowmap (PCF)");
		
		mFixedBiasSlider = mTrayMgr->createThickSlider(TL_NONE, "FixedBiasSlider", "Fixed Bias", 256, 80, 0, 0.02, 100);
		mFixedBiasSlider->setValue(0.0009, false);
		mFixedBiasSlider->hide();

		mSlopedBiasSlider = mTrayMgr->createThickSlider(TL_NONE, "SlopedBiasSlider", "Sloped Bias", 256, 80, 0, 0.2, 100);
		mSlopedBiasSlider->setValue(0.0008, false);
		mSlopedBiasSlider->hide();

		mClampSlider = mTrayMgr->createThickSlider(TL_NONE, "SlopeClampSlider", "Slope Clamp", 256, 80, 0, 2, 100);
		mClampSlider->setValue(0.2, false);
		mClampSlider->hide();

		updateGUI(mCurrentShadowTechnique);
		mTrayMgr->showCursor();
	}

	void updateGUI(ShadowTechnique newTech)
	{
		bool isTextureBased = (newTech & SHADOWDETAILTYPE_TEXTURE) != 0;

		if (isTextureBased) 
		{
			mProjectionMenu->show();
			mTrayMgr->moveWidgetToTray(mProjectionMenu, TL_TOPLEFT);
			mMaterialMenu->show();
			mTrayMgr->moveWidgetToTray(mMaterialMenu, TL_TOPLEFT);
		}
		else 
		{
			mProjectionMenu->hide();
			mTrayMgr->removeWidgetFromTray(mProjectionMenu);
			mMaterialMenu->hide();
			mTrayMgr->removeWidgetFromTray(mMaterialMenu);
		}
	}

	void itemSelected(SelectMenu* menu)
	{
		if (menu == mTechniqueMenu) handleShadowTypeChanged();
		else if (menu == mLightingMenu) handleShadowTypeChanged();
		else if (menu == mProjectionMenu) handleProjectionChanged();
		else if (menu == mMaterialMenu) handleMaterialChanged();
	}


	void handleShadowTypeChanged()
	{
		bool isStencil = mTechniqueMenu->getSelectionIndex() == 0;
		bool isAdditive = mLightingMenu->getSelectionIndex() == 0;
		ShadowTechnique newTech = mCurrentShadowTechnique;

		if (isStencil)
		{
			newTech = static_cast<ShadowTechnique>(
				(newTech & ~SHADOWDETAILTYPE_TEXTURE) | SHADOWDETAILTYPE_STENCIL);
			resetMaterials();
		}
		else
		{
			newTech = static_cast<ShadowTechnique>(
				(newTech & ~SHADOWDETAILTYPE_STENCIL) | SHADOWDETAILTYPE_TEXTURE);
		}
		
		if (isAdditive)
		{
			newTech = static_cast<ShadowTechnique>(
				(newTech & ~SHADOWDETAILTYPE_MODULATIVE) | SHADOWDETAILTYPE_ADDITIVE);
		}
		else
		{
			newTech = static_cast<ShadowTechnique>(
				(newTech & ~SHADOWDETAILTYPE_ADDITIVE) | SHADOWDETAILTYPE_MODULATIVE);
		}

		changeShadowTechnique(newTech);
	}

	void handleProjectionChanged()
	{
		ShadowProjection proj = (ShadowProjection)mProjectionMenu->getSelectionIndex();
		
		if (proj != mCurrentProjection)
		{
			switch(proj)
			{
			case UNIFORM:
				mCurrentShadowCameraSetup = 
					ShadowCameraSetupPtr(new DefaultShadowCameraSetup());
				break;
			case UNIFORM_FOCUSED:
				mCurrentShadowCameraSetup = 
					ShadowCameraSetupPtr(new FocusedShadowCameraSetup());
				break;
			case LISPSM:
				{
					mLiSPSMSetup = new LiSPSMShadowCameraSetup();
					//mLiSPSMSetup->setUseAggressiveFocusRegion(false);
					mCurrentShadowCameraSetup = ShadowCameraSetupPtr(mLiSPSMSetup);
				}
				break;
			case PLANE_OPTIMAL:
				mCurrentShadowCameraSetup = 
					ShadowCameraSetupPtr(new PlaneOptimalShadowCameraSetup(mPlane));
				break;

			};
			mCurrentProjection = proj;

			mSceneMgr->setShadowCameraSetup(mCurrentShadowCameraSetup);

			//updateTipForCombo(cbo);
			if (!mCustomRockwallVparams.isNull() && !mCustomRockwallFparams.isNull())
			{
				// set
				setDefaultDepthShadowParams();
			}

		}
	}

	void updateDepthShadowParams()
	{
		mCustomRockwallFparams->setNamedConstant("fixedDepthBias", 
			mFixedBiasSlider->getValue());
		mCustomRockwallFparams->setNamedConstant("gradientScaleBias",
			mSlopedBiasSlider->getValue());
		mCustomRockwallFparams->setNamedConstant("gradientClamp",
			mClampSlider->getValue());

		mCustomAtheneFparams->setNamedConstant("fixedDepthBias", 
			mFixedBiasSlider->getValue());
		mCustomAtheneFparams->setNamedConstant("gradientScaleBias",
			mSlopedBiasSlider->getValue());
		mCustomAtheneFparams->setNamedConstant("gradientClamp",
			mClampSlider->getValue());
	}

	void setDefaultDepthShadowParams()
	{
		switch(mCurrentProjection)
		{
		case UNIFORM:
		case UNIFORM_FOCUSED:
		case PLANE_OPTIMAL:
			mFixedBiasSlider->setValue(0.0f, false);
			mSlopedBiasSlider->setValue(0.0f, false);
			break;
		case LISPSM:
			mFixedBiasSlider->setValue(0.009f, false);
			mSlopedBiasSlider->setValue(0.04f, false);
			break;
		};

		updateDepthShadowParams();

	}

	void sliderMoved(Slider* slider)
	{
		if (!mCustomRockwallVparams.isNull() && !mCustomRockwallFparams.isNull())
		{
			updateDepthShadowParams();
		}
	}

	void rebindDebugShadowOverlays()
	{
		/*MaterialPtr debugMat = 
			MaterialManager::getSingleton().getByName("Ogre/DebugShadowMap0");
		TexturePtr shadowTex = mSceneMgr->getShadowTexture(0);
		debugMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(shadowTex->getName());

		debugMat = 
			MaterialManager::getSingleton().getByName("Ogre/DebugShadowMap1");
		shadowTex = mSceneMgr->getShadowTexture(1);
		debugMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(shadowTex->getName());*/
	}

	void resetMaterials()
	{
		// Sort out base materials
		pPlaneEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
		mAthene->setMaterialName(BASIC_ATHENE_MATERIAL);
		for (vector<Entity*>::type::iterator i = pColumns.begin();
			i != pColumns.end(); ++i)
		{
			(*i)->setMaterialName(BASIC_ROCKWALL_MATERIAL);
		}

		mCustomRockwallVparams.setNull();
		mCustomRockwallFparams.setNull();
		mCustomAtheneVparams.setNull();
		mCustomAtheneFparams.setNull();

	}
	
	void handleMaterialChanged()
	{
		bool showSliders = false;	
		ShadowMaterial mat = (ShadowMaterial)mMaterialMenu->getSelectionIndex();
		MaterialPtr themat;
		if (mat != mCurrentMaterial)
		{
			switch(mat)
			{
			case MAT_STANDARD:
				mSceneMgr->setShadowTexturePixelFormat(PF_X8R8G8B8);
				mSceneMgr->setShadowTextureCasterMaterial(StringUtil::BLANK);
				mSceneMgr->setShadowTextureReceiverMaterial(StringUtil::BLANK);
				mSceneMgr->setShadowTextureSelfShadow(false);	
				
				resetMaterials();

				break;
			case MAT_DEPTH_FLOAT:
				if (mIsOpenGL)
				{
					// GL performs much better if you pick half-float format
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT16_R);
				}
				else
				{
					// D3D is the opposite - if you ask for PF_FLOAT16_R you
					// get an integer format instead! You can ask for PF_FLOAT16_GR
					// but the precision doesn't work well
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT32_R);
				}
				mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
				mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL);
				mSceneMgr->setShadowTextureSelfShadow(true);	
				// Sort out base materials
				pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
				mAthene->setMaterialName(CUSTOM_ATHENE_MATERIAL);
				for (vector<Entity*>::type::iterator i = pColumns.begin();
					i != pColumns.end(); ++i)
				{
					(*i)->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
				}

				themat = MaterialManager::getSingleton().getByName(CUSTOM_ROCKWALL_MATERIAL);
				mCustomRockwallVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
				mCustomRockwallFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();
				themat = MaterialManager::getSingleton().getByName(CUSTOM_ATHENE_MATERIAL);
				mCustomAtheneVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
				mCustomAtheneFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();
				showSliders = true;


				// set the current params
				setDefaultDepthShadowParams();
				break;
			case MAT_DEPTH_FLOAT_PCF:
				if (mIsOpenGL)
				{
					// GL performs much better if you pick half-float format
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT16_R);
				}
				else
				{
					// D3D is the opposite - if you ask for PF_FLOAT16_R you
					// get an integer format instead! You can ask for PF_FLOAT16_GR
					// but the precision doesn't work well
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT32_R);
				}
				mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
				mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL + "/PCF");
				mSceneMgr->setShadowTextureSelfShadow(true);	
				// Sort out base materials
				pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
				mAthene->setMaterialName(CUSTOM_ATHENE_MATERIAL + "/PCF");
				for (vector<Entity*>::type::iterator i = pColumns.begin();
					i != pColumns.end(); ++i)
				{
					(*i)->setMaterialName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
				}

				themat = MaterialManager::getSingleton().getByName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
				mCustomRockwallVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
				mCustomRockwallFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();
				themat = MaterialManager::getSingleton().getByName(CUSTOM_ATHENE_MATERIAL + "/PCF");
				mCustomAtheneVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
				mCustomAtheneFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();
				showSliders = true;

				// set the current params
				setDefaultDepthShadowParams();
				break;
			};
			mCurrentMaterial = mat;

			if (showSliders)
			{
				mFixedBiasSlider->show();
				mTrayMgr->moveWidgetToTray(mFixedBiasSlider, TL_TOPRIGHT);
				mSlopedBiasSlider->show();
				mTrayMgr->moveWidgetToTray(mSlopedBiasSlider, TL_TOPRIGHT);
				mClampSlider->show();	
				mTrayMgr->moveWidgetToTray(mClampSlider, TL_TOPRIGHT);			
			}
			else
			{
				mFixedBiasSlider->hide();
				mTrayMgr->removeWidgetFromTray(mFixedBiasSlider);
				mSlopedBiasSlider->hide();
				mTrayMgr->removeWidgetFromTray(mSlopedBiasSlider);
				mClampSlider->hide();		
				mTrayMgr->removeWidgetFromTray(mClampSlider);		
			}
			//updateTipForCombo(cbo);
			//rebindDebugShadowOverlays();
		}
	}
};
