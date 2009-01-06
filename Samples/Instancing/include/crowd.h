


#include "CEGUI/CEGUI.h"
#include "OgreCEGUIRenderer.h"
#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreFrameListener.h"
#include "ExampleApplication.h"
#include "OgreInstancedGeometry.h"

using namespace Ogre;

#define maxObjectsPerBatch 4
const size_t numTypeMeshes = 6;
#define FLT_MAX         3.402823466e+38F        /* max value */
class CrowdApplication;

enum CurrentGeomOpt{
	INSTANCE_OPT,
	ENTITY_OPT
};

CEGUI::MouseButton convertOISMouseButtonToCegui(int buttonID)
{
  switch (buttonID)
    {
	case 0: return CEGUI::LeftButton;
	case 1: return CEGUI::RightButton;
	case 2:	return CEGUI::MiddleButton;
	case 3: return CEGUI::X1Button;
	default: return CEGUI::LeftButton;
    }
}
//out of the listener class...if set as a member, the sample crashes when moving the mouse!

// Event handler to add ability to alter subdivision
class CrowdListener : public ExampleFrameListener, public OIS::KeyListener, public OIS::MouseListener
{
protected:
	CrowdApplication * mMain;
	bool mRequestShutDown;
	bool mLMBDown;
	bool mRMBDown;
	double mAvgFrameTime;
	size_t meshSelected;
	size_t numMesh;
	size_t objectCount;
	String mDebugText;

	CurrentGeomOpt currentGeomOpt;

	size_t numRender;

	Ogre::Timer*timer;
	double mLastTime,mBurnAmount;

	vector <InstancedGeometry *>::type		renderInstance;
	vector <StaticGeometry *>::type	renderStatic;
	vector <Entity *>::type			renderEntity;
	vector <AnimationState*>::type animations;

	vector <SceneNode *>::type			nodes; 
	vector <Vector3 *>::type			posMatrices;

	CEGUI::Renderer* mGUIRenderer;
	CEGUI::Window* mGuiAvg;
	CEGUI::Window* mGuiCurr;
	CEGUI::Window* mGuiBest;
	CEGUI::Window* mGuiWorst;
	CEGUI::Window* mGuiTris;
	CEGUI::Window* mGuiDbg;
	CEGUI::Window* mRoot;
	CEGUI::Point mLastMousePosition;
public:

	//-----------------------------------------------------------------------
	CrowdListener(RenderWindow* win, Camera* cam,CEGUI::Renderer* renderer, CrowdApplication*main);
	//-----------------------------------------------------------------------
	~CrowdListener();
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
	void setupInstancedMaterialToEntity(Entity*);
	//-----------------------------------------------------------------------
	String buildCrowdMaterial(const String &,int);
	//-----------------------------------------------------------------------
	void createStaticGeom();
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
	void changSelectedMesh(size_t number)
	{	
		meshSelected=number;
	}
	bool mouseMoved( const OIS::MouseEvent &arg );
	//----------------------------------------------------------------//
	bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	//----------------------------------------------------------------//
	bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	 //-----------------------------------------------------------------------
	void mouseReleased (OIS::MouseEvent *e);
	void mouseClicked(OIS::MouseEvent* e);
	void mouseEntered(OIS::MouseEvent* e) ;
	void mouseExited(OIS::MouseEvent* e);
	void requestShutdown(void);
	void setCurrentGeometryOpt(CurrentGeomOpt opt);
	bool handleMouseMove(const CEGUI::EventArgs& e);
	//--------------------------------------------------------------------------
	bool handleMouseButtonUp(const CEGUI::EventArgs& e);
	//--------------------------------------------------------------------------
	bool handleMouseButtonDown(const CEGUI::EventArgs& e);
	//--------------------------------------------------------------------------
	void updateStats(void);
	//----------------------------------------------------------------//
	bool keyPressed( const OIS::KeyEvent &arg )
	{
		if( arg.key == OIS::KC_ESCAPE )
			mRequestShutDown = true;
		return true;
	}

	//----------------------------------------------------------------//
	bool keyReleased( const OIS::KeyEvent &arg )
	{
		return true;
	}

};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
class CrowdApplication : public ExampleApplication
{

public:
	//-----------------------------------------------------------------------
	CrowdApplication():mGUIRenderer(0),
        mGUISystem(0),
        mEditorGuiSheet(0)  { }
	//-----------------------------------------------------------------------
	~CrowdApplication()
	{
		 if(mEditorGuiSheet)
       {
           CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);
       }
       if(mGUISystem)
       {
           delete mGUISystem;
           mGUISystem = 0;
       }
       if(mGUIRenderer)
       {
           delete mGUIRenderer;
           mGUIRenderer = 0;
       }
	
	}
	RenderWindow*getRenderWindow(void)
	{
		return mWindow;
	}

