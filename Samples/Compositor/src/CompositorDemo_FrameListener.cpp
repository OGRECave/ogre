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

#include <Ogre.h>
#include <OgreTimer.h>
#include <CEGUI/CEGUIPropertyHelper.h>

#include "CompositorDemo_FrameListener.h"
#include "Compositor.h"


inline Ogre::String operator +(const Ogre::String& l,const CEGUI::String& o)
{
	return l+o.c_str();
}
/*
inline CEGUI::String operator +(const CEGUI::String& l,const Ogre::String& o)
{
	return l+o.c_str();
}
*/
/*************************************************************************
	                    HeatVisionListener Methods
*************************************************************************/
//---------------------------------------------------------------------------
    HeatVisionListener::HeatVisionListener()
    {
		timer = new Ogre::Timer();
        start = end = curr = 0.0f;
    }
//---------------------------------------------------------------------------
    HeatVisionListener::~HeatVisionListener()
    {
       delete timer;
    }
//---------------------------------------------------------------------------
    void HeatVisionListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if(pass_id == 0xDEADBABE)
        {
            timer->reset();
            fpParams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
        }
    }
//---------------------------------------------------------------------------
    void HeatVisionListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if(pass_id == 0xDEADBABE)
        {
            // "random_fractions" parameter
            fpParams->setNamedConstant("random_fractions", Ogre::Vector4(Ogre::Math::RangeRandom(0.0, 1.0), Ogre::Math::RangeRandom(0, 1.0), 0, 0));

            // "depth_modulator" parameter
            float inc = ((float)timer->getMilliseconds())/1000.0f;
            if ( (fabs(curr-end) <= 0.001) ) {
                // take a new value to reach
                end = Ogre::Math::RangeRandom(0.95, 1.0);
                start = curr;
            } else {
                if (curr > end) curr -= inc;
                else curr += inc;
            }
            timer->reset();

            fpParams->setNamedConstant("depth_modulator", Ogre::Vector4(curr, 0, 0, 0));
        }
    }
