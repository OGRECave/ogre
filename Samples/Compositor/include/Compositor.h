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

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

#define COMPOSITORS_PER_PAGE 10

//---------------------------------------------------------------------------
    //class CompositorDemo_FrameListener;
//---------------------------------------------------------------------------
	class _OgreSampleClassExport Sample_Compositor : public SdkSample
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


        //CompositorDemo_FrameListener* mFrameListener;
        
        SceneNode * mSpinny;
		StringVector mCompositorNames;
		size_t mActiveCompositorPage;
		size_t mNumCompositorPages;
		virtual void setupContent(void);
        
		void createCamera(void);
		void createControls(void);
        void createScene(void);
        //void createFrameListener(void);
        void createEffects(void);
		void createTextures(void);

        void connectEventHandlers(void);
        //void configureShaderControls(void);

		void cleanupContent(void);

		bool frameRenderingQueued(const FrameEvent& evt);
	
		void registerCompositors();
		void changePage(size_t pageNum);

		virtual void checkBoxToggled(OgreBites::CheckBox * box);
		virtual void buttonHit(OgreBites::Button* button);
    public:
        Sample_Compositor()
        {
			mInfo["Title"] = "Compositor";
			mInfo["Description"] = "A demo of Ogre's post-processing framework.";
			mInfo["Thumbnail"] = "thumb_comp.png";
			mInfo["Category"] = "Unsorted";
        }

        ~Sample_Compositor();
    };


#endif	// end _CompositorDemo_H_
