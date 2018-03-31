#ifndef __SCENELOADER_H__
#define __SCENELOADER_H__

#include "OgreSceneManager.h"
#include "OgreDataStream.h"

namespace Ogre {
    
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** Abstract class for loading scenes from a file (DataStream).
    */
    class _OgreExport SceneLoader
    {
    public:
        virtual ~SceneLoader() {};
        /** Load a scene from a file
        @param stream Weak reference to a data stream which is the source of the scene
        @param groupName The name of a resource group which should be used if any resources
            are created during the parse of this script.
        @param rootNode The root node for the scene being created
        */
        virtual void load(DataStreamPtr& stream, const String& groupName, SceneNode *rootNode) = 0;
    };
    /** @} */
    /** @} */
}

#endif
