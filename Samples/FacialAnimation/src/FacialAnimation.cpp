/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        FacialAnimation.cpp
    \brief
        Demonstration of facial animation features, using Pose animation
*/

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/elements/CEGUICombobox.h>
#include <CEGUI/elements/CEGUIListbox.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
#include <CEGUI/elements/CEGUIRadioButton.h>
#include "OgreCEGUIRenderer.h"
#include "OgreCEGUIResourceProvider.h"

#include "ExampleApplication.h"

//----------------------------------------------------------------//
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

AnimationState* speakAnimState;
AnimationState* manualAnimState;
VertexPoseKeyFrame* manualKeyFrame;

enum ScrollbarIndex
{
	SI_HAPPY = 0,
	SI_SAD = 1,
	SI_ANGRY = 2,
	SI_A = 3,
	SI_E = 4,
	SI_I = 5,
	SI_O = 6,
	SI_U = 7,
	SI_C = 8,
	SI_W = 9,
	SI_M = 10,
	SI_L = 11,
	SI_F = 12,
	SI_T = 13,
	SI_P = 14,
	SI_R = 15,
	SI_S = 16,
	SI_TH = 17,
	SI_COUNT = 18
};
String scrollbarNames[SI_COUNT] = {
	"Facial/Happy_Scroll",
	"Facial/Sad_Scroll",
	"Facial/Angry_Scroll",
	"Facial/A_Scroll",
	"Facial/E_Scroll",
	"Facial/I_Scroll",
	"Facial/O_Scroll",
	"Facial/U_Scroll",
	"Facial/C_Scroll",
	"Facial/W_Scroll",
	"Facial/M_Scroll",
	"Facial/L_Scroll",
	"Facial/F_Scroll",
	"Facial/T_Scroll",
	"Facial/P_Scroll",
	"Facial/R_Scroll",
	"Facial/S_Scroll",
	"Facial/TH_Scroll",
};
unsigned short poseIndexes[SI_COUNT] = 
{ 1, 2, 3, 4, 7, 8, 6, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

CEGUI::Scrollbar* scrollbars[SI_COUNT];


class GuiFrameListener : public ExampleFrameListener, public OIS::MouseListener
{
private:
    CEGUI::Renderer* mGUIRenderer;
    bool mShutdownRequested;

public:
    // NB using buffered input, this is the only change
    GuiFrameListener(RenderWindow* win, Camera* cam, CEGUI::Renderer* renderer)
        : ExampleFrameListener(win, cam, false, true), 
          mGUIRenderer(renderer),
          mShutdownRequested(false)
    {
		mMouse->setEventCallback(this);
	}

    /// Tell the frame listener to exit at the end of the next frame
    void requestShutdown(void)
    {
        mShutdownRequested = true;
    }

    bool frameEnded(const FrameEvent& evt)
    {
        if (mShutdownRequested)
            return false;
        else
            return ExampleFrameListener::frameEnded(evt);
    }

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;
		speakAnimState->addTime(evt.timeSinceLastFrame);
		return true;

	}
	//----------------------------------------------------------------//
	bool mouseMoved( const OIS::MouseEvent &arg )
	{
		CEGUI::System::getSingleton().injectMouseMove( arg.state.X.rel, arg.state.Y.rel );
		return true;
	}

	//----------------------------------------------------------------//
	bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertOISMouseButtonToCegui(id));
		return true;
	}

	//----------------------------------------------------------------//
	bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertOISMouseButtonToCegui(id));
		return true;
	}

	//----------------------------------------------------------------//
	bool keyPressed( const OIS::KeyEvent &arg )
	{
		if( arg.key == OIS::KC_ESCAPE )
			mShutdownRequested = true;
		CEGUI::System::getSingleton().injectKeyDown( arg.key );
		CEGUI::System::getSingleton().injectChar( arg.text );
		return true;
	}

	//----------------------------------------------------------------//
	bool keyReleased( const OIS::KeyEvent &arg )
	{
		CEGUI::System::getSingleton().injectKeyUp( arg.key );
		return true;
	}
};

