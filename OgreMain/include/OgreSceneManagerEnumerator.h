/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __SceneManagerEnumerator_H__
#define __SceneManagerEnumerator_H__

#include "OgrePrerequisites.h"

#include "OgreSceneManager.h"
#include "OgreSingleton.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {
    
	/// Factory for default scene manager
	class _OgreExport DefaultSceneManagerFactory : public SceneManagerFactory
	{
	protected:
		void initMetaData(void) const;
	public:
		DefaultSceneManagerFactory() {}
		~DefaultSceneManagerFactory() {}
		/// Factory type name
		static const String FACTORY_TYPE_NAME;
		SceneManager* createInstance(const String& instanceName);
		void destroyInstance(SceneManager* instance);
	};
	/// Default scene manager
	class _OgreExport DefaultSceneManager : public SceneManager
	{
	public:
		DefaultSceneManager(const String& name);
		~DefaultSceneManager();
		const String& getTypeName(void) const;
	};

    /** Enumerates the SceneManager classes available to applications.
        @remarks
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
    class _OgreExport SceneManagerEnumerator : public Singleton<SceneManagerEnumerator>, public SceneMgtAlloc
    {
	public:
		/// Scene manager instances, indexed by instance name
		typedef std::map<String, SceneManager*> Instances;
		/// List of available scene manager types as meta data
		typedef std::vector<const SceneManagerMetaData*> MetaDataList;
    private:
		/// Scene manager factories
		typedef std::list<SceneManagerFactory*> Factories;
		Factories mFactories;
		Instances mInstances;
		/// Stored separately to allow iteration
		MetaDataList mMetaDataList;
		/// Factory for default scene manager
		DefaultSceneManagerFactory mDefaultFactory;
		/// Count of creations for auto-naming
		unsigned long mInstanceCreateCount;
		/// Currently assigned render system
		RenderSystem* mCurrentRenderSystem;


    public:
        SceneManagerEnumerator();
        ~SceneManagerEnumerator();

		/** Register a new SceneManagerFactory. 
		@remarks
			Plugins should call this to register as new SceneManager providers.
		*/
		void addFactory(SceneManagerFactory* fact);

		/** Remove a SceneManagerFactory. 
		*/
		void removeFactory(SceneManagerFactory* fact);

		/** Get more information about a given type of SceneManager.
		@remarks
			The metadata returned tells you a few things about a given type 
			of SceneManager, which can be created using a factory that has been
			registered already. 
		@param typeName The type name of the SceneManager you want to enquire on.
			If you don't know the typeName already, you can iterate over the 
			metadata for all types using getMetaDataIterator.
		*/
		const SceneManagerMetaData* getMetaData(const String& typeName) const;

		typedef ConstVectorIterator<MetaDataList> MetaDataIterator;
		/** Iterate over all types of SceneManager available for construction, 
			providing some information about each one.
		*/
		MetaDataIterator getMetaDataIterator(void) const;

		/** Create a SceneManager instance of a given type.
		@remarks
			You can use this method to create a SceneManager instance of a 
			given specific type. You may know this type already, or you may
			have discovered it by looking at the results from getMetaDataIterator.
		@note
			This method throws an exception if the named type is not found.
		@param typeName String identifying a unique SceneManager type
		@param instanceName Optional name to given the new instance that is
			created. If you leave this blank, an auto name will be assigned.
		*/
		SceneManager* createSceneManager(const String& typeName, 
			const String& instanceName = StringUtil::BLANK);

		/** Create a SceneManager instance based on scene type support.
		@remarks
			Creates an instance of a SceneManager which supports the scene types
			identified in the parameter. If more than one type of SceneManager 
			has been registered as handling that combination of scene types, 
			in instance of the last one registered is returned.
		@note This method always succeeds, if a specific scene manager is not
			found, the default implementation is always returned.
		@param typeMask A mask containing one or more SceneType flags
		@param instanceName Optional name to given the new instance that is
			created. If you leave this blank, an auto name will be assigned.
		*/
		SceneManager* createSceneManager(SceneTypeMask typeMask, 
			const String& instanceName = StringUtil::BLANK);

		/** Destroy an instance of a SceneManager. */
		void destroySceneManager(SceneManager* sm);

		/** Get an existing SceneManager instance that has already been created,
			identified by the instance name.
		@param instanceName The name of the instance to retrieve.
		*/
		SceneManager* getSceneManager(const String& instanceName) const;

		typedef MapIterator<Instances> SceneManagerIterator;
		/** Get an iterator over all the existing SceneManager instances. */
		SceneManagerIterator getSceneManagerIterator(void);

        /** Notifies all SceneManagers of the destination rendering system.
        */
        void setRenderSystem(RenderSystem* rs);

        /// Utility method to control shutdown of the managers
        void shutdownAll(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static SceneManagerEnumerator& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static SceneManagerEnumerator* getSingletonPtr(void);

    };


}

#endif
