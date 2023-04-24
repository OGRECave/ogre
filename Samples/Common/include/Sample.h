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
#ifndef __Sample_H__
#define __Sample_H__

#include "OgreRoot.h"
#include "OgreOverlaySystem.h"
#include "OgreResourceManager.h"
#include "OgreComponents.h"

#include "OgreFileSystemLayer.h"

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#   include "OgreRTShaderSystem.h"
#endif //INCLUDE_RTSHADER_SYSTEM

#include "OgreInput.h"
#include "OgreTrays.h"
#include "OgreCameraMan.h"
#include "OgreMaterialManager.h"

namespace OgreBites
{


    /*=============================================================================
    | Base class responsible for everything specific to one sample.
    | Designed to be subclassed for each sample.
    =============================================================================*/
    class Sample : public InputListener, public TrayListener, public Ogre::GeneralAllocatedObject
    {
    public:
        /*=============================================================================
        | Utility comparison structure for sorting samples using SampleSet.
        =============================================================================*/
        struct Comparer
        {
            bool operator()(const Sample* a, const Sample* b) const
            {
                return a->getInfo().at("Title") < b->getInfo().at("Title");
            }
        };

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        Sample() : mShaderGenerator(0)
#else
        Sample()
#endif
        {
            mRoot = Ogre::Root::getSingletonPtr();
            mWindow = 0;
            mSceneMgr = 0;
            mDone = true;
            mResourcesLoaded = false;
            mContentSetup = false;

            mCamera = 0;
            mCameraNode = 0;
            mViewport = 0;

            mFSLayer = 0;
            mOverlaySystem = 0;

            // so we don't have to worry about checking if these keys exist later
            mInfo["Title"] = "Untitled";
            mInfo["Description"] = "";
            mInfo["Category"] = "Unsorted";
            mInfo["Thumbnail"] = "thumb_error.png";
            mInfo["Help"] = "";
        }

        virtual ~Sample() {}

        /*-----------------------------------------------------------------------------
        | Retrieves custom sample info.
        -----------------------------------------------------------------------------*/
        const Ogre::NameValuePairList& getInfo() const { return mInfo; }
        Ogre::NameValuePairList& getInfo() { return mInfo; }

        /*-----------------------------------------------------------------------------
        | Tests to see if target machine meets any special requirements of
        | this sample. Signal a failure by throwing an exception.
        -----------------------------------------------------------------------------*/
        virtual void testCapabilities(const Ogre::RenderSystemCapabilities* caps) {}

