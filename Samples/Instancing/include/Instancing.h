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

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreFrameListener.h"
#include "OgreInstancedGeometry.h"
#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

#define maxObjectsPerBatch 80
#ifndef FLT_MAX
#  define FLT_MAX         3.402823466e+38F        /* max value */
#endif

const size_t numTypeMeshes = 4;
class Sample_Instancing;
Ogre::String meshes[]=
{ 
	"razor", //0
	"knot", 
	"tudorhouse",
	"WoodPallet"//6

};

enum CurrentGeomOpt{
	INSTANCE_OPT,
	STATIC_OPT,
	ENTITY_OPT
};


//out of the listener class...if set as a member, the sample crashes when moving the mouse!

// Event handler to add ability to alter subdivision
class InstancingListener : public FrameListener
{
protected:
	Sample_Instancing* mMain;
	double mAvgFrameTime;
	size_t meshSelected;
	size_t numMesh;
	size_t objectCount;
	String mDebugText;
	CurrentGeomOpt currentGeomOpt;
	
	Camera* mCamera;
	size_t numRender;

	Ogre::Timer*timer;
	double mLastTime,mBurnAmount;

	vector <InstancedGeometry *>::type		renderInstance;
	vector <StaticGeometry *>::type	renderStatic;
	vector <Entity *>::type			renderEntity;
	vector <SceneNode *>::type			nodes; 
	vector <Vector3 *>::type			posMatrices;

public:

	//-----------------------------------------------------------------------
	InstancingListener(Camera* cam, Sample_Instancing* main);
	//-----------------------------------------------------------------------
	~InstancingListener();
	//-----------------------------------------------------------------------
	bool frameRenderingQueued(const FrameEvent& evt);
	//-----------------------------------------------------------------------
	void burnCPU(void);
	//-----------------------------------------------------------------------
	void destroyCurrentGeomOpt();
	//-----------------------------------------------------------------------
	void createCurrentGeomOpt();
	//-----------------------------------------------------------------------
	void createInstanceGeom();
	//-----------------------------------------------------------------------
	void destroyInstanceGeom();
	//-----------------------------------------------------------------------
	void createStaticGeom();
	//-----------------------------------------------------------------------
	void setupInstancedMaterialToEntity(Entity*);
	//-----------------------------------------------------------------------
	String buildInstancedMaterial(const String &);
	//-----------------------------------------------------------------------
	void destroyStaticGeom();
	//-----------------------------------------------------------------------
	void createEntityGeom();
	//-----------------------------------------------------------------------
	void destroyEntityGeom();
	//-----------------------------------------------------------------------
	void setObjectCount(size_t val)
	{
		numMesh=val;
	};
	//-----------------------------------------------------------------------
	void setBurnedTime(double timeBurned)
	{
		mBurnAmount=timeBurned;
	};
	//-----------------------------------------------------------------------
	void changeSelectedMesh(size_t number)
	{	
		meshSelected=number;
	}
	 //-----------------------------------------------------------------------
	void requestShutdown(void);
	void setCurrentGeometryOpt(CurrentGeomOpt opt);
	//--------------------------------------------------------------------------
	
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
class _OgreSampleClassExport  Sample_Instancing : public SdkSample
{

public:
	#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchPressed(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseDown(evt)) return true;
		if (evt.state.touchIsType(OIS::MT_Pressed)) mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}

	bool touchReleased(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseUp(evt)) return true;
		if (evt.state.touchIsType(OIS::MT_Pressed)) mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}

	bool touchMoved(const OIS::MultiTouchEvent& evt)
	{
		// only rotate the camera if cursor is hidden
		if (mTrayMgr->isCursorVisible()) mTrayMgr->injectMouseMove(evt);
		else mCameraMan->injectMouseMove(evt);
		return true;
	}
#else
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseDown(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}
    
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseUp(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}
    
	bool mouseMoved(const OIS::MouseEvent& evt)
	{
		// only rotate the camera if cursor is hidden
		if (mTrayMgr->isCursorVisible()) mTrayMgr->injectMouseMove(evt);
		else mCameraMan->injectMouseMove(evt);
		return true;
	}
