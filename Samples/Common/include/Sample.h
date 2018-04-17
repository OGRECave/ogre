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

#include "OgreFileSystemLayer.h"

#ifdef INCLUDE_RTSHADER_SYSTEM
#   include "OgreRTShaderSystem.h"
#endif //INCLUDE_RTSHADER_SYSTEM

#include "OgreInput.h"

namespace OgreBites
{
    /*=============================================================================
    | Base class responsible for everything specific to one sample.
    | Designed to be subclassed for each sample.
    =============================================================================*/
    class Sample : public Ogre::GeneralAllocatedObject
    {
    public:
        /*=============================================================================
        | Utility comparison structure for sorting samples using SampleSet.
        =============================================================================*/
        struct Comparer
        {
            bool operator() (const Sample* a, const Sample* b) const
            {
                auto aTitle = a->getInfo().find("Title");
                auto bTitle = b->getInfo().find("Title");
                
                if (aTitle != a->getInfo().end() && bTitle != b->getInfo().end())
                    return aTitle->second.compare(bTitle->second) < 0;
                else return false;
            }
        };

#ifdef INCLUDE_RTSHADER_SYSTEM
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

            mFSLayer = 0;
            mOverlaySystem = 0;
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

        /*-----------------------------------------------------------------------------
        | If this sample requires a specific render system to run, this method
        | will be used to return its name.
        -----------------------------------------------------------------------------*/
        virtual Ogre::String getRequiredRenderSystem()
        {
            return "";
        }

        /*-----------------------------------------------------------------------------
        | If this sample requires specific plugins to run, this method will be
        | used to return their names.
        -----------------------------------------------------------------------------*/
        virtual Ogre::StringVector getRequiredPlugins()
        {
            return Ogre::StringVector();
        }

        Ogre::SceneManager* getSceneManager()
        {
            return mSceneMgr;
        }

        bool isDone()
        {
            return mDone;
        }

        /*-----------------------------------------------------------------------------
        | Sets up a sample. Used by the SampleContext class. Do not call directly.
        -----------------------------------------------------------------------------*/
        virtual void _setup(Ogre::RenderWindow* window, Ogre::FileSystemLayer* fsLayer, Ogre::OverlaySystem* overlaySys)
        {
            // assign mRoot here in case Root was initialised after the Sample's constructor ran.
            mRoot = Ogre::Root::getSingletonPtr();
            mOverlaySystem = overlaySys;
            mWindow = window;

            mFSLayer = fsLayer;

            locateResources();
            createSceneManager();
            setupView();

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
#ifdef INCLUDE_RTSHADER_SYSTEM
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
        virtual bool keyPressed(const KeyboardEvent& evt) { return true; }
        virtual bool keyReleased(const KeyboardEvent& evt) { return true; }
        virtual bool touchMoved(const TouchFingerEvent& evt) { return true; }
        virtual bool touchPressed(const TouchFingerEvent& evt) { return true; }
        virtual bool touchReleased(const TouchFingerEvent& evt) { return true; }
        virtual bool mouseMoved(const MouseMotionEvent& evt) { return true; }
        virtual bool mouseWheelRolled(const MouseWheelEvent& evt) { return true; }
        virtual bool mousePressed(const MouseButtonEvent& evt) { return true; }
        virtual bool mouseReleased(const MouseButtonEvent& evt) { return true; }
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
#ifdef INCLUDE_RTSHADER_SYSTEM
            mShaderGenerator->addSceneManager(mSceneMgr);
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
            Ogre::ResourceGroupManager::ResourceManagerIterator resMgrs =
            Ogre::ResourceGroupManager::getSingleton().getResourceManagerIterator();

            while (resMgrs.hasMoreElements())
            {
                resMgrs.getNext()->unloadUnreferencedResources();
            }
        }   

        Ogre::Root* mRoot;                // OGRE root object
        Ogre::OverlaySystem* mOverlaySystem; // OverlaySystem
        Ogre::RenderWindow* mWindow;      // context render window
        Ogre::FileSystemLayer* mFSLayer;          // file system abstraction layer
        Ogre::SceneManager* mSceneMgr;    // scene manager for this sample
        Ogre::NameValuePairList mInfo;    // custom sample info
        bool mDone;                       // flag to mark the end of the sample
        bool mResourcesLoaded;    // whether or not resources have been loaded
        bool mContentSetup;       // whether or not scene was created
#ifdef INCLUDE_RTSHADER_SYSTEM
        Ogre::RTShader::ShaderGenerator*            mShaderGenerator;           // The Shader generator instance.
    public:
        void setShaderGenerator(Ogre::RTShader::ShaderGenerator* shaderGenerator) 
        { 
            mShaderGenerator = shaderGenerator;
        };
#endif
    };

    typedef std::set<Sample*, Sample::Comparer> SampleSet;
}

#endif
