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
#ifndef __ParticleSystemManager_H__
#define __ParticleSystemManager_H__


#include "OgrePrerequisites.h"
#include "OgreParticleSystem.h"
#include "OgreFrameListener.h"
#include "OgreSingleton.h"
#include "OgreIteratorWrappers.h"
#include "OgreScriptLoader.h"
#include "OgreResourceGroupManager.h"

namespace Ogre {

	// Forward decl
	class ParticleSystemFactory;
	
    /** Manages particle systems, particle system scripts (templates) and the 
		available emitter & affector factories.
    @remarks
        This singleton class is responsible for creating and managing particle 
		systems. All particle systems must be created and destroyed using this 
		object, although the user interface to creating them is via
		SceneManager. Remember that like all other MovableObject
        subclasses, ParticleSystems do not get rendered until they are 
		attached to a SceneNode object.
    @par
        This class also manages factories for ParticleEmitter and 
		ParticleAffector classes. To enable easy extensions to the types of 
		emitters (particle sources) and affectors (particle modifiers), the
        ParticleSystemManager lets plugins or applications register factory 
		classes which submit new subclasses to ParticleEmitter and 
		ParticleAffector. Ogre comes with a number of them already provided,
        such as cone, sphere and box-shaped emitters, and simple affectors such
	   	as constant directional force and colour faders. However using this 
		registration process, a plugin can create any behaviour required.
    @par
        This class also manages the loading and parsing of particle system 
		scripts, which are text files describing named particle system 
		templates. Instances of particle systems using these templates can
        then be created easily through the createParticleSystem method.
    */
    class _OgreExport ParticleSystemManager: 
		public Singleton<ParticleSystemManager>, public ScriptLoader, public FXAlloc
    {
		friend class ParticleSystemFactory;
	public:
        typedef std::map<String, ParticleSystem*> ParticleTemplateMap;
		typedef std::map<String, ParticleAffectorFactory*> ParticleAffectorFactoryMap;
		typedef std::map<String, ParticleEmitterFactory*> ParticleEmitterFactoryMap;
		typedef std::map<String, ParticleSystemRendererFactory*> ParticleSystemRendererFactoryMap;
    protected:
		OGRE_AUTO_MUTEX
			
        /// Templates based on scripts
        ParticleTemplateMap mSystemTemplates;
        
        /// Factories for named emitter types (can be extended using plugins)
        ParticleEmitterFactoryMap mEmitterFactories;

        /// Factories for named affector types (can be extended using plugins)
        ParticleAffectorFactoryMap mAffectorFactories;

		/// Map of renderer types to factories
		ParticleSystemRendererFactoryMap mRendererFactories;

        StringVector mScriptPatterns;

		// Factory instance
		ParticleSystemFactory* mFactory;

        /** Internal script parsing method. */
        void parseNewEmitter(const String& type, DataStreamPtr& chunk, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseNewAffector(const String& type, DataStreamPtr& chunk, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseAttrib(const String& line, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseEmitterAttrib(const String& line, ParticleEmitter* sys);
        /** Internal script parsing method. */
        void parseAffectorAttrib(const String& line, ParticleAffector* sys);
        /** Internal script parsing method. */
        void skipToNextCloseBrace(DataStreamPtr& chunk);
        /** Internal script parsing method. */
        void skipToNextOpenBrace(DataStreamPtr& chunk);

		/// Internal implementation of createSystem
        ParticleSystem* createSystemImpl(const String& name, size_t quota, 
			const String& resourceGroup);
		/// Internal implementation of createSystem
        ParticleSystem* createSystemImpl(const String& name, const String& templateName);
		/// Internal implementation of destroySystem
        void destroySystemImpl(ParticleSystem* sys);
		
		
    public:

        ParticleSystemManager();
        virtual ~ParticleSystemManager();

        /** Adds a new 'factory' object for emitters to the list of available emitter types.
        @remarks
            This method allows plugins etc to add new particle emitter types to Ogre. Particle emitters
            are sources of particles, and generate new particles with their start positions, colours and
            momentums appropriately. Plugins would create new subclasses of ParticleEmitter which 
            emit particles a certain way, and register a subclass of ParticleEmitterFactory to create them (since multiple 
            emitters can be created for different particle systems).
        @par
            All particle emitter factories have an assigned name which is used to identify the emitter
            type. This must be unique.
        @par
            Note that the object passed to this function will not be destroyed by the ParticleSystemManager,
            since it may have been allocated on a different heap in the case of plugins. The caller must
            destroy the object later on, probably on plugin shutdown.
        @param
            factory Pointer to a ParticleEmitterFactory subclass created by the plugin or application code.
        */
        void addEmitterFactory(ParticleEmitterFactory* factory);

        /** Adds a new 'factory' object for affectors to the list of available affector types.
        @remarks
            This method allows plugins etc to add new particle affector types to Ogre. Particle
            affectors modify the particles in a system a certain way such as affecting their direction
            or changing their colour, lifespan etc. Plugins would
            create new subclasses of ParticleAffector which affect particles a certain way, and register
            a subclass of ParticleAffectorFactory to create them.
        @par
            All particle affector factories have an assigned name which is used to identify the affector
            type. This must be unique.
        @par
            Note that the object passed to this function will not be destroyed by the ParticleSystemManager,
            since it may have been allocated on a different heap in the case of plugins. The caller must
            destroy the object later on, probably on plugin shutdown.
        @param
            factory Pointer to a ParticleAffectorFactory subclass created by the plugin or application code.
        */
        void addAffectorFactory(ParticleAffectorFactory* factory);

		/** Registers a factory class for creating ParticleSystemRenderer instances. 
        @par
            Note that the object passed to this function will not be destroyed by the ParticleSystemManager,
            since it may have been allocated on a different heap in the case of plugins. The caller must
            destroy the object later on, probably on plugin shutdown.
        @param
            factory Pointer to a ParticleSystemRendererFactory subclass created by the plugin or application code.
		*/
		void addRendererFactory(ParticleSystemRendererFactory* factory);

        /** Adds a new particle system template to the list of available templates. 
        @remarks
            Instances of particle systems in a scene are not normally unique - often you want to place the
            same effect in many places. This method allows you to register a ParticleSystem as a named template,
            which can subsequently be used to create instances using the createSystem method.
        @par
            Note that particle system templates can either be created programmatically by an application 
            and registered using this method, or they can be defined in a script file (*.particle) which is
            loaded by the engine at startup, very much like Material scripts.
        @param
            name The name of the template. Must be unique across all templates.
        @param
            sysTemplate A pointer to a particle system to be used as a template. The manager
                will take over ownership of this pointer.
            
        */
        void addTemplate(const String& name, ParticleSystem* sysTemplate);

        /** Removes a specified template from the ParticleSystemManager.
        @remarks
            This method removes a given template from the particle system manager, optionally deleting
            the template if the deleteTemplate method is called.  Throws an exception if the template
            could not be found.
        @param
            name The name of the template to remove.
        @param
            deleteTemplate Whether or not to delete the template before removing it.
        */
        void removeTemplate(const String& name, bool deleteTemplate = true);

        /** Removes a specified template from the ParticleSystemManager.
        @remarks
            This method removes all templates from the ParticleSystemManager.
        @param
            deleteTemplate Whether or not to delete the templates before removing them.
        */
        void removeAllTemplates(bool deleteTemplate = true);

        /** Create a new particle system template. 
        @remarks
            This method is similar to the addTemplate method, except this just creates a new template
            and returns a pointer to it to be populated. Use this when you don't already have a system
            to add as a template and just want to create a new template which you will build up in-place.
        @param
            name The name of the template. Must be unique across all templates.
        @param
            resourceGroup The name of the resource group which will be used to 
                load any dependent resources.
            
        */
        ParticleSystem* createTemplate(const String& name, const String& resourceGroup);

        /** Retrieves a particle system template for possible modification. 
        @remarks
            Modifying a template does not affect the settings on any ParticleSystems already created
            from this template.
        */
        ParticleSystem* getTemplate(const String& name);

        /** Internal method for creating a new emitter from a factory.
        @remarks
            Used internally by the engine to create new ParticleEmitter instances from named
            factories. Applications should use the ParticleSystem::addEmitter method instead, 
            which calls this method to create an instance.
        @param
            emitterType String name of the emitter type to be created. A factory of this type must have been registered.
        @param 
            psys The particle system this is being created for
        */
        ParticleEmitter* _createEmitter(const String& emitterType, ParticleSystem* psys);

        /** Internal method for destroying an emitter.
        @remarks
            Because emitters are created by factories which may allocate memory from separate heaps,
            the memory allocated must be freed from the same place. This method is used to ask the factory
            to destroy the instance passed in as a pointer.
        @param
            emitter Pointer to emitter to be destroyed. On return this pointer will point to invalid (freed) memory.
        */
        void _destroyEmitter(ParticleEmitter* emitter);

        /** Internal method for creating a new affector from a factory.
        @remarks
            Used internally by the engine to create new ParticleAffector instances from named
            factories. Applications should use the ParticleSystem::addAffector method instead, 
            which calls this method to create an instance.
        @param
            effectorType String name of the affector type to be created. A factory of this type must have been registered.
        @param
            psys The particle system it is being created for
        */
        ParticleAffector* _createAffector(const String& affectorType, ParticleSystem* psys);

        /** Internal method for destroying an affector.
        @remarks
            Because affectors are created by factories which may allocate memory from separate heaps,
            the memory allocated must be freed from the same place. This method is used to ask the factory
            to destroy the instance passed in as a pointer.
        @param
            affector Pointer to affector to be destroyed. On return this pointer will point to invalid (freed) memory.
        */
        void _destroyAffector(ParticleAffector* affector);

        /** Internal method for creating a new renderer from a factory.
        @remarks
            Used internally by the engine to create new ParticleSystemRenderer instances from named
            factories. Applications should use the ParticleSystem::setRenderer method instead, 
            which calls this method to create an instance.
        @param
            rendererType String name of the renderer type to be created. A factory of this type must have been registered.
        */
        ParticleSystemRenderer* _createRenderer(const String& rendererType);

        /** Internal method for destroying a renderer.
        @remarks
            Because renderer are created by factories which may allocate memory from separate heaps,
            the memory allocated must be freed from the same place. This method is used to ask the factory
            to destroy the instance passed in as a pointer.
        @param
            renderer Pointer to renderer to be destroyed. On return this pointer will point to invalid (freed) memory.
        */
        void _destroyRenderer(ParticleSystemRenderer* renderer);

        /** Init method to be called by OGRE system.
        @remarks
            Due to dependencies between various objects certain initialisation tasks cannot be done
            on construction. OGRE will call this method when the rendering subsystem is initialised.
        */
        void _initialise(void);

        /// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const;
        /// @copydoc ScriptLoader::parseScript
        void parseScript(DataStreamPtr& stream, const String& groupName);
        /// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const;

		typedef MapIterator<ParticleAffectorFactoryMap> ParticleAffectorFactoryIterator;
		typedef MapIterator<ParticleEmitterFactoryMap> ParticleEmitterFactoryIterator;
		typedef MapIterator<ParticleSystemRendererFactoryMap> ParticleRendererFactoryIterator;
		/** Return an iterator over the affector factories currently registered */
		ParticleAffectorFactoryIterator getAffectorFactoryIterator(void);
		/** Return an iterator over the emitter factories currently registered */
		ParticleEmitterFactoryIterator getEmitterFactoryIterator(void);
		/** Return an iterator over the renderer factories currently registered */
		ParticleRendererFactoryIterator getRendererFactoryIterator(void);


        typedef MapIterator<ParticleTemplateMap> ParticleSystemTemplateIterator;
        /** Gets an iterator over the list of particle system templates. */
        ParticleSystemTemplateIterator getTemplateIterator(void)
        {
            return ParticleSystemTemplateIterator(
                mSystemTemplates.begin(), mSystemTemplates.end());
        } 

        /** Get an instance of ParticleSystemFactory (internal use). */
		ParticleSystemFactory* _getFactory(void) { return mFactory; }
		
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
        static ParticleSystemManager& getSingleton(void);
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
        static ParticleSystemManager* getSingletonPtr(void);

    };

	/** Factory object for creating ParticleSystem instances */
	class _OgreExport ParticleSystemFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
	public:
		ParticleSystemFactory() {}
		~ParticleSystemFactory() {}
		
		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};
	
}

#endif