#endif

	//-----------------------------------------------------------------------
	Sample_Instancing()  
	{ 
		mInfo["Title"] = "Instancing";
		mInfo["Description"] = "A demo of different methods to handle a large number of objects.";
		mInfo["Thumbnail"] = "thumb_instancing.png";
		mInfo["Category"] = "Unsorted";
	}
	//-----------------------------------------------------------------------
	~Sample_Instancing()
	{
		 
	
	}
	RenderWindow*getRenderWindow(void)
	{
		return mWindow;
	}

	InstancingListener* mFrameListener;

protected:
   
	//-----------------------------------------------------------------------
	// Just override the mandatory create scene method
	void setupContent(void)
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
		Light* l = mSceneMgr->createLight("MainLight");
		//add a skybox
		mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox", 1000);
		//setup the light
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(-0.5, -0.5, 0);

		mCamera->setPosition(500,500, 1500);
		mCamera->lookAt(0,0,0);
		   Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            1500,1500,20,20,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("Examples/Rockwall");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
		
		CompositorManager::getSingleton().addCompositor(mViewport,"Bloom");

		setupControls();
		createFrameListener();
	}

	//-----------------------------------------------------------------------
	void cleanupContent(void)
	{

	}
	//-----------------------------------------------------------------------
	void createFrameListener(void)
	{
		// This is where we instantiate our own frame listener
		mFrameListener= new InstancingListener(mCamera, this);
		mRoot->addFrameListener(mFrameListener);
	}

	//-----------------------------------------------------------------------
	void setupControls(void)
	{
		SelectMenu* technique = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "TechniqueType", "Instancing Technique", 200, 3);
		technique->addItem("Instancing");
		technique->addItem("Static Geometry");
		technique->addItem("Independent Entities");

		mTrayMgr->createThickSlider(TL_TOPLEFT, "ObjectCountSlider", "Object count", 200, 50, 0, 1000, 101)->setValue(100, false);

		mTrayMgr->createCheckBox(TL_TOPLEFT, "ShadowCheckBox", "Shadows", 200);
		
		SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "ObjectType", "Object : ", 200, 4);
		objectType->addItem("razor");
		objectType->addItem("knot");
		objectType->addItem("tudorhouse");
		objectType->addItem("woodpallet");

		mTrayMgr->createThickSlider(TL_TOPLEFT, "CPUOccupationSlider", "CPU Load (ms)", 200, 75, 0, 1000.0f / 60, 20);

		mTrayMgr->createCheckBox(TL_TOPLEFT, "PostEffectCheckBox", "Post Effect", 200);

		mTrayMgr->showCursor();
	}
	
	void sliderMoved(Slider* slider)
	{
		if (slider->getName() == "ObjectCountSlider")
		{
			mFrameListener->destroyCurrentGeomOpt();
			mFrameListener->setObjectCount((size_t)slider->getValue());
			mFrameListener->createCurrentGeomOpt();
		}
		else if (slider->getName() == "CPUOccupationSlider")
		{
			mFrameListener->setBurnedTime(slider->getValue() / 1000.0f);
		}
	}

	void itemSelected(SelectMenu* menu)
	{
		if (menu->getName() == "TechniqueType")
		{
			//Menu items are synchronized with enum
			CurrentGeomOpt selectedOption = (CurrentGeomOpt)menu->getSelectionIndex();
			mFrameListener->destroyCurrentGeomOpt();
			mFrameListener->setCurrentGeometryOpt(selectedOption);
			mFrameListener->createCurrentGeomOpt();
		}
		else if (menu->getName() == "ObjectType")
		{
			mFrameListener->destroyCurrentGeomOpt();
			mFrameListener->changeSelectedMesh(menu->getSelectionIndex());
			mFrameListener->createCurrentGeomOpt();
		}
	}

	void checkBoxToggled(CheckBox* box)
	{
		if (box->getName() == "ShadowCheckBox")
		{
			if (box->isChecked())
			{
				mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
			}
			else
			{
				mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
			}
		} 
		else if (box->getName() == "PostEffectCheckBox")
		{
			CompositorManager::getSingleton().setCompositorEnabled(mViewport,"Bloom",box->isChecked());
		}
	}
	
	
};
