#ifndef __SCENELOADERMANAGER_H__
#define __SCENELOADERMANAGER_H__

#include "OgreSingleton.h"
#include "OgreSceneLoader.h"

namespace Ogre {
    
    class _OgreExport SceneLoaderManager : public Singleton<SceneLoaderManager>
    {
    public:
        SceneLoaderManager();
        virtual ~SceneLoaderManager();
        void registerSceneLoader(const String& name, SceneLoader *sl);
        SceneLoader *getByName(const String& name);
        
        static SceneLoaderManager& getSingleton(void);
        static SceneLoaderManager* getSingletonPtr(void);
        
    protected:
        typedef map<String, SceneLoader*>::type SceneLoaderMap;
        SceneLoaderMap mSceneLoaders;
    };
}

#endif
