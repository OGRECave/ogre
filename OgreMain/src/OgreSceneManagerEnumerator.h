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
#ifndef __SceneManagerEnumerator_H__
#define __SceneManagerEnumerator_H__

#include "OgrePrerequisites.h"

#include "OgreSceneManager.h"
#include "OgreSingleton.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /// Factory for default scene manager
    class DefaultSceneManagerFactory : public SceneManagerFactory
    {
    public:
        DefaultSceneManagerFactory() {}
        ~DefaultSceneManagerFactory() {}
        SceneManager* createInstance(const String& instanceName) override;
        const String& getTypeName(void) const override { return SMT_DEFAULT; }
    };

    /// Default scene manager
    class DefaultSceneManager : public SceneManager
    {
    public:
        DefaultSceneManager(const String& name);
        ~DefaultSceneManager();
        const String& getTypeName(void) const override;
    };

    /** Enumerates the SceneManager classes available to applications.

            As described in the SceneManager class, SceneManagers are responsible
            for organising the scene and issuing rendering commands to the
            RenderSystem. Certain scene types can benefit from different
            rendering approaches, and it is intended that subclasses will
            be created to special case this.
        @par
            In order to give applications easy access to these implementations,
            this class has a number of methods to create or retrieve a SceneManager
            which is appropriate to the scene type. 
        @par
            SceneManagers are created by SceneManagerFactory instances. New factories
            for new types of SceneManager can be registered with this class to make
            them available to clients.
        @par
            Note that you can still plug in your own custom SceneManager without
            using a factory, should you choose, it's just not as flexible that way.
            Just instantiate your own SceneManager manually and use it directly.
    */
    class SceneManagerEnumerator : public Singleton<SceneManagerEnumerator>, public SceneMgtAlloc
    {
    public:
        /// Scene manager instances, indexed by instance name
        typedef std::map<String, SceneManager*> Instances;
    private:
        /// Scene manager factories
        typedef std::map<String, SceneManagerFactory*> Factories;
        Factories mFactories;
        Instances mInstances;
        /// Stored separately to allow iteration
        StringVector mMetaDataList;
        /// Factory for default scene manager
        DefaultSceneManagerFactory mDefaultFactory;
        /// Count of creations for auto-naming
        unsigned long mInstanceCreateCount;
        /// Currently assigned render system
        RenderSystem* mCurrentRenderSystem;


    public:
        SceneManagerEnumerator();
        ~SceneManagerEnumerator();

        void addFactory(SceneManagerFactory* fact);

        void removeFactory(SceneManagerFactory* fact);

        const StringVector& getMetaData() const { return mMetaDataList; }

        SceneManager* createSceneManager(const String& typeName, 
            const String& instanceName = BLANKSTRING);

        void destroySceneManager(SceneManager* sm);

        SceneManager* getSceneManager(const String& instanceName) const;

        bool hasSceneManager(const String& instanceName) const;

        /// Get all the existing SceneManager instances.
        const Instances& getSceneManagers() const;

        /** Notifies all SceneManagers of the destination rendering system.
        */
        void setRenderSystem(RenderSystem* rs);

        /// Utility method to control shutdown of the managers
        void shutdownAll(void);
        /// @copydoc Singleton::getSingleton()
        static SceneManagerEnumerator& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static SceneManagerEnumerator* getSingletonPtr(void);

    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
