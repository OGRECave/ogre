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
#include "OgreSceneManagerEnumerator.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> SceneManagerEnumerator* Singleton<SceneManagerEnumerator>::msSingleton = 0;
    SceneManagerEnumerator* SceneManagerEnumerator::getSingletonPtr(void)
    {
        return msSingleton;
    }
    SceneManagerEnumerator& SceneManagerEnumerator::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    //-----------------------------------------------------------------------
    SceneManagerEnumerator::SceneManagerEnumerator()
        : mInstanceCreateCount(0), mCurrentRenderSystem(0)
    {
        addFactory(&mDefaultFactory);

    }
    //-----------------------------------------------------------------------
    SceneManagerEnumerator::~SceneManagerEnumerator()
    {
        // Destroy all remaining instances
        // Really should have shutdown and unregistered by now, but catch here in case
        Instances instancesCopy = mInstances;
        for (auto& i : instancesCopy)
        {
            // destroy instances
            for(auto& fp : mFactories)
            {
                if (fp.first == i.second->getTypeName())
                {
                    fp.second->destroyInstance(i.second);
                    mInstances.erase(i.first);
                    break;
                }
            }
        }
        mInstances.clear();
    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::addFactory(SceneManagerFactory* fact)
    {
        mFactories[fact->getTypeName()] = fact;
        // add to metadata
        mMetaDataList.push_back(fact->getTypeName());
        // Log
        LogManager::getSingleton().logMessage("SceneManagerFactory for type '" + fact->getTypeName() + "' registered.");
    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::removeFactory(SceneManagerFactory* fact)
    {
        OgreAssert(fact, "Cannot remove a null SceneManagerFactory");

        // destroy all instances for this factory
        for (Instances::iterator i = mInstances.begin(); i != mInstances.end(); )
        {
            SceneManager* instance = i->second;
            if (instance->getTypeName() == fact->getTypeName())
            {
                fact->destroyInstance(instance);
                Instances::iterator deli = i++;
                mInstances.erase(deli);
            }
            else
            {
                ++i;
            }
        }
        // remove from metadata
        auto it = std::remove(mMetaDataList.begin(), mMetaDataList.end(), fact->getTypeName());
        mMetaDataList.erase(it, mMetaDataList.end());
        mFactories.erase(fact->getTypeName());
    }
    //-----------------------------------------------------------------------
    SceneManager* SceneManagerEnumerator::createSceneManager(
        const String& typeName, const String& instanceName)
    {
        if (mInstances.find(instanceName) != mInstances.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "SceneManager instance called '" + instanceName + "' already exists",
                "SceneManagerEnumerator::createSceneManager");
        }

        auto it = mFactories.find(typeName);

        if (it == mFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No factory found for scene manager of type '" + typeName + "'");
        }

        SceneManager* inst = 0;
        if (instanceName.empty())
        {
            // generate a name
            StringStream s;
            s << "SceneManagerInstance" << ++mInstanceCreateCount;
            inst = it->second->createInstance(s.str());
        }
        else
        {
            inst = it->second->createInstance(instanceName);
        }

        /// assign rs if already configured
        if (mCurrentRenderSystem)
            inst->_setDestinationRenderSystem(mCurrentRenderSystem);

        mInstances[inst->getName()] = inst;
        
        return inst;
        

    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::destroySceneManager(SceneManager* sm)
    {
        OgreAssert(sm, "Cannot destroy a null SceneManager");

        // Erase instance from map
        mInstances.erase(sm->getName());

        // Find factory to destroy
        for(auto& fp : mFactories)
        {
            if (fp.first== sm->getTypeName())
            {
                fp.second->destroyInstance(sm);
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    SceneManager* SceneManagerEnumerator::getSceneManager(const String& instanceName) const
    {
        Instances::const_iterator i = mInstances.find(instanceName);
        if(i != mInstances.end())
        {
            return i->second;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "SceneManager instance with name '" + instanceName + "' not found.",
                "SceneManagerEnumerator::getSceneManager");
        }

    }
    //---------------------------------------------------------------------
    bool SceneManagerEnumerator::hasSceneManager(const String& instanceName) const
    {
        return mInstances.find(instanceName) != mInstances.end();
    }
    //-----------------------------------------------------------------------
    const SceneManagerEnumerator::Instances& SceneManagerEnumerator::getSceneManagers(void) const
    {
        return mInstances;
    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::setRenderSystem(RenderSystem* rs)
    {
        mCurrentRenderSystem = rs;

        for (auto& i : mInstances)
        {
            i.second->_setDestinationRenderSystem(rs);
        }
    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::shutdownAll(void)
    {
        for (auto& i : mInstances)
        {
            // shutdown instances (clear scene)
            i.second->clearScene();
        }
    }
    //-----------------------------------------------------------------------
    const String SMT_DEFAULT = "DefaultSceneManager";
    //-----------------------------------------------------------------------
    SceneManager* DefaultSceneManagerFactory::createInstance(
        const String& instanceName)
    {
        return OGRE_NEW DefaultSceneManager(instanceName);
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    DefaultSceneManager::DefaultSceneManager(const String& name)
        : SceneManager(name)
    {
    }
    //-----------------------------------------------------------------------
    DefaultSceneManager::~DefaultSceneManager()
    {
    }
    //-----------------------------------------------------------------------
    const String& DefaultSceneManager::getTypeName(void) const
    {
        return SMT_DEFAULT;
    }
    //-----------------------------------------------------------------------
}