        void requireMaterial(const Ogre::String& name)
        {
            Ogre::StringStream err;
            err << "Material: " << name << " ";
            auto mat = Ogre::MaterialManager::getSingleton().getByName(name);
            if(!mat)
            {
                err << "not found";
                OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, err.str());
            }
            mat->load();
            if (mat->getSupportedTechniques().empty())
            {
                err << mat->getUnsupportedTechniquesExplanation();
                OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, err.str());
            }
        }

        /*-----------------------------------------------------------------------------
        | If this sample requires specific plugins to run, this method will be
        | used to return their names.
        -----------------------------------------------------------------------------*/
        virtual Ogre::StringVector getRequiredPlugins() { return Ogre::StringVector(); }

        Ogre::SceneManager* getSceneManager() { return mSceneMgr; }
        bool isDone() { return mDone; }

        /** Adds a screenshot frame to the list - this should
         *    be done during setup of the test. */
        void addScreenshotFrame(int frame)
        {
            mScreenshotFrames.insert(frame);
        }

        /** Returns whether or not a screenshot should be taken at the given frame */
        virtual bool isScreenshotFrame(int frame)
        {
            if (mScreenshotFrames.empty())
            {
                mDone = true;
            }
            else if (frame == *(mScreenshotFrames.begin()))
            {
                mScreenshotFrames.erase(mScreenshotFrames.begin());
                if (mScreenshotFrames.empty())
                    mDone = true;
                return true;
            }
            return false;
        }

        // enable trays GUI for this sample
        void _setupTrays(Ogre::RenderWindow* window)
        {
            mTrayMgr.reset(new TrayManager("SampleControls", window, this));  // create a tray interface
            // show stats and logo and hide the cursor
            mTrayMgr->showFrameStats(TL_BOTTOMLEFT);
            mTrayMgr->showLogo(TL_BOTTOMRIGHT);
            mTrayMgr->hideCursor();
        }

        /*-----------------------------------------------------------------------------
        | Sets up a sample. Used by the SampleContext class. Do not call directly.
        -----------------------------------------------------------------------------*/
        virtual void _setup(Ogre::RenderWindow* window, Ogre::FileSystemLayer* fsLayer, Ogre::OverlaySystem* overlaySys)
        {
            mOverlaySystem = overlaySys;
            mWindow = window;

            mFSLayer = fsLayer;

            locateResources();
            createSceneManager();
            setupView();

            mCameraMan.reset(new CameraMan(mCameraNode));   // create a default camera controller

            loadResources();
            mResourcesLoaded = true;
            setupContent();
            mContentSetup = true;

            mDone = false;
        }

        /*-----------------------------------------------------------------------------
        | Shuts down a sample. Used by the SampleContext class. Do not call directly.
        -----------------------------------------------------------------------------*/
        virtual void _shutdown()
        {
            Ogre::ControllerManager::getSingleton().clearControllers();

            if (mContentSetup)
                cleanupContent();
            if (mSceneMgr)
                mSceneMgr->clearScene();
            mContentSetup = false;

            if (mResourcesLoaded)
                unloadResources();
            mResourcesLoaded = false;
            if (mSceneMgr) 
            {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
                mShaderGenerator->removeSceneManager(mSceneMgr);
#endif
                mSceneMgr->removeRenderQueueListener(mOverlaySystem);
                mRoot->destroySceneManager(mSceneMgr);              
            }
            mSceneMgr = 0;

            mDone = true;
        }

        /*-----------------------------------------------------------------------------
        | Actions to perform when the context stops sending frame listener events
        | and input device events to this sample.
        -----------------------------------------------------------------------------*/
        virtual void paused() {}

        /*-----------------------------------------------------------------------------
        | Actions to perform when the context continues sending frame listener
        | events and input device events to this sample.
        -----------------------------------------------------------------------------*/
        virtual void unpaused() {}

        /*-----------------------------------------------------------------------------
        | Saves the sample state. Optional. Used during reconfiguration.
        -----------------------------------------------------------------------------*/
        virtual void saveState(Ogre::NameValuePairList& state) {}

        /*-----------------------------------------------------------------------------
        | Restores the sample state. Optional. Used during reconfiguration.
        -----------------------------------------------------------------------------*/
        virtual void restoreState(Ogre::NameValuePairList& state) {}

        // callback interface copied from various listeners to be used by SampleContext

        virtual bool frameStarted(const Ogre::FrameEvent& evt) { return true; }
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) { return true; }
        virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
        virtual void windowMoved(Ogre::RenderWindow* rw) {}
        virtual void windowResized(Ogre::RenderWindow* rw) {}
        virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }
        virtual void windowClosed(Ogre::RenderWindow* rw) {}
        virtual void windowFocusChange(Ogre::RenderWindow* rw) {}
    protected:

        /*-----------------------------------------------------------------------------
        | Finds sample-specific resources. No such effort is made for most samples,
        | but this is useful for special samples with large, exclusive resources.
        -----------------------------------------------------------------------------*/
        virtual void locateResources() {}

        /*-----------------------------------------------------------------------------
        | Loads sample-specific resources. No such effort is made for most samples,
        | but this is useful for special samples with large, exclusive resources.
        -----------------------------------------------------------------------------*/
        virtual void loadResources() {}

        /*-----------------------------------------------------------------------------
        | Creates a scene manager for the sample. A generic one is the default,
        | but many samples require a special kind of scene manager.
        -----------------------------------------------------------------------------*/
        virtual void createSceneManager()
        {
            mSceneMgr = Ogre::Root::getSingleton().createSceneManager();
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
            mShaderGenerator->addSceneManager(mSceneMgr);
            auto mainRenderState = mShaderGenerator->getRenderState(Ogre::MSN_SHADERGEN);
            // reset global light state
            mainRenderState->setLightCount(0);
            mainRenderState->setLightCountAutoUpdate(true);
            mainRenderState->resetToBuiltinSubRenderStates();
#endif
            if(mOverlaySystem)
                mSceneMgr->addRenderQueueListener(mOverlaySystem);
        }

        /*-----------------------------------------------------------------------------
        | Sets up viewport layout and camera.
        -----------------------------------------------------------------------------*/
        virtual void setupView() {}

        /*-----------------------------------------------------------------------------
        | Sets up the scene (and anything else you want for the sample).
        -----------------------------------------------------------------------------*/
        virtual void setupContent() {}

        /*-----------------------------------------------------------------------------
        | Cleans up the scene (and anything else you used).
        -----------------------------------------------------------------------------*/
        virtual void cleanupContent() {}

        /*-----------------------------------------------------------------------------
        | Unloads sample-specific resources. My method here is simple and good
        | enough for most small samples, but your needs may vary.
        -----------------------------------------------------------------------------*/
        virtual void unloadResources()
        {
            for (auto& it : Ogre::ResourceGroupManager::getSingleton().getResourceManagers())
            {
                it.second->unloadUnreferencedResources();
            }
        }   

        Ogre::Root* mRoot;                // OGRE root object
        Ogre::OverlaySystem* mOverlaySystem; // OverlaySystem
        Ogre::RenderWindow* mWindow;      // context render window
        Ogre::FileSystemLayer* mFSLayer;          // file system abstraction layer
        Ogre::SceneManager* mSceneMgr;    // scene manager for this sample
        Ogre::NameValuePairList mInfo;    // custom sample info

        Ogre::Viewport* mViewport;          // main viewport
        Ogre::Camera* mCamera;              // main camera
        Ogre::SceneNode* mCameraNode;       // camera node

        // SdkSample fields
        std::unique_ptr<TrayManager> mTrayMgr;           // tray interface manager
        std::unique_ptr<CameraMan> mCameraMan;           // basic camera controller

        bool mDone;               // flag to mark the end of the sample
        bool mResourcesLoaded;    // whether or not resources have been loaded
        bool mContentSetup;       // whether or not scene was created
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        Ogre::RTShader::ShaderGenerator*            mShaderGenerator;           // The Shader generator instance.
    public:
        void setShaderGenerator(Ogre::RTShader::ShaderGenerator* shaderGenerator) 
        { 
            mShaderGenerator = shaderGenerator;
        }
#endif
    private:
        // VisualTest fields
        std::set<int> mScreenshotFrames;
    };

    typedef std::set<Sample*, Sample::Comparer> SampleSet;
}

#endif
