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
        Gui.cpp
    \brief
        An example of CEGUI's features
*/

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/CEGUIPropertyHelper.h>
#include <CEGUI/elements/CEGUICombobox.h>
#include <CEGUI/elements/CEGUIListbox.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
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

class GuiFrameListener : public ExampleFrameListener, public OIS::KeyListener, public OIS::MouseListener
{
private:
    CEGUI::Renderer* mGUIRenderer;
    bool mShutdownRequested;

public:
    // NB using buffered input, this is the only change
    GuiFrameListener(RenderWindow* win, Camera* cam, CEGUI::Renderer* renderer)
        : ExampleFrameListener(win, cam, true, true, true), 
          mGUIRenderer(renderer),
          mShutdownRequested(false)
    {
		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);
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

class GuiApplication : public ExampleApplication
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
	typedef map<CEGUI::String, CEGUI::String>::type DescriptionMap;
	DescriptionMap mDescriptionMap;

public:
    GuiApplication()
      : mGUIRenderer(0),
        mGUISystem(0),
        mEditorGuiSheet(0)
    {
		mDescriptionMap[(CEGUI::utf8*)"Demo8"] = 
			(CEGUI::utf8*)"The main containing panel";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1"] = 
			(CEGUI::utf8*)"A test window";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Listbox"] = 
			(CEGUI::utf8*)"A list box";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/Red"] = 
			(CEGUI::utf8*)"A colour slider";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/Green"] = 
			(CEGUI::utf8*)"A colour slider";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/Blue"] = 
			(CEGUI::utf8*)"A colour slider";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/ColourSample"] = 
			(CEGUI::utf8*)"The colour that will be used for the selection when added to the list";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/Editbox"] = 
			(CEGUI::utf8*)"An edit box; this text will be added to the list";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/Add"] = 
			(CEGUI::utf8*)"Adds the text to the list";
		mDescriptionMap[(CEGUI::utf8*)"Demo8/Window1/Controls/ins1"] = 
			(CEGUI::utf8*)"Some static text";


	}

