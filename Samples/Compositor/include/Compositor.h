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

#ifndef _CompositorDemo_H_
#define _CompositorDemo_H_

// Static plugins declaration section
// Note that every entry in here adds an extra header / library dependency
#ifdef OGRE_STATIC_LIB
#  define OGRE_STATIC_GL
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define OGRE_STATIC_Direct3D9
// dx10 will only work on vista, so be careful about statically linking
#    if OGRE_USE_D3D10
#      define OGRE_STATIC_Direct3D10
#    endif
#  endif
#  define OGRE_STATIC_BSPSceneManager
#  define OGRE_STATIC_ParticleFX
#  define OGRE_STATIC_CgProgramManager
#  ifdef OGRE_USE_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#  else
#    define OGRE_STATIC_OctreeSceneManager
#  endif

#  include "OgreStaticPluginLoader.h"
#endif

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
#ifdef OGRE_STATIC_LIB
		Ogre::StaticPluginLoader	  mStaticPluginLoader;
#endif
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
