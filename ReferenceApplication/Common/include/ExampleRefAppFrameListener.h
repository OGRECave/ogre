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

/*
-----------------------------------------------------------------------------
Filename:    ExampleFrameListener.h
Description: Defines an example frame listener which responds to frame events.
             This frame listener just moves a specified camera around based on
             keyboard and mouse movements.
             Mouse:    Freelook
             W or Up:  Forward
             S or Down:Backward
             A:           Step left
             D:        Step right
             PgUp:     Move upwards
             PgDown:   Move downwards
             O/P:       Yaw the root scene node (and it's children)
             I/K:       Pitch the root scene node (and it's children)
             F:           Toggle frame rate stats on/off
-----------------------------------------------------------------------------
*/

#ifndef __ExampleRefAppFrameListener_H__
#define __ExampleRefAppFrameListener_H__

#include "OgreReferenceAppLayer.h"
#include "OgreException.h"
#include <OIS/OIS.h>

using namespace Ogre;
using namespace OgreRefApp;

class ExampleRefAppFrameListener: public FrameListener
{
private:
    void updateStats(void)
    {
        static String currFps = "Current FPS: ";
        static String avgFps = "Average FPS: ";
        static String bestFps = "Best FPS: ";
        static String worstFps = "Worst FPS: ";
        static String tris = "Triangle Count: ";

        // update stats when necessary
        OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
        OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
        OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
        OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");
        
        guiAvg->setCaption(avgFps + StringConverter::toString(mWindow->getAverageFPS()));
        guiCurr->setCaption(currFps + StringConverter::toString(mWindow->getLastFPS()));
        guiBest->setCaption(bestFps + StringConverter::toString(mWindow->getBestFPS())
            +" "+StringConverter::toString(mWindow->getBestFrameTime())+" ms");
        guiWorst->setCaption(worstFps + StringConverter::toString(mWindow->getWorstFPS())
            +" "+StringConverter::toString(mWindow->getWorstFrameTime())+" ms");
            
        OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
        guiTris->setCaption(tris + StringConverter::toString(mWindow->getTriangleCount()));

        OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
        guiDbg->setCaption(mDebugText);
    }
    
public:
    // Constructor takes a RenderWindow because it uses that to determine input context
    ExampleRefAppFrameListener(RenderWindow* win, CollideCamera* cam, bool bufferedKeys = false, bool bufferedMouse = false)
    {
		OIS::ParamList pl;	
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		win->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

		mInputManager = OIS::InputManager::createInputSystem( pl );

		//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, bufferedKeys ));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, bufferedMouse ));

		unsigned int width, height, depth;
		int left, top;
		win->getMetrics(width, height, depth, left, top);

		//Set Mouse Region.. if window resizes, we should alter this to reflect as well
		const OIS::MouseState &ms = mMouse->getMouseState();
		ms.width = width;
		ms.height = height;

		mCamera = cam;
        mWindow = win;
        mStatsOn = true;
		mNumScreenShots = 0;
		mTimeUntilNextToggle = 0;

        showDebugOverlay(true);

		mDebugText = "Press SPACE to throw the ball";
    }
    virtual ~ExampleRefAppFrameListener()
    {
		if(mInputManager)
		{
			mInputManager->destroyInputObject(mMouse);
			mInputManager->destroyInputObject(mKeyboard);
			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
    }

    bool processUnbufferedKeyInput(const FrameEvent& evt)
    {
		
		if(mKeyboard->isKeyDown(OIS::KC_A))
			mTranslateVector.x = -mMoveScale;	// Move camera left

		if(mKeyboard->isKeyDown(OIS::KC_D))
			mTranslateVector.x = mMoveScale;	// Move camera RIGHT

		if(mKeyboard->isKeyDown(OIS::KC_UP) || mKeyboard->isKeyDown(OIS::KC_W) )
			mTranslateVector.z = -mMoveScale;	// Move camera forward

		if(mKeyboard->isKeyDown(OIS::KC_DOWN) || mKeyboard->isKeyDown(OIS::KC_S) )
			mTranslateVector.z = mMoveScale;	// Move camera backward

		if(mKeyboard->isKeyDown(OIS::KC_PGUP))
			mTranslateVector.y = mMoveScale;	// Move camera up

		if(mKeyboard->isKeyDown(OIS::KC_PGDOWN))
			mTranslateVector.y = -mMoveScale;	// Move camera down

		if(mKeyboard->isKeyDown(OIS::KC_RIGHT))
			mCamera->yaw(-mRotScale);
		
		if(mKeyboard->isKeyDown(OIS::KC_LEFT))
			mCamera->yaw(mRotScale);

		if( mKeyboard->isKeyDown(OIS::KC_ESCAPE) || mKeyboard->isKeyDown(OIS::KC_Q) )
			return false;

       	if( mKeyboard->isKeyDown(OIS::KC_F) && mTimeUntilNextToggle <= 0 ) 
		{
			mStatsOn = !mStatsOn;
			showDebugOverlay(mStatsOn);
			mTimeUntilNextToggle = 1;
		}

		if(mKeyboard->isKeyDown(OIS::KC_SYSRQ) && mTimeUntilNextToggle <= 0)
		{
			std::ostringstream ss;
			ss << "screenshot_" << ++mNumScreenShots << ".png";
			mWindow->writeContentsToFile(ss.str());
			mTimeUntilNextToggle = 0.5;
			mDebugText = "Saved: " + ss.str();
		}


		// Return true to continue rendering
		return true;
    }

    bool processUnbufferedMouseInput(const FrameEvent& evt)
    {
		
		// Rotation factors, may not be used if the second mouse button is pressed
		// 2nd mouse button - slide, otherwise rotate
		const OIS::MouseState &ms = mMouse->getMouseState();
		if( ms.buttonDown( OIS::MB_Right ) )
		{
			mTranslateVector.x += ms.X.rel * 0.13;
			mTranslateVector.y -= ms.Y.rel * 0.13;
		}
		else
		{
			mRotX = Degree(-ms.X.rel * 0.13);
			mRotY = Degree(-ms.Y.rel * 0.13);
		}

		return true;
	}

	void moveCamera()
	{

        // Make all the changes to the camera
        // Note that YAW direction is around a fixed axis (freelook style) rather than a natural YAW (e.g. airplane)
        mCamera->yaw(mRotX);
        mCamera->pitch(mRotY);
        mCamera->translate(mTranslateVector);
	}

    void showDebugOverlay(bool show)
    {   
        Overlay* o = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
        if (!o)
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find overlay Core/DebugOverlay",
                "showDebugOverlay" );
        if (show)
            o->show();
        else
            o->hide();
    }