    ~GuiApplication()
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
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        // setup GUI system
        mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, 
            Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);

        mGUISystem = new CEGUI::System(mGUIRenderer);

        CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(ogreHead);


        // Setup Render To Texture for preview window
		TexturePtr rttTex = TextureManager::getSingleton().createManual("RttTex", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
			512, 512, 1, 0, PF_R8G8B8, TU_RENDERTARGET);
        {
            Camera* rttCam = mSceneMgr->createCamera("RttCam");
            SceneNode* camNode = 
                mSceneMgr->getRootSceneNode()->createChildSceneNode("rttCamNode");
            camNode->attachObject(rttCam);
            rttCam->setPosition(0,0,200);
            //rttCam->setVisible(true);

            Viewport *v = rttTex->getBuffer()->getRenderTarget()->addViewport( rttCam );
            v->setOverlaysEnabled(false);
            v->setClearEveryFrame( true );
            v->setBackgroundColour( ColourValue::Black );
        }

        // Retrieve CEGUI texture for the RTT
        CEGUI::Texture* rttTexture = mGUIRenderer->createTexture((CEGUI::utf8*)"RttTex");

        CEGUI::Imageset* rttImageSet = 
            CEGUI::ImagesetManager::getSingleton().createImageset(
                    (CEGUI::utf8*)"RttImageset", rttTexture);

        rttImageSet->defineImage((CEGUI::utf8*)"RttImage", 
                CEGUI::Point(0.0f, 0.0f),
                CEGUI::Size(rttTexture->getWidth(), rttTexture->getHeight()),
                CEGUI::Point(0.0f,0.0f));

        // load scheme and set up defaults
        CEGUI::SchemeManager::getSingleton().loadScheme(
                (CEGUI::utf8*)"TaharezLookSkin.scheme");
        mGUISystem->setDefaultMouseCursor(
                (CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mGUISystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

        CEGUI::Window* sheet = 
            CEGUI::WindowManager::getSingleton().loadWindowLayout(
                (CEGUI::utf8*)"ogregui.layout"); 
        mGUISystem->setGUISheet(sheet);

        CEGUI::Combobox* objectComboBox = (CEGUI::Combobox*)CEGUI::WindowManager::getSingleton().getWindow("OgreGuiDemo/TabCtrl/Page2/ObjectTypeList");

        CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"FrameWindow", 0);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Horizontal Scrollbar", 1);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Vertical Scrollbar", 2);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"StaticText", 3);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"StaticImage", 4);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Render to Texture", 5);
        objectComboBox->addItem(item);

        setupEventHandlers();
    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GuiFrameListener(mWindow, mCamera, mGUIRenderer);
        mRoot->addFrameListener(mFrameListener);
    }

    void setupEventHandlers(void)
    {
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/QuitButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleQuit, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/NewButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleNew, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/LoadButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleLoad, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page2/ObjectTypeList")
			->subscribeEvent(
				CEGUI::Combobox::EventListSelectionAccepted, 
				CEGUI::Event::Subscriber(&GuiApplication::handleObjectSelection, this));

    }

	void setupEnterExitEvents(CEGUI::Window* win)
	{
		win->subscribeEvent(
			CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&GuiApplication::handleMouseEnters, this));
		win->subscribeEvent(
			CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&GuiApplication::handleMouseLeaves, this));
		for (unsigned int i = 0; i < win->getChildCount(); ++i)
		{
			CEGUI::Window* child = win->getChildAtIdx(i);
			setupEnterExitEvents(child);
		}

	}

	void setupLoadedLayoutHandlers(void)
	{
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		mRed = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Red"));
		mGreen = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Green"));
		mBlue = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Blue"));
		mPreview = wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/ColourSample");
		mList = static_cast<CEGUI::Listbox*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Listbox"));
		mEditBox = 
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Editbox");
		mTip = 
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window2/Tips");
	
		mRed->subscribeEvent(
				CEGUI::Scrollbar::EventScrollPositionChanged, 
				CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));
		mGreen->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));
		mBlue->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));

		wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Add")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&GuiApplication::handleAdd, this));

		CEGUI::Window* root = wmgr.getWindow("Demo8");
		setupEnterExitEvents(root);



	}

    CEGUI::Window* createRttGuiObject(void)
    {
        static unsigned int rttCounter = 0;
		String guiObjectName = "NewRttImage" + StringConverter::toString(rttCounter);

        CEGUI::Imageset* rttImageSet = 
            CEGUI::ImagesetManager::getSingleton().getImageset(
                (CEGUI::utf8*)"RttImageset");
        CEGUI::Window* si = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/StaticImage", (CEGUI::utf8*)guiObjectName.c_str());
        si->setSize(CEGUI::UVector2( CEGUI::UDim(0.5f, 0), CEGUI::UDim(0.4f, 0)));
        si->setProperty("Image", CEGUI::PropertyHelper::imageToString(
            &rttImageSet->getImage((CEGUI::utf8*)"RttImage")));

        rttCounter++;

        return si;
    }

    CEGUI::Window* createStaticImageObject(void)
    {
        static unsigned int siCounter = 0;
        String guiObjectName = "NewStaticImage" + StringConverter::toString(siCounter);

        CEGUI::Imageset* imageSet = 
            CEGUI::ImagesetManager::getSingleton().getImageset(
                (CEGUI::utf8*)"TaharezLook");

        CEGUI::Window* si = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/StaticImage", (CEGUI::utf8*)guiObjectName.c_str());
        si->setSize(CEGUI::UVector2( CEGUI::UDim(0.2f, 0), CEGUI::UDim(0.2f, 0)));
        si->setProperty("Image", CEGUI::PropertyHelper::imageToString(
            &imageSet->getImage((CEGUI::utf8*)"ClientBrush")));

        siCounter++;

        return si;
    }

    bool handleQuit(const CEGUI::EventArgs& e)
    {
        static_cast<GuiFrameListener*>(mFrameListener)->requestShutdown();
        return true;
    }

    bool handleNew(const CEGUI::EventArgs& e)
    {
        if(mEditorGuiSheet)
            CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);

        mEditorGuiSheet = CEGUI::WindowManager::getSingleton().createWindow("DefaultGUISheet", "NewLayout");

        CEGUI::Window* editorWindow = CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"OgreGuiDemo2/MainWindow");
        editorWindow->addChildWindow(mEditorGuiSheet);

        return true;
    }

    bool handleLoad(const CEGUI::EventArgs& e)
    {
        if(mEditorGuiSheet)
            CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);

        mEditorGuiSheet = 
            CEGUI::WindowManager::getSingleton().loadWindowLayout(
                (CEGUI::utf8*)"cegui8.layout"); 
		setupLoadedLayoutHandlers();

        CEGUI::Window* editorWindow = CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"OgreGuiDemo2/MainWindow");
        editorWindow->addChildWindow(mEditorGuiSheet);

        return true;
    }


    bool handleObjectSelection(const CEGUI::EventArgs& e)
    {
        static unsigned int windowNumber = 0;
        static unsigned int vertScrollNumber = 0;
        static unsigned int horizScrollNumber = 0;
        static unsigned int textScrollNumber = 0;
        String guiObjectName;
        CEGUI::Window* window = 0;

        // Set a random position to place this object.
        Real posX = Math::RangeRandom(0.0, 0.7); 
        Real posY = Math::RangeRandom(0.1, 0.7); 

        const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(e);
        CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();

        CEGUI::Window* editorWindow = CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"OgreGuiDemo2/MainWindow");

        switch(item->getID())
        {
        case 0:
            guiObjectName = "NewWindow" + StringConverter::toString(windowNumber);
            window = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/FrameWindow", (CEGUI::utf8*)guiObjectName.c_str());
            window->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f,0), CEGUI::UDim(0.3f,0)));
            window->setText((CEGUI::utf8*)"New Window");
            windowNumber++;
            break;
        case 1:
            guiObjectName = "NewHorizScroll" + StringConverter::toString(horizScrollNumber);
            window = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/HorizontalScrollbar", (CEGUI::utf8*)guiObjectName.c_str());
            window->setSize(CEGUI::UVector2(CEGUI::UDim(0.75f,0), CEGUI::UDim(0.03f,0)));
            horizScrollNumber++;
            break;
        case 2:
            guiObjectName = "NewVertScroll" + StringConverter::toString(vertScrollNumber);
            window = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/VerticalScrollbar", (CEGUI::utf8*)guiObjectName.c_str());
            window->setSize(CEGUI::UVector2(CEGUI::UDim(0.03f,0), CEGUI::UDim(0.75f,0)));
            vertScrollNumber++;
            break;
        case 3:
            guiObjectName = "NewStaticText" + StringConverter::toString(textScrollNumber);
            window = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"TaharezLook/StaticText", (CEGUI::utf8*)guiObjectName.c_str());
            window->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f,0), CEGUI::UDim(0.1f,0)));
            window->setText((CEGUI::utf8*)"Example static text");
            textScrollNumber++;
            break;
        case 4:
            window = createStaticImageObject();
            break;
        case 5:
            window = createRttGuiObject();
            break;
        };

        editorWindow->addChildWindow(window);
        window->setPosition(CEGUI::UVector2(CEGUI::UDim(posX, 0), CEGUI::UDim(posY, 0)));

        return true;
    }

	bool handleColourChanged(const CEGUI::EventArgs& e)
	{
        mPreview->setProperty("ImageColours",
            CEGUI::PropertyHelper::colourToString(CEGUI::colour(
                mRed->getScrollPosition() / 255.0f,
                mGreen->getScrollPosition() / 255.0f,
                mBlue->getScrollPosition() / 255.0f)));

		return true;

	}

	bool handleAdd(const CEGUI::EventArgs& e)
	{
		CEGUI::ListboxTextItem *listboxitem = 
			new CEGUI::ListboxTextItem (mEditBox->getText());
		listboxitem->setSelectionBrushImage("TaharezLook", "ListboxSelectionBrush");
		listboxitem->setSelected(mList->getItemCount() == 0);
		listboxitem->setSelectionColours(
            CEGUI::PropertyHelper::stringToColourRect(mPreview->getProperty("ImageColours")));
		mList->addItem(listboxitem);
		return true;
	}

	bool handleMouseEnters(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs& we = ((CEGUI::WindowEventArgs&)e);
		DescriptionMap::iterator i = 
			mDescriptionMap.find(we.window->getName());
		if (i != mDescriptionMap.end())
		{
			mTip->setText(i->second);
		}
		else
		{
			mTip->setText((CEGUI::utf8*)"");
		}
		
		return true;
	}
	bool handleMouseLeaves(const CEGUI::EventArgs& e)
	{
		mTip->setText((CEGUI::utf8*)"");
		return true;
	}

};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{

    // Create application object
    GuiApplication app;

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
}

