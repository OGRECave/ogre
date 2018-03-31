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
            @param ext A StringVector containing the supported extensions for the
                SceneLoader being registered.
            @param sl Pointer to the SceneLoader instance.
        */
        void registerSceneLoader(const String& name, StringVectorPtr ext, SceneLoader *sl);
        /** Load a scene from a SceneLoader
        @param filename The name (and path) of the file to be loaded.
            This is also used to determine the SceneLoader to use by the file extension.
        @param groupName The name of a resource group which should be used if any resources
            are created during the parse of this script.
        @param rootNode The root node for the scene being loaded.
        */
        void load(const String& filename, const String& groupName, SceneNode *rootNode);
        /** Load a scene from a SceneLoader
        @param stream Weak reference to a data stream which is the source of the scene.
            This is also used to determine the SceneLoader to use by the file extension.
        @param groupName The name of a resource group which should be used if any resources
            are created during the parse of this script.
        @param rootNode The root node for the scene being loaded.
        */
        void load(DataStreamPtr& stream, const String& groupName, SceneNode *rootNode);
        
        /// @copydoc Singleton::getSingleton()
        static SceneLoaderManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static SceneLoaderManager* getSingletonPtr(void);
        
    protected:
        /// Struct for storing data about SceneLoaders internally
        struct SceneLoaderInfo
        {
            SceneLoaderInfo(SceneLoader *l, StringVectorPtr ext);
            SceneLoader* loader;
            StringVectorPtr supportedExt;
        };
        /// Map from scene loader names to SceneLoaderInfo
        typedef map<String, SceneLoaderInfo>::type SceneLoaderMap;
        SceneLoaderMap mSceneLoaders;
    };
}

#endif
