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
#include "OgreStableHeaders.h"
#include "OgreMaterialManager.h"

#include "OgreMaterialSerializer.h"
#include "OgreTechnique.h"
#include "OgreScriptCompiler.h"
#include "OgreLodStrategyManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> MaterialManager* Singleton<MaterialManager>::msSingleton = 0;
    MaterialManager* MaterialManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    MaterialManager& MaterialManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }
    String MaterialManager::DEFAULT_SCHEME_NAME = "Default";
    //-----------------------------------------------------------------------
    MaterialManager::MaterialManager() : OGRE_THREAD_POINTER_INIT(mSerializer)
    {
        // Create primary thread copies of script compiler / serializer
        // other copies for other threads may also be instantiated
        OGRE_THREAD_POINTER_SET(mSerializer, OGRE_NEW MaterialSerializer());

        // Loading order
        mLoadOrder = 100.0f;
        // Scripting is supported by this manager

        // Resource type
        mResourceType = "Material";

        // Register with resource group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

        // Default scheme
        mActiveSchemeIndex = 0;
        mActiveSchemeName = DEFAULT_SCHEME_NAME;
        mSchemes[mActiveSchemeName] = 0;

    }
    //-----------------------------------------------------------------------
    MaterialManager::~MaterialManager()
    {
        mDefaultSettings.setNull();
        // Resources cleared by superclass
        // Unregister with resource group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);

        // delete primary thread instances directly, other threads will delete
        // theirs automatically when the threads end.
        OGRE_THREAD_POINTER_DELETE(mSerializer);
    }
    //-----------------------------------------------------------------------
    Resource* MaterialManager::createImpl(const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader,
    const NameValuePairList* params)
    {
        return OGRE_NEW Material(this, name, handle, group, isManual, loader);
    }
    //-----------------------------------------------------------------------
    MaterialPtr MaterialManager::create (const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return createResource(name,group,isManual,loader,createParams).staticCast<Material>();
    }
    //-----------------------------------------------------------------------
    MaterialPtr MaterialManager::getByName(const String& name, const String& groupName)
    {
        return getResourceByName(name, groupName).staticCast<Material>();
    }
    //-----------------------------------------------------------------------
    void MaterialManager::initialise(void)
    {
        // Set up default material - don't use name constructor as we want to avoid applying defaults
        mDefaultSettings = create("DefaultSettings", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        // Add a single technique and pass, non-programmable
        mDefaultSettings->createTechnique()->createPass();

        // Set up a lit base white material
        create("BaseWhite", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        // Set up an unlit base white material
        MaterialPtr baseWhiteNoLighting = create("BaseWhiteNoLighting",
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    }
    //-----------------------------------------------------------------------
    void MaterialManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
    }
    //-----------------------------------------------------------------------
    unsigned short MaterialManager::_getSchemeIndex(const String& schemeName)
    {
        unsigned short ret = 0;
        SchemeMap::iterator i = mSchemes.find(schemeName);
        if (i != mSchemes.end())
        {
            ret = i->second;
        }
        else
        {
            // Create new
            ret = static_cast<unsigned short>(mSchemes.size());
            mSchemes[schemeName] = ret;
        }
        return ret;

    }
    //-----------------------------------------------------------------------
    const String& MaterialManager::_getSchemeName(unsigned short index)
    {
        for (SchemeMap::iterator i = mSchemes.begin(); i != mSchemes.end(); ++i)
        {
            if (i->second == index)
                return i->first;
        }
        return DEFAULT_SCHEME_NAME;
    }
    //-----------------------------------------------------------------------
    unsigned short MaterialManager::_getActiveSchemeIndex(void) const
    {
        return mActiveSchemeIndex;
    }
    //-----------------------------------------------------------------------
    const String& MaterialManager::getActiveScheme(void) const
    {
        return mActiveSchemeName;
    }
    //-----------------------------------------------------------------------
    void MaterialManager::setActiveScheme(const String& schemeName)
    {
        if (mActiveSchemeName != schemeName)
        {   
            // Allow the creation of new scheme indexes on demand
            // even if they're not specified in any Technique
            mActiveSchemeIndex = _getSchemeIndex(schemeName);
            mActiveSchemeName = schemeName;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialManager::addListener(Listener* l, const Ogre::String& schemeName)
    {
        mListenerMap[schemeName].push_back(l);
    }
    //---------------------------------------------------------------------
    void MaterialManager::removeListener(Listener* l, const Ogre::String& schemeName)
    {
        mListenerMap[schemeName].remove(l);
    }
    //---------------------------------------------------------------------
    Technique* MaterialManager::_arbitrateMissingTechniqueForActiveScheme(
        Material* mat, unsigned short lodIndex, const Renderable* rend)
    {
        //First, check the scheme specific listeners
        ListenerMap::iterator it = mListenerMap.find(mActiveSchemeName);
        if (it != mListenerMap.end()) 
        {
            ListenerList& listenerList = it->second;
            for (ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
            {
                Technique* t = (*i)->handleSchemeNotFound(mActiveSchemeIndex, 
                    mActiveSchemeName, mat, lodIndex, rend);
                if (t)
                    return t;
            }
        }

        //If no success, check generic listeners
        it = mListenerMap.find(BLANKSTRING);
        if (it != mListenerMap.end()) 
        {
            ListenerList& listenerList = it->second;
            for (ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
            {
                Technique* t = (*i)->handleSchemeNotFound(mActiveSchemeIndex, 
                    mActiveSchemeName, mat, lodIndex, rend);
                if (t)
                    return t;
            }
        }
        

        return 0;

    }

	void MaterialManager::_notifyAfterIlluminationPassesCreated(Technique* tech)
	{
		// First, check the scheme specific listeners
		ListenerMap::iterator it = mListenerMap.find(mActiveSchemeName);
		if(it != mListenerMap.end())
		{
			ListenerList& listenerList = it->second;
			for(ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
			{
				bool handled = (*i)->afterIlluminationPassesCreated(tech);
				if(handled)
					return;
			}
		}

		//If no success, check generic listeners
		it = mListenerMap.find(BLANKSTRING);
		if(it != mListenerMap.end())
		{
			ListenerList& listenerList = it->second;
			for(ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
			{
				bool handled = (*i)->afterIlluminationPassesCreated(tech);
				if(handled)
					return;
			}
		}
	}

	void MaterialManager::_notifyBeforeIlluminationPassesCleared(Technique* tech)
	{
		// First, check the scheme specific listeners
		ListenerMap::iterator it = mListenerMap.find(mActiveSchemeName);
		if(it != mListenerMap.end())
		{
			ListenerList& listenerList = it->second;
			for(ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
			{
				bool handled = (*i)->beforeIlluminationPassesCleared(tech);
				if(handled)
					return;
			}
		}

		//If no success, check generic listeners
		it = mListenerMap.find(BLANKSTRING);
		if(it != mListenerMap.end())
		{
			ListenerList& listenerList = it->second;
			for(ListenerList::iterator i = listenerList.begin(); i != listenerList.end(); ++i)
			{
				bool handled = (*i)->beforeIlluminationPassesCleared(tech);
				if(handled)
					return;
			}
		}
	}

}