protected:
   CEGUI::OgreCEGUIRenderer* mGUIRenderer;
   CEGUI::System* mGUISystem;
   CEGUI::Window* mEditorGuiSheet;
	//-----------------------------------------------------------------------
	// Just override the mandatory scene manager choice method
      virtual void chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
    }
	//-----------------------------------------------------------------------
	// Just override the mandatory create scene method

	void createScene(void)
	{
		String terrain_cfg("terrain.cfg");
        mSceneMgr -> setWorldGeometry( terrain_cfg );
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

		Light* l = mSceneMgr->createLight("MainLight");

		//add a skybox
		mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox", 1000);

		//setup the light
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(-0.3, -1, 0.2);
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(1.0, 1.0, 0.8);

		mCamera->setPosition(700,100, 500);
		mCamera->lookAt(600,80,600);

		mSceneMgr->setShadowTextureSize(1024);
		setupGUI();
		setupEventHandlers();
		//set up the compositor
		Ogre::Viewport* pViewport = mWindow->getViewport(0);
		pViewport->setCamera(mCamera);
		CompositorManager::getSingleton().addCompositor(pViewport,"Bloom");
	}
	//-----------------------------------------------------------------------
	void destroyScene(void)
	{
	}
	//-----------------------------------------------------------------------
	void createFrameListener(void)
	{
		// This is where we instantiate our own frame listener
		mFrameListener= new CrowdListener(mWindow, mCamera,mGUIRenderer,this);
		mRoot->addFrameListener(mFrameListener);
	}
	//-----------------------------------------------------------------------
	void setupResources(void)
	{
		ExampleApplication::setupResources();

		/*ResourceGroupManager *rsm = ResourceGroupManager::getSingletonPtr ();
		StringVector groups = rsm->getResourceGroups ();        
		if (std::find(groups.begin(), groups.end(), String("Crowd")) == groups.end())
		{
			rsm->createResourceGroup("Crowd");
			try
			{
				rsm->addResourceLocation("../../../../../ogreaddons/Crowd/Media/materials/programs","FileSystem", "Crowd");
				rsm->addResourceLocation("../../../../../ogreaddons/Crowd/Media/materials/scripts","FileSystem", "Crowd");
				rsm->addResourceLocation("../../../../../ogreaddons/Crowd/Media/models","FileSystem", "Crowd");
			}
			catch (Ogre::Exception& e)
			{
				String error = e.getFullDescription();
				rsm->addResourceLocation("../../../Crowd/Media/materials/programs","FileSystem", "Crowd");
				rsm->addResourceLocation("../../../Crowd/Media/materials/scripts","FileSystem", "Crowd");
				rsm->addResourceLocation("../../../Crowd/Media/models","FileSystem", "Crowd");
			}
		}*/
	}
	void setupGUI(void)
	{
		// Set up GUI system
       mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, true, 3000, mSceneMgr);
       mGUISystem = new CEGUI::System(mGUIRenderer);
       CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

	   CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
       mGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
       CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseMoveCursor");
       mEditorGuiSheet= CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"DefaultWindow", (CEGUI::utf8*)"Sheet");  
       mGUISystem->setGUISheet(mEditorGuiSheet);
	 


	    mEditorGuiSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"CrowdDemo.layout");
		mGUISystem->setGUISheet(mEditorGuiSheet);
	}
	void setupEventHandlers(void)
	{
		using namespace CEGUI;
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		wmgr.getWindow((CEGUI::utf8*)"ExitDemoBtn")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CrowdApplication::handleQuit, this));

		wmgr.getWindow((CEGUI::utf8*)"tInstancing")->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&CrowdApplication::handleTechniqueChanged, this));
		wmgr.getWindow((CEGUI::utf8*)"tIndependantEntities")->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&CrowdApplication::handleTechniqueChanged, this));
		wmgr.getWindow((CEGUI::utf8*)"Object Count")->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,CEGUI::Event::Subscriber(&CrowdApplication::handleObjectCountChanged, this));
		wmgr.getWindow((CEGUI::utf8*)"Shadows")->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,CEGUI::Event::Subscriber(&CrowdApplication::handleShadowsChanged, this));
		
		wmgr.getWindow((CEGUI::utf8*)"Time Burner")->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged,CEGUI::Event::Subscriber(&CrowdApplication::handleTimeBurnerChanged, this));
		wmgr.getWindow((CEGUI::utf8*)"PostEffect")->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,CEGUI::Event::Subscriber(&CrowdApplication::handlePostEffectChanged, this));


		CEGUI::Window* wndw = CEGUI::WindowManager::getSingleton().getWindow("root");

		wndw->subscribeEvent(Window::EventMouseMove, CEGUI::Event::Subscriber(&CrowdApplication::handleMouseMove,this));

		wndw->subscribeEvent(Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&CrowdApplication::handleMouseButtonUp,this));

		wndw->subscribeEvent(Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&CrowdApplication::handleMouseButtonDown,this));

}
	//-----------------------------------------------------------------------
	bool handleQuit(const CEGUI::EventArgs& e)
	{
		static_cast<CrowdListener*>(mFrameListener)->requestShutdown();
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleMouseMove(const CEGUI::EventArgs& e)
	{
		static_cast<CrowdListener*>(mFrameListener)->handleMouseMove(e);
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleMouseButtonDown(const CEGUI::EventArgs& e)
	{
		static_cast<CrowdListener*>(mFrameListener)->handleMouseButtonDown(e);
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleMouseButtonUp(const CEGUI::EventArgs& e)
	{
		static_cast<CrowdListener*>(mFrameListener)->handleMouseButtonUp(e);
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleTechniqueChanged(const CEGUI::EventArgs& e)
	{

		static_cast<CrowdListener*>(mFrameListener)->destroyCurrentGeomOpt();
				
		CEGUI::uint id = ((CEGUI::RadioButton*)((const CEGUI::WindowEventArgs&)e).window)->getSelectedButtonInGroup()->getID();
		if (id == 0)
		{
			static_cast<CrowdListener*>(mFrameListener)->setCurrentGeometryOpt(INSTANCE_OPT);
		}
		if (id == 1)
		{
			static_cast<CrowdListener*>(mFrameListener)->setCurrentGeometryOpt(ENTITY_OPT);
		}
		static_cast<CrowdListener*>(mFrameListener)->createCurrentGeomOpt();
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleObjectCountChanged(const CEGUI::EventArgs& e)
	{
		static_cast<CrowdListener*>(mFrameListener)->destroyCurrentGeomOpt();
		float scrollval = ((CEGUI::Scrollbar*)((const CEGUI::WindowEventArgs&)e).window)->getScrollPosition();
		int objectCount = 1000*scrollval;
		static_cast<CrowdListener*>(mFrameListener)->setObjectCount(objectCount );
		static_cast<CrowdListener*>(mFrameListener)->createCurrentGeomOpt();
		String value=StringConverter::toString(objectCount);
		CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Object Count Number")->setText(CEGUI::String(value.c_str()));

		return true;
	}
	//-----------------------------------------------------------------------
	bool handleTimeBurnerChanged(const CEGUI::EventArgs& e)
	{
		
		float scrollval = ((CEGUI::Scrollbar*)((const CEGUI::WindowEventArgs&)e).window)->getScrollPosition();
		double timeBurned = 0.0166f*scrollval;
		char* timeChar= new char[10];
		sprintf(timeChar,"%0.1f ms",timeBurned*1000.0f);
		CEGUI::String timeText(timeChar);
		CEGUI::WindowManager::getSingleton().getWindow("Time Burner Value")->setText(timeText);

		static_cast<CrowdListener*>(mFrameListener)->setBurnedTime(timeBurned);
		delete[] timeChar;
		return true;
	}
	//-----------------------------------------------------------------------
	bool handlePostEffectChanged(const CEGUI::EventArgs& e)
	{
		Ogre::Viewport* pViewport = mWindow->getViewport(0);
		if(((CEGUI::Checkbox*)((const CEGUI::WindowEventArgs&)e).window)->isSelected())
		{
				CompositorManager::getSingleton().setCompositorEnabled(pViewport,"Bloom",true);
		}
		else
		{
				CompositorManager::getSingleton().setCompositorEnabled(pViewport,"Bloom",false);
		}
		return true;
	}
	//-----------------------------------------------------------------------
	bool handleShadowsChanged(const CEGUI::EventArgs&e)
	{
		if(((CEGUI::Checkbox*)((const CEGUI::WindowEventArgs&)e).window)->isSelected())
		{
			mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
			LiSPSMShadowCameraSetup* lispsm = new LiSPSMShadowCameraSetup();
			lispsm->setOptimalAdjustFactor(0.3);
			mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(lispsm));
			mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
		}
		else
		{
			mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
		}
		return true;
	}


	
};
