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
    /// @deprecated migrate to Codec API
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
        void registerSceneLoader(const String& name, const StringVector& ext, SceneLoader *sl);

        void unregisterSceneLoader(const String& name);

        SceneLoader* _getSceneLoader(const String& name) const
        {
            auto it = mSceneLoaders.find(name);
            return it == mSceneLoaders.end() ? NULL : it->second.loader;
        }

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
        
        /// @deprecated migrate to Codec API
        OGRE_DEPRECATED static SceneLoaderManager& getSingleton(void);
        /// @deprecated migrate to Codec API
        OGRE_DEPRECATED static SceneLoaderManager* getSingletonPtr(void);
        
    protected:
        /// Struct for storing data about SceneLoaders internally
        struct SceneLoaderInfo
        {
            SceneLoaderInfo(SceneLoader *l, const StringVector& ext);
            SceneLoader* loader;
            StringVector supportedExt;
        };
        /// Map from scene loader names to SceneLoaderInfo
        typedef std::map<String, SceneLoaderInfo> SceneLoaderMap;
        SceneLoaderMap mSceneLoaders;
    };
    /** @} */
    /** @} */
}

#endif