    // Override frameEnded event 
    bool frameEnded(const FrameEvent& evt)
    {
		mMouse->capture();
		mKeyboard->capture();

		if( !mMouse->buffered() || !mKeyboard->buffered() )
		{
			// one of the input modes is immediate, so setup what is needed for immediate mouse/key movement
			if (mTimeUntilNextToggle >= 0) 
				mTimeUntilNextToggle -= evt.timeSinceLastFrame;

			// If this is the first frame, pick a speed
			if (evt.timeSinceLastFrame == 0)
			{
				mMoveScale = 0.5;
				mRotScale = 0.1;
			}
			// Otherwise scale movement units by time passed since last frame
			else
			{
				// Move about 50 units per second,
				mMoveScale = 50.0 * evt.timeSinceLastFrame;
				// Take about 10 seconds for full rotation
				mRotScale = Degree(36 * evt.timeSinceLastFrame);
			}
			mRotX = 0;
            mRotY = 0;
	        mTranslateVector = Vector3::ZERO;
		}

		//Check to see which device is not buffered, and handle it
		if( !mKeyboard->buffered() )
			if( processUnbufferedKeyInput(evt) == false )
				return false;
		if( !mMouse->buffered() )
			if( processUnbufferedMouseInput(evt) == false )
				return false;
		
		if( !mMouse->buffered() || !mKeyboard->buffered() )
			moveCamera();

        // Perform simulation step
        World::getSingleton().simulationStep(evt.timeSinceLastFrame);

        updateStats();
		return true;
    }

protected:
	OIS::Mouse *mMouse;
	OIS::Keyboard *mKeyboard;
    OIS::InputManager *mInputManager;
    
    CollideCamera* mCamera;
    Vector3 mTranslateVector;
    RenderWindow* mWindow;
    bool mStatsOn;
	unsigned int mNumScreenShots;
    float mMoveScale;
    Radian mRotScale;
    // just to stop toggles flipping too fast
    Real mTimeUntilNextToggle ;
    Radian mRotX, mRotY;

	std::string mDebugText;
};
#endif
