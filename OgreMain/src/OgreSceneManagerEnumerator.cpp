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
            for(auto *f : mFactories)
            {
                if (f->getMetaData().typeName == i.second->getTypeName())
                {
                    f->destroyInstance(i.second);
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
        mFactories.push_back(fact);
        // add to metadata
        mMetaDataList.push_back(&fact->getMetaData());
        // Log
        LogManager::getSingleton().logMessage("SceneManagerFactory for type '" +
            fact->getMetaData().typeName + "' registered.");
    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::removeFactory(SceneManagerFactory* fact)
    {
        OgreAssert(fact, "Cannot remove a null SceneManagerFactory");

        // destroy all instances for this factory
        for (Instances::iterator i = mInstances.begin(); i != mInstances.end(); )
        {
            SceneManager* instance = i->second;
            if (instance->getTypeName() == fact->getMetaData().typeName)
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
        for (MetaDataList::iterator m = mMetaDataList.begin(); m != mMetaDataList.end(); ++m)
        {
            if(*m == &(fact->getMetaData()))
            {
                mMetaDataList.erase(m);
                break;
            }
        }
        mFactories.remove(fact);
    }
    //-----------------------------------------------------------------------
    const SceneManagerMetaData* SceneManagerEnumerator::getMetaData(const String& typeName) const
    {
        for (auto *d : mMetaDataList)
        {
            if (typeName == d->typeName)
            {
                return d;
            }
        }

        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "No metadata found for scene manager of type '" + typeName + "'",
            "SceneManagerEnumerator::createSceneManager");
    }
    //-----------------------------------------------------------------------
    SceneManagerEnumerator::MetaDataIterator 
    SceneManagerEnumerator::getMetaDataIterator(void) const
    {
        return MetaDataIterator(mMetaDataList.begin(), mMetaDataList.end());
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

        SceneManager* inst = 0;
        for(auto *f : mFactories)
        {
            if (f->getMetaData().typeName == typeName)
            {
                if (instanceName.empty())
                {
                    // generate a name
                    StringStream s;
                    s << "SceneManagerInstance" << ++mInstanceCreateCount;
                    inst = f->createInstance(s.str());
                }
                else
                {
                    inst = f->createInstance(instanceName);
                }
                break;
            }
        }

        if (!inst)
        {
            // Error!
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "No factory found for scene manager of type '" + typeName + "'",
                "SceneManagerEnumerator::createSceneManager");
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
        for(auto *f : mFactories)
        {
            if (f->getMetaData().typeName == sm->getTypeName())
            {
                f->destroyInstance(sm);
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
    SceneManagerEnumerator::SceneManagerIterator 
    SceneManagerEnumerator::getSceneManagerIterator(void)
    {
        return SceneManagerIterator(mInstances.begin(), mInstances.end());

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
    const String DefaultSceneManagerFactory::FACTORY_TYPE_NAME = "DefaultSceneManager";
    //-----------------------------------------------------------------------
    void DefaultSceneManagerFactory::initMetaData(void) const
    {
        mMetaData.typeName = FACTORY_TYPE_NAME;
        mMetaData.worldGeometrySupported = false;
    }
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
        return DefaultSceneManagerFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
}
