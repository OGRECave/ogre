/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2014 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */
#ifndef __SdkSample_H__
#define __SdkSample_H__

#include "Sample.h"
#include "Ogre.h"
#include "OgreAdvancedRenderControls.h"

namespace OgreBites
{
    class TouchAgnosticInputListenerChain : public InputListenerChain
    {
        Ogre::RenderWindow* mWindow;
    public:
        TouchAgnosticInputListenerChain() : mWindow(NULL) {}
        TouchAgnosticInputListenerChain(Ogre::RenderWindow* window, std::vector<InputListener*> chain)
            : InputListenerChain(chain), mWindow(window)
        {
        }

        // convert and redirect
        bool touchMoved(const TouchFingerEvent& evt) override {
            MouseMotionEvent e;
            e.x = evt.x * mWindow->getWidth();
            e.y = evt.y * mWindow->getHeight();
            e.xrel = evt.dx * mWindow->getWidth();
            e.yrel = evt.dy * mWindow->getHeight();
            return mouseMoved(e);
        }
        bool touchPressed(const TouchFingerEvent& evt) override {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mousePressed(e);
        }
        bool touchReleased(const TouchFingerEvent& evt) override {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mouseReleased(e);
        }
    };

    /*=============================================================================
    // Base SDK sample class. Includes default player camera and SDK trays.
    =============================================================================*/
    class SdkSample : public Sample
    {
    public:
        SdkSample()
        {
            mCameraMan = 0;
            mCursorWasVisible = false;
            mDragLook = false;
        }

        void paused() override { mContext->removeInputListener(&mInputListenerChain); }

        /*-----------------------------------------------------------------------------
        | Manually update the cursor position after being unpaused.
        -----------------------------------------------------------------------------*/
        void unpaused() override
        {
            mContext->addInputListener(&mInputListenerChain);
            mTrayMgr->refreshCursor();
        }

        /*-----------------------------------------------------------------------------
        | Automatically saves position and orientation for free-look cameras.
        -----------------------------------------------------------------------------*/
        void saveState(Ogre::NameValuePairList& state) override
        {
            if (mCameraMan->getStyle() == CS_FREELOOK)
            {
                state["CameraPosition"] = Ogre::StringConverter::toString(mCameraNode->getPosition());
                state["CameraOrientation"] = Ogre::StringConverter::toString(mCameraNode->getOrientation());
            }
        }

        /*-----------------------------------------------------------------------------
        | Automatically restores position and orientation for free-look cameras.
        -----------------------------------------------------------------------------*/
        void restoreState(Ogre::NameValuePairList& state) override
        {
            if (state.find("CameraPosition") != state.end() && state.find("CameraOrientation") != state.end())
            {
                mCameraMan->setStyle(CS_FREELOOK);
                mCameraNode->setPosition(Ogre::StringConverter::parseVector3(state["CameraPosition"]));
                mCameraNode->setOrientation(Ogre::StringConverter::parseQuaternion(state["CameraOrientation"]));
            }
        }

        bool frameRenderingQueued(const Ogre::FrameEvent& evt) override
        {
            if (mTrayMgr && mTrayMgr->isDialogVisible())
                return true;

            mInputListenerChain.frameRendered(evt);
            return true;
        }

        bool keyPressed(const KeyboardEvent& evt) override
        {
        	int key = evt.keysym.sym;
        	
            if (key == 'h' || key == SDLK_F1)   // toggle visibility of help dialog
            {
                if (!mTrayMgr->isDialogVisible() && mInfo["Help"] != "") mTrayMgr->showOkDialog("Help", mInfo["Help"]);
                else mTrayMgr->closeDialog();
            }

            if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

            // pass down
            return false;
        }

        bool mousePressed(const MouseButtonEvent& evt) override
        {
            if (mDragLook && evt.button == BUTTON_LEFT)
            {
                mCameraMan->setStyle(CS_FREELOOK);
                mTrayMgr->hideCursor();
            }

            // pass down
            return false;
        }

        bool mouseReleased(const MouseButtonEvent& evt) override
        {
            if (mDragLook && evt.button == BUTTON_LEFT)
            {
                mCameraMan->setStyle(CS_MANUAL);
                mTrayMgr->showCursor();
            }

            // pass down
            return false;
        }

