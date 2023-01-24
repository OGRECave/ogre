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

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreViewport.h"
#include "OgreOverlay.h"
#include "OgreResourceGroupManager.h"
#include "OgreOverlayElementFactory.h"
#include "OgreStringConverter.h"
#include "OgreOverlayTranslator.h"

namespace Ogre {

    //---------------------------------------------------------------------
    template<> OverlayManager *Singleton<OverlayManager>::msSingleton = 0;
    OverlayManager* OverlayManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    OverlayManager& OverlayManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayManager() 
      : mLastViewportWidth(0), 
        mLastViewportHeight(0),
        mPixelRatio(1)
    {

        // Scripting is supported by this manager
        mScriptPatterns.push_back("*.overlay");
        ResourceGroupManager::getSingleton()._registerScriptLoader(this);
        mTranslatorManager.reset(new OverlayTranslatorManager());
    }
    //---------------------------------------------------------------------
    OverlayManager::~OverlayManager()
    {
        destroyAll();
        // Overlays notify OverlayElements of their destruction, so we destroy Overlays first
        destroyAllOverlayElements(false);
        destroyAllOverlayElements(true);

        for(FactoryMap::iterator i = mFactories.begin(); i != mFactories.end(); ++i)
        {
            OGRE_DELETE i->second;
        }

        // Unregister with resource group manager
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
    //---------------------------------------------------------------------
    void OverlayManager::_releaseManualHardwareResources()
    {
        for(ElementMap::iterator i = mElements.begin(), i_end = mElements.end(); i != i_end; ++i)
            i->second->_releaseManualHardwareResources();
    }
    //---------------------------------------------------------------------
    void OverlayManager::_restoreManualHardwareResources()
    {
        for(ElementMap::iterator i = mElements.begin(), i_end = mElements.end(); i != i_end; ++i)
            i->second->_restoreManualHardwareResources();
    }
    //---------------------------------------------------------------------
    const StringVector& OverlayManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getLoadingOrder(void) const
    {
        // Load late
        return 1100.0f;
    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::create(const String& name)
    {
        Overlay* ret = 0;
        OverlayMap::iterator i = mOverlayMap.find(name);

        if (i == mOverlayMap.end())
        {
            ret = OGRE_NEW Overlay(name);
            assert(ret && "Overlay creation failed");
            mOverlayMap[name] = ret;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "Overlay with name '" + name + "' already exists!",
                "OverlayManager::create");
        }

        return ret;

    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::getByName(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }

    }

    void OverlayManager::addOverlay(Overlay* overlay)
    {
        bool succ = mOverlayMap.emplace(overlay->getName(), overlay).second;
        if(succ) return;
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                    "Overlay with name '" + overlay->getName() + "' already exists!");
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Overlay with name '" + name + "' not found.",
                "OverlayManager::destroy");
        }
        else
        {
            OGRE_DELETE i->second;
            mOverlayMap.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(Overlay* overlay)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            if (i->second == overlay)
            {
                OGRE_DELETE i->second;
                mOverlayMap.erase(i);
                return;
            }
        }

        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "Overlay not found.",
            "OverlayManager::destroy");
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroyAll(void)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mOverlayMap.clear();
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayMapIterator OverlayManager::getOverlayIterator(void)
    {
        return OverlayMapIterator(mOverlayMap.begin(), mOverlayMap.end());
    }
    void OverlayManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        // skip scripts that were already loaded as we lack proper re-loading support
        if(!stream->getName().empty() && !mLoadedScripts.emplace(stream->getName()).second)
        {
            LogManager::getSingleton().logWarning(
                StringUtil::format("Skipping loading '%s' as it is already loaded", stream->getName().c_str()));
            return;
        }

        ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
    }
    //---------------------------------------------------------------------
    void OverlayManager::_queueOverlaysForRendering(Camera* cam, 
        RenderQueue* pQueue, Viewport* vp)
    {
        // Flag for update pixel-based GUIElements if viewport has changed dimensions
        if (mLastViewportWidth != int(vp->getActualWidth() / mPixelRatio) ||
            mLastViewportHeight != int(vp->getActualHeight() / mPixelRatio))
        {
            mLastViewportWidth = int(vp->getActualWidth() / mPixelRatio);
            mLastViewportHeight = int(vp->getActualHeight() / mPixelRatio);
        }

        OverlayMap::iterator i, iend;
        iend = mOverlayMap.end();
        for (i = mOverlayMap.begin(); i != iend; ++i)
        {
            Overlay* o = i->second;
            o->_findVisibleObjects(cam, pQueue, vp);
        }
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportHeight(void) const
    {
        return mLastViewportHeight;
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportWidth(void) const
    {
        return mLastViewportWidth;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getViewportAspectRatio(void) const
    {
        return (Real)mLastViewportWidth / (Real)mLastViewportHeight;
    }
    //---------------------------------------------------------------------
    float OverlayManager::getPixelRatio(void) const
    {
        return mPixelRatio;
    }
    //---------------------------------------------------------------------
    void OverlayManager::setPixelRatio(float ratio)
    {
        mPixelRatio = ratio;
    }
    //---------------------------------------------------------------------
    OverlayElement* OverlayManager::createOverlayElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool)
    {

        OverlayElement* newObj  = NULL;

        if (templateName.empty())
        {
            newObj = createOverlayElement(typeName, instanceName);
        }
        else
        {
            // no template 
            OverlayElement* templateGui = getOverlayElement(templateName, true);

            String typeNameToCreate;
            if (typeName.empty())
            {
                typeNameToCreate = templateGui->getTypeName();
            }
            else
            {
                typeNameToCreate = typeName;
            }

            newObj = createOverlayElement(typeNameToCreate, instanceName);

            ((OverlayContainer*)newObj)->copyFromTemplate(templateGui);
        }

        return newObj;
    }


    //---------------------------------------------------------------------
    OverlayElement* OverlayManager::cloneOverlayElementFromTemplate(const String& templateName, const String& instanceName)
    {
        OverlayElement* templateGui = getOverlayElement(templateName, true);
        return templateGui->clone(instanceName);
    }

    //---------------------------------------------------------------------
    OverlayElement* OverlayManager::createOverlayElement(const String& typeName, const String& instanceName, bool)
    {
        // Check not duplicated
        ElementMap::iterator ii = mElements.find(instanceName);
        if (ii != mElements.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "OverlayElement with name " + instanceName +
                " already exists.", "OverlayManager::createOverlayElement" );
        }
        OverlayElement* newElem = createOverlayElementFromFactory(typeName, instanceName);

        // Register
        mElements.emplace(instanceName, newElem);

        return newElem;


    }

    //---------------------------------------------------------------------
    OverlayElement* OverlayManager::createOverlayElementFromFactory(const String& typeName, const String& instanceName)
    {
        // Look up factory
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "OverlayManager::createOverlayElement");
        }

        // create
        return fi->second->createOverlayElement(instanceName);
    }
    //---------------------------------------------------------------------
    OverlayElement* OverlayManager::getOverlayElement(const String& name,bool)
    {
        // Locate instance
        ElementMap::iterator ii = mElements.find(name);
        if (ii == mElements.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + name + " not found.");
        }

        return ii->second;
    }
    //---------------------------------------------------------------------
    bool OverlayManager::hasOverlayElement(const String& name,bool)
    {
        ElementMap::iterator ii = mElements.find(name);
        return ii != mElements.end();
    }

    //---------------------------------------------------------------------
    void OverlayManager::destroyOverlayElement(OverlayElement* pInstance,bool)
    {
        destroyOverlayElement(pInstance->getName());
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroyOverlayElement(const String& instanceName,bool)
    {
        // Locate instance
        ElementMap::iterator ii = mElements.find(instanceName);
        if (ii == mElements.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + instanceName +
                " not found.", "OverlayManager::destroyOverlayElement" );
        }
        // Look up factory
        const String& typeName = ii->second->getTypeName();
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "OverlayManager::destroyOverlayElement");
        }

        fi->second->destroyOverlayElement(ii->second);
        mElements.erase(ii);
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroyAllOverlayElements(bool)
    {
        ElementMap::iterator i;

        while ((i = mElements.begin()) != mElements.end())
        {
            OverlayElement* element = i->second;

            // Get factory to delete
            FactoryMap::iterator fi = mFactories.find(element->getTypeName());
            if (fi == mFactories.end())
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element " 
                    + element->getName(),
                    "OverlayManager::destroyAllOverlayElements");
            }

            // remove from parent, if any
            OverlayContainer* parent;
            if ((parent = element->getParent()) != 0)
            {
                parent->_removeChild(element->getName());
            }

            // children of containers will be auto-removed when container is destroyed.
            // destroy the element and remove it from the list
            fi->second->destroyOverlayElement(element);
            mElements.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::addOverlayElementFactory(OverlayElementFactory* elemFactory)
    {
        // Add / replace
        mFactories[elemFactory->getTypeName()] = elemFactory;

        LogManager::getSingleton().logMessage("OverlayElementFactory for type " + elemFactory->getTypeName()
            + " registered.");
    }
}