class FacialApplication : public ExampleApplication
{
private:
    CEGUI::OgreCEGUIRenderer* mGUIRenderer;
    CEGUI::System* mGUISystem;
    CEGUI::Window* mEditorGuiSheet;
	CEGUI::Scrollbar* mRed;
	CEGUI::Scrollbar* mGreen;
	CEGUI::Scrollbar* mBlue;
	CEGUI::Window* mPreview; // StaticImage
	CEGUI::Window* mTip;
	CEGUI::Listbox* mList;
	CEGUI::Window* mEditBox;

public:
    FacialApplication()
      : mGUIRenderer(0),
        mGUISystem(0),
        mEditorGuiSheet(0)
    {

	}

    ~FacialApplication()
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

protected:

	bool mPlayAnimation;

	// Handle the scrollbars changing
	bool handleScrollChanged(const CEGUI::EventArgs& e)
	{
		if (!mPlayAnimation)
		{
			// Alter the animation 
			// Find which one it is first
			const CEGUI::WindowEventArgs& args = static_cast<const CEGUI::WindowEventArgs&>(e);
			String name = args.window->getName().c_str();
			// Find which pose was changed
			int i;
			for (i = 0; i < SI_COUNT; ++i)
			{
				if (scrollbarNames[i] == name)
				{
					break;
				}
			}
			if (i != SI_COUNT)
			{
				// Update the pose
				manualKeyFrame->updatePoseReference(
					poseIndexes[i], scrollbars[i]->getScrollPosition());
				// Dirty animation state since we're fudging this manually
				manualAnimState->getParent()->_notifyDirty();
			}

		}
		return true;

	}

