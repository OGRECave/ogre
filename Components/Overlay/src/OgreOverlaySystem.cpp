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

#include "OgreOverlaySystem.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayElementFactory.h"
#include "OgreFontManager.h"

namespace Ogre {
    //---------------------------------------------------------------------
    /** Factory for creating PanelOverlayElement instances. */
    class PanelOverlayElementFactory: public OverlayElementFactory
    {
    public:
        OverlayElement* createOverlayElement(const String& instanceName) override
        {
            return OGRE_NEW PanelOverlayElement(instanceName);
        }
        const String& getTypeName(void) const override
        {
            static String name = "Panel";
            return name;
        }
    };

    /** Factory for creating BorderPanelOverlayElement instances. */
    class BorderPanelOverlayElementFactory: public OverlayElementFactory
    {
    public:
        OverlayElement* createOverlayElement(const String& instanceName) override
        {
            return OGRE_NEW BorderPanelOverlayElement(instanceName);
        }
        const String& getTypeName(void) const override
        {
            static String name = "BorderPanel";
            return name;
        }
    };

    /** Factory for creating TextAreaOverlayElement instances. */
    class TextAreaOverlayElementFactory: public OverlayElementFactory
    {
    public:
        OverlayElement* createOverlayElement(const String& instanceName) override
        {
            return OGRE_NEW TextAreaOverlayElement(instanceName);
        }
        const String& getTypeName(void) const override
        {
            static String name = "TextArea";
            return name;
        }
    };

    template<> OverlaySystem *Singleton<OverlaySystem>::msSingleton = 0;
    OverlaySystem* OverlaySystem::getSingletonPtr()
    {
        return msSingleton;
    }
    OverlaySystem& OverlaySystem::getSingleton()
    {
        assert( msSingleton );  return ( *msSingleton );
    }
    //---------------------------------------------------------------------
    OverlaySystem::OverlaySystem()
    {
        RenderSystem::setSharedListener(this);

        mOverlayManager = OGRE_NEW Ogre::OverlayManager();
        mOverlayManager->addOverlayElementFactory(OGRE_NEW Ogre::PanelOverlayElementFactory());

        mOverlayManager->addOverlayElementFactory(OGRE_NEW Ogre::BorderPanelOverlayElementFactory());

        mOverlayManager->addOverlayElementFactory(OGRE_NEW Ogre::TextAreaOverlayElementFactory());

        mFontManager = OGRE_NEW FontManager();
        if (auto prof = Profiler::getSingletonPtr())
        {
            mProfileListener = new Ogre::OverlayProfileSessionListener();
            prof->addListener(mProfileListener);
        }
    }
    //---------------------------------------------------------------------
    OverlaySystem::~OverlaySystem()
    {
        if(RenderSystem::getSharedListener() == this)
            RenderSystem::setSharedListener(0);

        if (auto prof = Profiler::getSingletonPtr())
        {
            prof->removeListener(mProfileListener);
            delete mProfileListener;
        }

        OGRE_DELETE mOverlayManager;
        OGRE_DELETE mFontManager;
    }
    //---------------------------------------------------------------------
    void OverlaySystem::preRenderQueues()
    {
        // this should really have been "postFindVisibleObjects", but changing it now will break too much existing code..
        Ogre::Viewport* vp = Ogre::Root::getSingletonPtr()->getRenderSystem()->_getViewport();
        if(vp != NULL)
        {
            Ogre::SceneManager* sceneMgr = vp->getCamera()->getSceneManager();
            if (vp->getOverlaysEnabled() && sceneMgr->_getCurrentRenderStage() != Ogre::SceneManager::IRS_RENDER_TO_TEXTURE)
            {
                OverlayManager::getSingleton()._queueOverlaysForRendering(vp->getCamera(), sceneMgr->getRenderQueue(), vp);
            }
        }
    }
    //---------------------------------------------------------------------
	void OverlaySystem::eventOccurred(const String& eventName, const NameValuePairList* parameters)
	{
		if(eventName == "DeviceLost")
		{
			mOverlayManager->_releaseManualHardwareResources();
		}
		else if(eventName == "DeviceRestored")
		{
			mOverlayManager->_restoreManualHardwareResources();
		}
	}
	//---------------------------------------------------------------------
}
