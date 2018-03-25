#ifndef __SCENELOADER_H__
#define __SCENELOADER_H__

#include "OgreSceneManager.h"

namespace Ogre {
    
    class _OgreExport SceneLoader
    {
    public:
        virtual ~SceneLoader() {};
        virtual bool load(const String& filename, const String& groupname, SceneNode *rn);
        virtual bool load(const String& filename, const String& groupname, SceneManager *sm);
    };
    
}

#endif
