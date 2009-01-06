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

#ifndef _CompositorDemo_H_
#define _CompositorDemo_H_

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ItemSelectorViewManager.h"

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

#include "OgreCEGUIRenderer.h"

//---------------------------------------------------------------------------
    class CompositorDemo_FrameListener;
//---------------------------------------------------------------------------
    class CompositorDemo
    {
    protected:
        Ogre::Root*			  mRoot;
        Ogre::Camera*		  mCamera;
        Ogre::SceneManager*	  mSceneMgr;
        // the scene node of the entity
        Ogre::SceneNode*	  mMainNode;

        CompositorDemo_FrameListener* mFrameListener;
        Ogre::RenderWindow*	  mWindow;
        CEGUI::OgreCEGUIRenderer*    mGUIRenderer;
        CEGUI::System*        mGUISystem;

        size_t				  mCurrentMaterial;
		Ogre::SceneNode * mSpinny;

//        typedef vector< ShaderControlGUIWidget >::type ShaderControlContainer;
//        typedef ShaderControlContainer::iterator ShaderControlIterator;

//        ShaderControlContainer    mShaderControlContainer;
//        MaterialControlsContainer mMaterialControlsContainer;

        // These internal methods package up the stages in the startup process
        /** Sets up the application - returns false if the user chooses to abandon configuration. */
        bool setup(void);

        /** Configures the application - returns false if the user chooses to abandon configuration. */
        bool configure(void);
        void chooseSceneManager(void);
        void createCamera(void);
        void createViewports(void);

        /// Method which will define the source of resources (other than current folder)
        void setupResources(void);
        void loadResources(void);
        void createScene(void);
        void createFrameListener(void);
        void createEffects(void);
		void createTextures(void);

        void connectEventHandlers(void);
        //void configureShaderControls(void);

        void doErrorBox(const char* text);

        bool handleQuit(const CEGUI::EventArgs& e);
        //bool handleShaderControl(const CEGUI::EventArgs& e);
        //bool handleShaderComboChanged(const CEGUI::EventArgs& e);
        bool handleErrorBox(const CEGUI::EventArgs& e);
        //void setShaderControlVal(const float val, const size_t index);

    public:
        CompositorDemo() : mRoot(0), mFrameListener(0), mGUIRenderer(0), mGUISystem(0)
        {
        }

        ~CompositorDemo();

        void go(void);
        Ogre::Camera* getCamera(void) const { return mCamera; }
        Ogre::SceneManager* getSceneManager(void) const { return mSceneMgr; }
        Ogre::RenderWindow* getRenderWindow(void) const { return mWindow; }
        Ogre::SceneNode* getMainNode(void) const { return mMainNode; }
		CEGUI::OgreCEGUIRenderer* getGuiRenderer(void) const { return mGUIRenderer; }

    };


#endif	// end _CompositorDemo_H_
