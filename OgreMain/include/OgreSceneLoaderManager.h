#ifndef __SCENELOADERMANAGER_H__
#define __SCENELOADERMANAGER_H__

#include "OgreSingleton.h"
#include "OgreSceneLoader.h"

namespace Ogre {
    
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** This class defines an interface for registering and using SceneLoaders.
    */
    class _OgreExport SceneLoaderManager : public Singleton<SceneLoaderManager>
    {
    public:
        SceneLoaderManager();
        virtual ~SceneLoaderManager();
        /** Register a new SceneLoader
            @param name The name for the SceneLoader for lookup.
            @param sl Pointer to the SceneLoader instance.
        */
        void registerSceneLoader(const String& name, SceneLoader *sl);
        /** Load a scene from a SceneLoader
        @remarks This is usually equivalent to calling
            SceneLoader::load(stream, groupName, rootNode)
        @param loaderName The name of the SceneLoader that will be used for loading.
        @param stream Weak reference to a data stream which is the source of the scene
        @param groupName The name of a resource group which should be used if any resources
            are created during the parse of this script.
        @param rootNode The root node for the scene being created
        */
        void load(const String& loaderName, DataStreamPtr& stream, const String& groupName, SceneNode *rootNode);
        
        /// @copydoc Singleton::getSingleton()
        static SceneLoaderManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static SceneLoaderManager* getSingletonPtr(void);
        
    protected:
        /// Map from scene loader names to SceneLoaders
        typedef map<String, SceneLoader*>::type SceneLoaderMap;
        SceneLoaderMap mSceneLoaders;
    };
}

#endif