//---------------------------------------------------------------------------

	/*************************************************************************
	HDRListener Methods
	*************************************************************************/
	//---------------------------------------------------------------------------
	HDRListener::HDRListener()
	{
	}
	//---------------------------------------------------------------------------
	HDRListener::~HDRListener()
	{
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
	{
		// Get some RTT dimensions for later calculations
		Ogre::CompositionTechnique::TextureDefinitionIterator defIter =
			instance->getTechnique()->getTextureDefinitionIterator();
		while (defIter.hasMoreElements())
		{
			Ogre::CompositionTechnique::TextureDefinition* def =
				defIter.getNext();
			if(def->name == "rt_bloom0")
			{
				mBloomSize = (int)def->width; // should be square
				// Calculate gaussian texture offsets & weights
				float deviation = 3.0f;
				float texelSize = 1.0f / (float)mBloomSize;

				// central sample, no offset
				mBloomTexOffsetsHorz[0][0] = 0.0f;
				mBloomTexOffsetsHorz[0][1] = 0.0f;
				mBloomTexOffsetsVert[0][0] = 0.0f;
				mBloomTexOffsetsVert[0][1] = 0.0f;
				mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
					mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
				mBloomTexWeights[0][3] = 1.0f;

				// 'pre' samples
				for(int i = 1; i < 8; ++i)
				{
					mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
						mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution(i, 0, deviation);
					mBloomTexWeights[i][3] = 1.0f;
					mBloomTexOffsetsHorz[i][0] = i * texelSize;
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = i * texelSize;
				}
				// 'post' samples
				for(int i = 8; i < 15; ++i)
				{
					mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
						mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
					mBloomTexWeights[i][3] = 1.0f;

					mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
				}

			}
		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		//case 994: // rt_lum4
		case 993: // rt_lum3
		case 992: // rt_lum2
		case 991: // rt_lum1
		case 990: // rt_lum0
			break;
		case 800: // rt_brightpass
			break;
		case 701: // rt_bloom1
			{
				// horizontal bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
//				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		case 700: // rt_bloom0
			{
				// vertical bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
//				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
	}
	//---------------------------------------------------------------------------


	/*************************************************************************
	GaussianListener Methods
	*************************************************************************/
	//---------------------------------------------------------------------------
	GaussianListener::GaussianListener()
	{
	}
	//---------------------------------------------------------------------------
	GaussianListener::~GaussianListener()
	{
	}
	//---------------------------------------------------------------------------
	void GaussianListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
		// Calculate gaussian texture offsets & weights
		float deviation = 3.0f;
		float texelSize = 1.0f / (float)std::min(mVpWidth, mVpHeight);

		// central sample, no offset
		mBloomTexOffsetsHorz[0][0] = 0.0f;
		mBloomTexOffsetsHorz[0][1] = 0.0f;
		mBloomTexOffsetsVert[0][0] = 0.0f;
		mBloomTexOffsetsVert[0][1] = 0.0f;
		mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
			mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
		mBloomTexWeights[0][3] = 1.0f;

		// 'pre' samples
		for(int i = 1; i < 8; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
				mBloomTexWeights[i][2] = Ogre::Math::gaussianDistribution(i, 0, deviation);
			mBloomTexWeights[i][3] = 1.0f;
			mBloomTexOffsetsHorz[i][0] = i * texelSize;
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = i * texelSize;
		}
		// 'post' samples
		for(int i = 8; i < 15; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
				mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
			mBloomTexWeights[i][3] = 1.0f;

			mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
		}
	}
	//---------------------------------------------------------------------------
	void GaussianListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		case 701: // blur horz
			{
				// horizontal bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
//				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		case 700: // blur vert
			{
				// vertical bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
//				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		}
	}
	//---------------------------------------------------------------------------
	void GaussianListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
	}
	//---------------------------------------------------------------------------

/*************************************************************************
	CompositorDemo_FrameListener methods that handle all input for this Compositor demo.
*************************************************************************/

    CompositorDemo_FrameListener::CompositorDemo_FrameListener(CompositorDemo* main)
        : mMain(main)
        , hvListener(0)
		, hdrListener(0)
		, gaussianListener(0)
        , mTranslateVector(Ogre::Vector3::ZERO)
        , mStatsOn(true)
        , mNumScreenShots(0)
        , mWriteToFile(false)
        , mSkipCount(0)
        , mUpdateFreq(50)
        , mSceneDetailIndex(0)
        , mFiltering(Ogre::TFO_BILINEAR)
        , mAniso(1)
        , mQuit(false)
        , mMoveScale(0.0f)
        , mRotScale(0.0f)
        , mSpeed(MINSPEED)
        , mAvgFrameTime(0.1)
        , mMoveSpeed(100)
        , mRotateSpeed(0)
        , mLastMousePositionSet(false)
        , mTimeUntilNextToggle(0)
        , mRotX(0)
        , mRotY(0)
        , mProcessMovement(false)
        , mUpdateMovement(false)

        , mLMBDown(false)
        , mRMBDown(false)
        , mMoveFwd(false)
        , mMoveBck(false)
        , mMoveLeft(false)
        , mMoveRight(false)
		, mSpinny(0)
        , mCompositorSelectorViewManager(0)
		, mMouse(0)
		, mKeyboard(0)

    {

        Ogre::Root::getSingleton().addFrameListener(this);

        // using buffered input
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		mMain->getRenderWindow()->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

		mInputManager = OIS::InputManager::createInputSystem( pl );

		//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

		unsigned int width, height, depth;
		int left, top;
		mMain->getRenderWindow()->getMetrics(width, height, depth, left, top);

		//Set Mouse Region.. if window resizes, we should alter this to reflect as well
		const OIS::MouseState &ms = mMouse->getMouseState();
		ms.width = width;
		ms.height = height;

		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);

        mGuiRenderer = CEGUI::System::getSingleton().getRenderer();

        mGuiAvg   = CEGUI::WindowManager::getSingleton().getWindow("OPAverageFPS");
        mGuiCurr  = CEGUI::WindowManager::getSingleton().getWindow("OPCurrentFPS");
        mGuiBest  = CEGUI::WindowManager::getSingleton().getWindow("OPBestFPS");
        mGuiWorst = CEGUI::WindowManager::getSingleton().getWindow("OPWorstFPS");
        mGuiTris  = CEGUI::WindowManager::getSingleton().getWindow("OPTriCount");
        mGuiDbg   = CEGUI::WindowManager::getSingleton().getWindow("OPDebugMsg");
        mRoot	  = CEGUI::WindowManager::getSingleton().getWindow("root");

        registerCompositors();
		initDebugRTTWindow();
        connectEventHandlers();
    }
//--------------------------------------------------------------------------
    CompositorDemo_FrameListener::~CompositorDemo_FrameListener()
    {
		if(mInputManager)
		{
			mInputManager->destroyInputObject(mKeyboard);
			mInputManager->destroyInputObject(mMouse);
			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}

        delete hvListener;
		delete hdrListener;
		delete gaussianListener;
        delete mCompositorSelectorViewManager;
    }
//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::connectEventHandlers(void)
    {
        CEGUI::Window* wndw = CEGUI::WindowManager::getSingleton().getWindow("root");

        wndw->subscribeEvent(CEGUI::Window::EventMouseMove, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseMove, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseButtonUp, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseButtonDown, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseWheel, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseWheelEvent, this));
        wndw->subscribeEvent(CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleKeyDownEvent, this ));
        wndw->subscribeEvent(CEGUI::Window::EventKeyUp, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleKeyUpEvent, this ));
    }
