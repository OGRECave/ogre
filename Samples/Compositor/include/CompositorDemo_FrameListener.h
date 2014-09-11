/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _CompositorDemo_FrameListener_H_
#define _CompositorDemo_FrameListener_H_

#include <OgreFrameListener.h>
#include <OIS/OIS.h>

#include "ItemSelectorViewManager.h"

//---------------------------------------------------------------------------
    
//---------------------------------------------------------------------------
    class CompositorDemo;

    class CompositorDemo_FrameListener : Ogre::FrameListener, OIS::KeyListener,
        OIS::MouseListener, ItemSelectorInterface
    {
    #define MINSPEED .150f
    #define MOVESPEED 30
    #define MAXSPEED 1.800f


    protected:
		Ogre::String mDebugText;

        CompositorDemo* mMain;
        Ogre::Vector3 mTranslateVector;
        bool mStatsOn;
        unsigned int mNumScreenShots;
        bool mWriteToFile;
        float mSkipCount;
        float mUpdateFreq;
        int mSceneDetailIndex;
        Ogre::TextureFilterOptions mFiltering;
        int mAniso;
        bool mQuit;

        float mMoveScale;
        float mRotScale;
        float mSpeed;
        float mAvgFrameTime;
        Ogre::Real mMoveSpeed;
        Ogre::Real mRotateSpeed;
        CEGUI::Point mLastMousePosition;
        bool mLastMousePositionSet;
        // just to stop toggles flipping too fast
        Ogre::Real mTimeUntilNextToggle ;
        float mRotX, mRotY;
        bool mProcessMovement;
        bool mUpdateMovement;
        bool mLMBDown;
        bool mRMBDown;
        bool mMoveFwd;
        bool mMoveBck;
        bool mMoveLeft;
        bool mMoveRight;
		Ogre::SceneNode* mSpinny;

        ItemSelectorViewManager* mCompositorSelectorViewManager;

		OIS::Mouse    *mMouse;
		OIS::Keyboard *mKeyboard;
		OIS::InputManager* mInputManager;

        CEGUI::Renderer* mGuiRenderer;
        CEGUI::Window* mGuiAvg;
        CEGUI::Window* mGuiCurr;
        CEGUI::Window* mGuiBest;
        CEGUI::Window* mGuiWorst;
        CEGUI::Window* mGuiTris;
        CEGUI::Window* mGuiDbg;
        CEGUI::Window* mRoot;
		CEGUI::Listbox* mDebugRTTListbox;
		CEGUI::Window* mDebugRTTStaticImage;
		typedef Ogre::vector<CEGUI::Imageset*>::type ImageSetList;
		ImageSetList mDebugRTTImageSets;

        CEGUI::MouseButton convertOISButtonToCegui(int ois_button_id);
        void CheckMovementKeys( CEGUI::Key::Scan keycode, bool state );
        void updateStats(void);
        void registerCompositors(void);
		void initDebugRTTWindow(void);
		void updateDebugRTTWindow(void);

    public:
        CompositorDemo_FrameListener(CompositorDemo* main);
        virtual ~CompositorDemo_FrameListener();
		void setSpinningNode(Ogre::SceneNode* node) { mSpinny = node; }

    private:
        void connectEventHandlers(void);

		virtual bool mouseMoved(const OIS::MouseEvent &e);
		virtual bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
        virtual bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

        virtual bool keyPressed(const OIS::KeyEvent &e);
        virtual bool keyReleased(const OIS::KeyEvent &e);

        // Event handlers
        bool frameRenderingQueued(const Ogre::FrameEvent& evt);
        bool handleMouseMove(const CEGUI::EventArgs& e);
        bool handleMouseButtonUp(const CEGUI::EventArgs& e);
        bool handleMouseButtonDown(const CEGUI::EventArgs& e);
        bool handleMouseWheelEvent(const CEGUI::EventArgs& e);
        bool handleKeyDownEvent(const CEGUI::EventArgs& e);
        bool handleKeyUpEvent(const CEGUI::EventArgs& e);
		bool handleRttSelection(const CEGUI::EventArgs& e);
        void itemStateChanged(const size_t index, const bool state);
    };

#endif