	// Handle play animation / manual tweaking event
	bool handleRadioChanged(const CEGUI::EventArgs& e)
	{
		mPlayAnimation = !mPlayAnimation;
		speakAnimState->setEnabled(mPlayAnimation);
		manualAnimState->setEnabled(!mPlayAnimation);
		for (int i = 0; i < SI_COUNT; ++i)
		{
			// enable / disable scrollbars
			scrollbars[i]->setEnabled(!mPlayAnimation);
		}

		return true;

	}

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);
		l->setDiffuseColour(1.0, 1.0, 1.0);

		// Create a light
		l = mSceneMgr->createLight("MainLight2");
		// Accept default settings: point light, white diffuse, just set position
		// NB I could attach the light to a SceneNode if I wanted it to move automatically with
		//  other objects, but I don't
		l->setPosition(-120,-80,-50);
		l->setDiffuseColour(0.7, 0.7, 0.6);


		// Pre-load the mesh so that we can tweak it with a manual animation
		MeshPtr mesh = MeshManager::getSingleton().load("facial.mesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Animation* anim = mesh->createAnimation("manual", 0);
		VertexAnimationTrack* track = anim->createVertexTrack(4, VAT_POSE);
		manualKeyFrame = track->createVertexPoseKeyFrame(0);
		// create pose references, initially zero
		for (int i = 0; i < SI_COUNT; ++i)
		{
			manualKeyFrame->addPoseReference(poseIndexes[i], 0.0f);
		}

		// setup GUI system
        mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, 
            Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);

        mGUISystem = new CEGUI::System(mGUIRenderer);

        CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

        Entity* head = mSceneMgr->createEntity("Head", "facial.mesh");
		speakAnimState = head->getAnimationState("Speak");
		speakAnimState->setEnabled(true);
		manualAnimState = head->getAnimationState("manual");
		manualAnimState->setTimePosition(0);

        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(head);

		mCamera->setPosition(-20, 50, 150);
		mCamera->lookAt(0,35,0);

        // load scheme and set up defaults
        CEGUI::SchemeManager::getSingleton().loadScheme(
                (CEGUI::utf8*)"TaharezLookSkin.scheme");
        mGUISystem->setDefaultMouseCursor(
                (CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mGUISystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

        CEGUI::Window* sheet = 
            CEGUI::WindowManager::getSingleton().loadWindowLayout(
                (CEGUI::utf8*)"facial.layout"); 
        mGUISystem->setGUISheet(sheet);

		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		for (int i = 0; i < SI_COUNT; ++i)
		{
			scrollbars[i] = static_cast<CEGUI::Scrollbar*>(
				wmgr.getWindow(scrollbarNames[i].c_str()));
			scrollbars[i]->subscribeEvent(
				CEGUI::Scrollbar::EventScrollPositionChanged, 
				CEGUI::Event::Subscriber(&FacialApplication::handleScrollChanged, this));
			// disable to begin with
			scrollbars[i]->setEnabled(false);

		}

		CEGUI::RadioButton* btn = static_cast<CEGUI::RadioButton*>(
			wmgr.getWindow((CEGUI::utf8*)"Facial/Radio/Play"));
		// play animation by default
		btn->setSelected(true);
		btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, 
			CEGUI::Event::Subscriber(&FacialApplication::handleRadioChanged, this));

		mPlayAnimation = true;



    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GuiFrameListener(mWindow, mCamera, mGUIRenderer);
        mRoot->addFrameListener(mFrameListener);
    }

    void setupEventHandlers(void)
    {
		/*
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/QuitButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&FacialApplication::handleQuit, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/NewButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&FacialApplication::handleNew, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/LoadButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&FacialApplication::handleLoad, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page2/ObjectTypeList")
			->subscribeEvent(
				CEGUI::Combobox::EventListSelectionAccepted, 
				CEGUI::Event::Subscriber(&FacialApplication::handleObjectSelection, this));
		*/

    }


	void setupLoadedLayoutHandlers(void)
	{
		/*
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		mRed = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Red"));
		mGreen = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Green"));
		mBlue = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Blue"));
		mPreview = static_cast<CEGUI::StaticImage*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/ColourSample"));
		mList = static_cast<CEGUI::Listbox*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Listbox"));
		mEditBox = 
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Editbox");
	
		mRed->subscribeEvent(
				CEGUI::Scrollbar::EventScrollPositionChanged, 
				CEGUI::Event::Subscriber(&FacialApplication::handleColourChanged, this));
		mGreen->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&FacialApplication::handleColourChanged, this));
		mBlue->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&FacialApplication::handleColourChanged, this));

		wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Add")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&FacialApplication::handleAdd, this));

		CEGUI::Window* root = wmgr.getWindow("Demo8");
		setupEnterExitEvents(root);
		*/


	}

    bool handleQuit(const CEGUI::EventArgs& e)
    {
        static_cast<GuiFrameListener*>(mFrameListener)->requestShutdown();
        return true;
    }


};

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
        [pool release];
        return retVal;
#else
    // Create application object
    FacialApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
}

- (void)go;

@end

@implementation AppDelegate

- (void)go {
    // Create application object
    FacialApplication app;
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
    }
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    // Create a window
    UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    // Create an image view
    UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]];
    [window addSubview:imageView];
    
    // Create an indeterminate status indicator
    UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
    [indicator setFrame:CGRectMake(150, 280, 20, 20)];
    [indicator startAnimating];
    [window addSubview:indicator];
    
    // Display our window
    [window makeKeyAndVisible];
    
    // Clean up
    [imageView release];
    [indicator release];

    [NSThread detachNewThreadSelector:@selector(go) toTarget:self withObject:nil];
}

- (void)applicationWillTerminate:(UIApplication *)application {
    Root::getSingleton().queueEndRendering();
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    // Pause FrameListeners and rendering
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    // Resume FrameListeners and rendering
//}

- (void)applicationWillTerminate:(UIApplication *)application {
    Root::getSingleton().queueEndRendering();
}

- (void)dealloc {
    [super dealloc];
}

@end
#   endif

#endif