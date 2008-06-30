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
#ifndef __HighLevelGpuProgramManager_H__
#define __HighLevelGpuProgramManager_H__

#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreSingleton.h"
#include "OgreException.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {

	/** Interface definition for factories of HighLevelGpuProgram. */
	class _OgreExport HighLevelGpuProgramFactory : public FactoryAlloc
	{
	public:
        HighLevelGpuProgramFactory() {}
        virtual ~HighLevelGpuProgramFactory();
		/// Get the name of the language this factory creates programs for
		virtual const String& getLanguage(void) const = 0;
        virtual HighLevelGpuProgram* create(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader) = 0;
		virtual void destroy(HighLevelGpuProgram* prog) = 0;
	};
	/** This ResourceManager manages high-level vertex and fragment programs. 
	@remarks
		High-level vertex and fragment programs can be used instead of assembler programs
		as managed by GpuProgramManager; however they typically result in a GpuProgram
		being created as a derivative of the high-level program. High-level programs are
		easier to write, and can often be API-independent, unlike assembler programs. 
	@par
		This class not only manages the programs themselves, it also manages the factory
		classes which allow the creation of high-level programs using a variety of high-level
		syntaxes. Plugins can be created which register themselves as high-level program
		factories and as such the engine can be extended to accept virtually any kind of
		program provided a plugin is written.
	*/
	class _OgreExport HighLevelGpuProgramManager 
		: public ResourceManager, public Singleton<HighLevelGpuProgramManager>
	{
	public:
		typedef std::map<String, HighLevelGpuProgramFactory*> FactoryMap;
	protected:
		/// Factories capable of creating HighLevelGpuProgram instances
		FactoryMap mFactories;

		/// Factory for dealing with programs for languages we can't create
		HighLevelGpuProgramFactory* mNullFactory;
		/// Factory for unified high-level programs
		HighLevelGpuProgramFactory* mUnifiedFactory;

		HighLevelGpuProgramFactory* getFactory(const String& language);

        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* params);
	public:
		HighLevelGpuProgramManager();
		~HighLevelGpuProgramManager();
		/** Add a new factory object for high-level programs of a given language. */
		void addFactory(HighLevelGpuProgramFactory* factory);
		/** Remove a factory object for high-level programs of a given language. */
		void removeFactory(HighLevelGpuProgramFactory* factory);


        /** Create a new, unloaded HighLevelGpuProgram. 
		@par
			This method creates a new program of the type specified as the second and third parameters.
			You will have to call further methods on the returned program in order to 
			define the program fully before you can load it.
		@param name The identifying name of the program
        @param groupName The name of the resource group which this program is
            to be a member of
		@param language Code of the language to use (e.g. "cg")
		@param gptype The type of program to create
		*/
		virtual HighLevelGpuProgramPtr createProgram(
			const String& name, const String& groupName, 
            const String& language, GpuProgramType gptype);

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
        static HighLevelGpuProgramManager& getSingleton(void);
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
        static HighLevelGpuProgramManager* getSingletonPtr(void);


	};

}

#endif