        /*-----------------------------------------------------------------------------
        | Extended to setup a default tray interface and camera controller.
        -----------------------------------------------------------------------------*/
        void _setup(ApplicationContextBase* context) override
        {
            mTrayMgr.reset(new TrayManager("SampleControls", context->getRenderWindow(), this));  // create a tray interface
            // show stats and logo and hide the cursor
            mTrayMgr->showFrameStats(TL_BOTTOMLEFT);
            mTrayMgr->showLogo(TL_BOTTOMRIGHT);
            mTrayMgr->hideCursor();

            Sample::_setup(context);

            mControls.reset(new AdvancedRenderControls(mTrayMgr.get(), mCamera));

            if (mInputListenerChain.empty()) // allow overrides
                mInputListenerChain =
                    TouchAgnosticInputListenerChain(mWindow, {mTrayMgr.get(), this, mCameraMan.get(), mControls.get()});

            mContext->addInputListener(&mInputListenerChain);
        }

        void _removeTrays()
        {
            mControls.reset();
            mTrayMgr.reset();
            mInputListenerChain = TouchAgnosticInputListenerChain(mWindow, {});
        }

        void _shutdown() override
        {
            if(mContext)
                mContext->removeInputListener(&mInputListenerChain);

            Sample::_shutdown();

            mControls.reset();
            mCameraMan.reset();
            mTrayMgr.reset();
            mInputListenerChain = TouchAgnosticInputListenerChain(mWindow, {});

            // restore settings we may have changed, so as not to affect other samples
            Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_BILINEAR);
            Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(1);
        }

    protected:

        void setupView() override
        {
            // setup default viewport layout and camera
            mCamera = mSceneMgr->createCamera("MainCamera");
            mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            mCameraNode->attachObject(mCamera);
            mCameraNode->setFixedYawAxis(true);
            mCameraMan.reset(new CameraMan(mCameraNode));   // create a default camera controller

            mViewport = mWindow->addViewport(mCamera);
            mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
            mCamera->setAutoAspectRatio(true);
            mCamera->setNearClipDistance(5);
        }

        virtual void setDragLook(bool enabled)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            return;
#endif
            if (enabled)
            {
                mCameraMan->setStyle(CS_MANUAL);
                mTrayMgr->showCursor();
                mDragLook = true;
            }
            else
            {
                mCameraMan->setStyle(CS_FREELOOK);
                mTrayMgr->hideCursor();
                mDragLook = false;
            }
        }

        void addTextureDebugOverlay(TrayLocation loc, const Ogre::TexturePtr& tex, size_t i)
        {
            using namespace Ogre;
            // Create material
            String matName = "Ogre/DebugTexture" + StringConverter::toString(i);
            MaterialPtr debugMat = MaterialManager::getSingleton().getByName(
                matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (!debugMat)
            {
                debugMat = MaterialManager::getSingleton().create(matName,
                                                                  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            }
            Pass* p = debugMat->getTechnique(0)->getPass(0);
            p->removeAllTextureUnitStates();
            p->setLightingEnabled(false);
            TextureUnitState *t = p->createTextureUnitState(tex->getName());
            t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

            // create template
            if (!OverlayManager::getSingleton().hasOverlayElement("Ogre/DebugTexOverlay", true))
            {
                OverlayElement* e = OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexOverlay", true);
                e->setMetricsMode(GMM_PIXELS);
                e->setWidth(128);
                e->setHeight(128);
            }

            // add widget
            String widgetName = "DebugTex"+ StringConverter::toString(i);
            Widget* w = mTrayMgr->getWidget(widgetName);
            if (!w)
            {
                w = mTrayMgr->createDecorWidget(loc, widgetName, "Ogre/DebugTexOverlay");
            }
            w->getOverlayElement()->setMaterial(debugMat);
        }

        std::unique_ptr<TrayManager> mTrayMgr;           // tray interface manager
        std::unique_ptr<CameraMan> mCameraMan;           // basic camera controller
        std::unique_ptr<AdvancedRenderControls> mControls; // sample details panel
        TouchAgnosticInputListenerChain mInputListenerChain;
        bool mCursorWasVisible;             // was cursor visible before dialog appeared
        bool mDragLook;                     // click and drag to free-look
    };
}

#endif
