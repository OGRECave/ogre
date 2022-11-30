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

        /*-----------------------------------------------------------------------------
        | Manually update the cursor position after being unpaused.
        -----------------------------------------------------------------------------*/
        void unpaused() override
        {
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
            if(!mTrayMgr) return true;

            mTrayMgr->frameRendered(evt);
            mControls->frameRendered(evt);

            if (!mTrayMgr->isDialogVisible())
            {
                mCameraMan->frameRendered(evt);   // if dialog isn't up, then update the camera
            }

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

            mControls->keyPressed(evt);
            mCameraMan->keyPressed(evt);
            return true;
        }

        bool keyReleased(const KeyboardEvent& evt) override
        {
            mCameraMan->keyReleased(evt);

            return true;
        }

        /* IMPORTANT: When overriding these following handlers, remember to allow the tray manager
        to filter out any interface-related mouse events before processing them in your scene.
        If the tray manager handler returns true, the event was meant for the trays, not you. */
        bool mouseMoved(const MouseMotionEvent& evt) override
        {
            if (mTrayMgr->mouseMoved(evt)) return true;

            mCameraMan->mouseMoved(evt);
            return true;
        }

        // convert and redirect
        bool touchMoved(const TouchFingerEvent& evt) override {
            MouseMotionEvent e;
            e.xrel = evt.dx * mWindow->getWidth();
            e.yrel = evt.dy * mWindow->getHeight();
            return mouseMoved(e);
        }

        bool mousePressed(const MouseButtonEvent& evt) override
        {
            if (mTrayMgr->mousePressed(evt)) return true;

            if (mDragLook && evt.button == BUTTON_LEFT)
            {
                mCameraMan->setStyle(CS_FREELOOK);
                mTrayMgr->hideCursor();
            }

            mCameraMan->mousePressed(evt);
            return true;
        }

        // convert and redirect
        bool touchPressed(const TouchFingerEvent& evt) override {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mousePressed(e);
        }

        bool mouseReleased(const MouseButtonEvent& evt) override
        {
            if (mTrayMgr->mouseReleased(evt)) return true;

            if (mDragLook && evt.button == BUTTON_LEFT)
            {
                mCameraMan->setStyle(CS_MANUAL);
                mTrayMgr->showCursor();
            }

            mCameraMan->mouseReleased(evt);
            return true;
        }

        // convert and redirect
        bool touchReleased(const TouchFingerEvent& evt) override {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mouseReleased(e);
        }

        bool mouseWheelRolled(const MouseWheelEvent& evt) override {
            if(mTrayMgr->mouseWheelRolled(evt))
                return true;
            mCameraMan->mouseWheelRolled(evt);
            return true;
        }

        /*-----------------------------------------------------------------------------
        | Extended to setup a default tray interface and camera controller.
        -----------------------------------------------------------------------------*/
        void _setup(Ogre::RenderWindow* window, Ogre::FileSystemLayer* fsLayer, Ogre::OverlaySystem* overlaySys) override
        {
            Sample::_setup(window, fsLayer, overlaySys);

            if(mTrayMgr)
                mControls.reset(new AdvancedRenderControls(mTrayMgr.get(), mCamera));
        }

        void _shutdown() override
        {
            Sample::_shutdown();

            mControls.reset();
            mCameraMan.reset();
            mTrayMgr.reset();

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

        std::unique_ptr<AdvancedRenderControls> mControls; // sample details panel
        bool mCursorWasVisible;             // was cursor visible before dialog appeared
        bool mDragLook;                     // click and drag to free-look
    };
}

#endif