//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::frameRenderingQueued(const Ogre::FrameEvent& evt)
    {
	mMouse->capture();
	mKeyboard->capture();

	if( mMain->getRenderWindow()->isClosed() )
		return false;

        if (mQuit)
            return false;
        else
        {
            mSkipCount++;
            if (mSkipCount >= mUpdateFreq)
            {
                mSkipCount = 0;
                updateStats();
            }
            // update movement process
            if(mProcessMovement || mUpdateMovement)
            {
                mTranslateVector.x += mMoveLeft ? mAvgFrameTime * -MOVESPEED : 0;
                mTranslateVector.x += mMoveRight ? mAvgFrameTime * MOVESPEED : 0;
                mTranslateVector.z += mMoveFwd ? mAvgFrameTime * -MOVESPEED : 0;
                mTranslateVector.z += mMoveBck ? mAvgFrameTime * MOVESPEED : 0;

                mMain->getCamera()->yaw(Ogre::Angle(mRotX));
                mMain->getCamera()->pitch(Ogre::Angle(mRotY));
                mMain->getCamera()->moveRelative(mTranslateVector);

                mUpdateMovement = false;
                mRotX = 0;
                mRotY = 0;
                mTranslateVector = Ogre::Vector3::ZERO;
            }

            if(mWriteToFile)
            {
                mMain->getRenderWindow()->writeContentsToFile("frame_" +
                    Ogre::StringConverter::toString(++mNumScreenShots) + ".png");
            }

			if (mSpinny)
				mSpinny->yaw(Ogre::Degree(10 * evt.timeSinceLastFrame));

            return true;
        }
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::mouseMoved (const OIS::MouseEvent &e)
    {
        CEGUI::System::getSingleton().injectMouseMove( e.state.X.rel, e.state.Y.rel );
		CEGUI::System::getSingleton().injectMouseWheelChange(e.state.Z.rel);
		return true;
    }

//--------------------------------------------------------------------------
	bool CompositorDemo_FrameListener::keyPressed (const OIS::KeyEvent &e)
    {
        // give 'quitting' priority
		if (e.key == OIS::KC_ESCAPE)
        {
            mQuit = true;
            return false;
        }

		if (e.key == OIS::KC_V)
			mMain->getRenderWindow()->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0,1,0));

        if (e.key == OIS::KC_SYSRQ )
        {
			Ogre::StringStream ss;
            ss << "screenshot_" << ++mNumScreenShots << ".png";
            mMain->getRenderWindow()->writeContentsToFile(ss.str());
            mDebugText = "Saved: " + ss.str();
			//mTimeUntilNextToggle = 0.5;
        }

        // do event injection
        CEGUI::System& cegui = CEGUI::System::getSingleton();
        cegui.injectKeyDown(e.key);
		cegui.injectChar(e.text);
		return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::keyReleased (const OIS::KeyEvent &e)
    {
        CEGUI::System::getSingleton().injectKeyUp(e.key);
		return true;
    }

