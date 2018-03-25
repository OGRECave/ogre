#include "OgreSceneLoaderManager.h"
#include "OgreException.h"

namespace Ogre {
    
    template<> SceneLoaderManager* Singleton<SceneLoaderManager>::msSingleton = 0;
    
    SceneLoaderManager *SceneLoaderManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    
    SceneLoaderManager& SceneLoaderManager::getSingleton(void)
    {
        assert(msSingleton);
        return *msSingleton;
    }
    
    SceneLoaderManager::SceneLoaderManager()
    {
        
    }
    
    SceneLoaderManager::~SceneLoaderManager()
    {
        
    }
    
    void SceneLoaderManager::registerSceneLoader(const String& name, SceneLoader *sl)
    {
        if(mSceneLoaders.find(name) != mSceneLoaders.end())
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                String("SceneLoader with name \"") +
                name + String("\" already exists"),
                "SceneLoaderManager::registerSceneLoader");
        
        mSceneLoaders.insert(std::pair<String, SceneLoader*>(name, sl));
    }
    
    SceneLoader *SceneLoaderManager::getByName(const String& name)
    {
        auto i = mSceneLoaders.find(name);
        SceneLoader *sl;
        
        if(i == mSceneLoaders.end())
            sl = NULL;
        else
            sl = i -> second;
        
        return sl;
    }
    
}