//--------------------------------------------------------------------------
	bool CompositorDemo_FrameListener::mousePressed (const OIS::MouseEvent &e, OIS::MouseButtonID id)
    {
        CEGUI::System::getSingleton().injectMouseButtonDown(convertOISButtonToCegui(id));
		return true;
	}

//--------------------------------------------------------------------------
	bool CompositorDemo_FrameListener::mouseReleased (const OIS::MouseEvent &e, OIS::MouseButtonID id)
    {
        CEGUI::System::getSingleton().injectMouseButtonUp(convertOISButtonToCegui(id));
		return true;
    }

//--------------------------------------------------------------------------
    CEGUI::MouseButton CompositorDemo_FrameListener::convertOISButtonToCegui(int ois_button_id)
    {
        switch (ois_button_id)
		{
		case 0: return CEGUI::LeftButton;
		case 1: return CEGUI::RightButton;
		case 2:	return CEGUI::MiddleButton;
		case 3: return CEGUI::X1Button;
		default: return CEGUI::LeftButton;
		}
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::updateStats(void)
    {
        static CEGUI::String currFps = "Current FPS: ";
        static CEGUI::String avgFps = "Average FPS: ";
        static CEGUI::String bestFps = "Best FPS: ";
        static CEGUI::String worstFps = "Worst FPS: ";
        static CEGUI::String tris = "Triangle Count: ";


        const Ogre::RenderTarget::FrameStats& stats = mMain->getRenderWindow()->getStatistics();

        mGuiAvg->setText(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
        mGuiCurr->setText(currFps + Ogre::StringConverter::toString(stats.lastFPS));
        mGuiBest->setText(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
            + " " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
        mGuiWorst->setText(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
            + " " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

        mGuiTris->setText(tris + Ogre::StringConverter::toString(stats.triangleCount));
        mGuiDbg->setText(mDebugText.c_str());
        mAvgFrameTime = 1.0f/(stats.avgFPS + 1.0f);
        if (mAvgFrameTime > 0.1f) mAvgFrameTime = 0.1f;

    }


//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseMove(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        if( mLMBDown && !mRMBDown)
        {
            // rotate camera
            mRotX += -((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * 10.0;
            mRotY += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * 10.0;
            MouseCursor::getSingleton().setPosition( mLastMousePosition );
            mUpdateMovement = true;
        }
        else
        {
            if( mRMBDown && !mLMBDown)
            {
                // translate camera
                mTranslateVector.x += ((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * MOVESPEED;
                mTranslateVector.y += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * MOVESPEED;
                //mTranslateVector.z = 0;
                MouseCursor::getSingleton().setPosition( mLastMousePosition );
                mUpdateMovement = true;
            }
            else
            {
                if( mRMBDown && mLMBDown)
                {
                    mTranslateVector.z += (((const MouseEventArgs&)e).moveDelta.d_x + ((const MouseEventArgs&)e).moveDelta.d_y) * mAvgFrameTime * MOVESPEED;
                    MouseCursor::getSingleton().setPosition( mLastMousePosition );
                    mUpdateMovement = true;
                }

            }
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseButtonUp(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        //Window* wndw = ((const WindowEventArgs&)e).window;
        if( ((const MouseEventArgs&)e).button == LeftButton )
        {
            mLMBDown = false;
        }

        if( ((const MouseEventArgs&)e).button == RightButton )
        {
            mRMBDown = false;
        }
        if( !mLMBDown && !mRMBDown )
        {
            MouseCursor::getSingleton().show();
            if(mLastMousePositionSet)
            {
                MouseCursor::getSingleton().setPosition( mLastMousePosition );
                mLastMousePositionSet = false;
            }
            mRoot->releaseInput();
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseButtonDown(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        //Window* wndw = ((const WindowEventArgs&)e).window;
        if( ((const MouseEventArgs&)e).button == LeftButton )
        {
            mLMBDown = true;
        }

        if( ((const MouseEventArgs&)e).button == RightButton )
        {
            mRMBDown = true;
        }

        if( mLMBDown || mRMBDown )
        {
            MouseCursor::getSingleton().hide();
            if (!mLastMousePositionSet)
            {
                mLastMousePosition = MouseCursor::getSingleton().getPosition();
                mLastMousePositionSet = true;
            }
            mRoot->captureInput();
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseWheelEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;
        mTranslateVector.z += ((const MouseEventArgs&)e).wheelChange * -5.0;
        mUpdateMovement = true;

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleKeyDownEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        CheckMovementKeys( ((const KeyEventArgs&)e).scancode , true);

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleKeyUpEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;
        CheckMovementKeys( ((const KeyEventArgs&)e).scancode, false );

        return true;
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::CheckMovementKeys( CEGUI::Key::Scan scancode, bool state )
    {
        using namespace CEGUI;

        switch ( scancode )
        {
            case Key::A:
                mMoveLeft = state;
                break;

            case Key::D:
                mMoveRight = state;
                break;

            case Key::S:
                mMoveBck = state;
                break;

            case Key::W:
                mMoveFwd = state;
                break;

            default:
                break;

        }

        mProcessMovement = mMoveLeft || mMoveRight || mMoveFwd || mMoveBck;

    }

//-----------------------------------------------------------------------------------
    void CompositorDemo_FrameListener::itemStateChanged(const size_t index, const bool state)
    {
        // get the item text and tell compositor manager to set enable state
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(mMain->getRenderWindow()->getViewport(0),
            mCompositorSelectorViewManager->getItemSelectorText(index), state);
		updateDebugRTTWindow();
    }
//-----------------------------------------------------------------------------------
    void CompositorDemo_FrameListener::registerCompositors(void)
    {
        Ogre::Viewport *vp = mMain->getRenderWindow()->getViewport(0);
        hvListener = new HeatVisionListener();
		hdrListener = new HDRListener();
		gaussianListener = new GaussianListener();

        mCompositorSelectorViewManager = new ItemSelectorViewManager("CompositorSelectorWin");
        // tell view manager to notify us when an item changes selection state
        mCompositorSelectorViewManager->setItemSelectorController(this);
        //iterate through Compositor Managers resources and add name keys ast Item selectors to Compositor selector view manager
        Ogre::CompositorManager::ResourceMapIterator resourceIterator =
            Ogre::CompositorManager::getSingleton().getResourceIterator();

        // add all compositor resources to the view container
        while (resourceIterator.hasMoreElements())
        {
            Ogre::ResourcePtr resource = resourceIterator.getNext();
            const Ogre::String& compositorName = resource->getName();
            // Don't add base Ogre/Scene compositor to view
            if (compositorName == "Ogre/Scene")
                continue;

            mCompositorSelectorViewManager->addItemSelector(compositorName);
			int addPosition = -1;
			if (compositorName == "HDR")
			{
				// HDR must be first in the chain
				addPosition = 0;
			}
            Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(vp, compositorName, addPosition);
            Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, compositorName, false);
            // special handling for Heat Vision which uses a listener
            if(instance && (compositorName == "Heat Vision"))
                instance->addListener(hvListener);
			else if(instance && (compositorName == "HDR"))
			{
				instance->addListener(hdrListener);
				hdrListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
				hdrListener->notifyCompositor(instance);

			}
			else if(instance && (compositorName == "Gaussian Blur"))
			{
				instance->addListener(gaussianListener);
				gaussianListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
			}
        }
    }
	//---------------------------------------------------------------------
	void CompositorDemo_FrameListener::initDebugRTTWindow(void)
	{
		mDebugRTTStaticImage = CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"DebugRTTImage");
		mDebugRTTListbox = static_cast<CEGUI::Listbox*>(
			CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"DebugRTTListbox"));
		mDebugRTTListbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
			CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleRttSelection, this));
	}
	//---------------------------------------------------------------------
	bool CompositorDemo_FrameListener::handleRttSelection(const CEGUI::EventArgs& e)
	{
		if (mDebugRTTListbox->getSelectedCount() > 0)
		{
			// image set is in user data
			CEGUI::Imageset* imgSet = (CEGUI::Imageset*)mDebugRTTListbox->getFirstSelectedItem()->getUserData();

			mDebugRTTStaticImage->setProperty("Image",
                CEGUI::PropertyHelper::imageToString(&imgSet->getImage("RttImage")));

		}
		else
		{
			mDebugRTTStaticImage->setProperty("Image", "");

		}
		return true;
	}
	//---------------------------------------------------------------------
	void CompositorDemo_FrameListener::updateDebugRTTWindow(void)
	{
		// Clear listbox
		mDebugRTTListbox->resetList();
		// Clear imagesets
		mDebugRTTStaticImage->setProperty("Image", "");
		for (ImageSetList::iterator isIt = mDebugRTTImageSets.begin();
			isIt != mDebugRTTImageSets.end(); ++isIt)
		{
			CEGUI::ImagesetManager::getSingleton().destroyImageset(*isIt);
		}
		mDebugRTTImageSets.clear();
		Ogre::set<Ogre::String>::type uniqueTextureNames;
		// Add an entry for each render texture for all active compositors
		Ogre::Viewport* vp = mMain->getRenderWindow()->getViewport(0);
		Ogre::CompositorChain* chain = Ogre::CompositorManager::getSingleton().getCompositorChain(vp);
		Ogre::CompositorChain::InstanceIterator it = chain->getCompositors();
		while (it.hasMoreElements())
		{
			Ogre::CompositorInstance* inst = it.getNext();
			if (inst->getEnabled())
			{
				Ogre::CompositionTechnique::TextureDefinitionIterator texIt =
					inst->getTechnique()->getTextureDefinitionIterator();
				while (texIt.hasMoreElements())
				{
					Ogre::CompositionTechnique::TextureDefinition* texDef = texIt.getNext();

					// Get instance name of texture (NB only index 0 if MRTs for now)
					const Ogre::String& instName = inst->getTextureInstanceName(texDef->name, 0);
					// Create CEGUI texture from name of OGRE texture
					CEGUI::Texture* tex = mMain->getGuiRenderer()->createTexture(instName.c_str());
					// Create imageset
					// Note that if we use shared textures in compositor, the same texture name
					// will occur more than once, so we have to cater for this
					if (uniqueTextureNames.find(instName) == uniqueTextureNames.end())
					{
						CEGUI::Imageset* imgSet =
							CEGUI::ImagesetManager::getSingleton().createImageset(
								instName.c_str(), tex);
						mDebugRTTImageSets.push_back(imgSet);
						imgSet->defineImage((CEGUI::utf8*)"RttImage",
							CEGUI::Point(0.0f, 0.0f),
							CEGUI::Size(tex->getWidth(), tex->getHeight()),
							CEGUI::Point(0.0f,0.0f));


						CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(texDef->name.c_str(), 0, imgSet);
						item->setSelectionBrushImage("TaharezLook", "ListboxSelectionBrush");
						item->setSelectionColours(CEGUI::colour(0,0,1));
						mDebugRTTListbox->addItem(item);

						uniqueTextureNames.insert(instName);
					}

				}

			}

		}



	}


