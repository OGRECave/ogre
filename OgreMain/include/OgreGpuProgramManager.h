/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __GpuProgramManager_H_
#define __GpuProgramManager_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreException.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

        //TODO Add documentation and explain rationale of this class.

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	class _OgreExport GpuProgramManager : public ResourceManager, public Singleton<GpuProgramManager>
	{
	public:

		typedef set<String>::type SyntaxCodes;
		typedef map<String, GpuSharedParametersPtr>::type SharedParametersMap;

		typedef MemoryDataStreamPtr Microcode;
		typedef map<String, Microcode>::type MicrocodeMap;

	protected:

		SharedParametersMap mSharedParametersMap;
		MicrocodeMap mMicrocodeCache;
		bool mSaveMicrocodesToCache;
		bool mCacheDirty;			// When this is true the cache is 'dirty' and should be resaved to disk.
			
		static String addRenderSystemToName( const String &  name );

        /// Specialised create method with specific parameters
        virtual Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader,
            GpuProgramType gptype, const String& syntaxCode) = 0;
	public:
		GpuProgramManager();
		virtual ~GpuProgramManager();

		/// Get a resource by name
		/// @see GpuProgramManager::getResourceByName
		GpuProgramPtr getByName(const String& name, bool preferHighLevelPrograms = true);

        /** Loads a GPU program from a file of assembly. 
		@remarks
			This method creates a new program of the type specified as the second parameter.
			As with all types of ResourceManager, this class will search for the file in
			all resource locations it has been configured to look in.
		@param name The name of the GpuProgram
		@param groupName The name of the resource group
		@param filename The file to load
		@param gptype The type of program to create
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgramPtr load(const String& name, const String& groupName, 
			const String& filename, GpuProgramType gptype, 
            const String& syntaxCode);

		/** Loads a GPU program from a string of assembly code.
		@remarks
			The assembly code must be compatible with this manager - call the 
			getSupportedSyntax method for details of the supported syntaxes 
		@param name The identifying name to give this program, which can be used to
			retrieve this program later with getByName.
		@param groupName The name of the resource group
		@param code A string of assembly code which will form the program to run
		@param gptype The type of program to create.
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgramPtr loadFromString(const String& name, const String& groupName,
			const String& code, GpuProgramType gptype,
            const String& syntaxCode);

		/** Returns the syntaxes that this manager supports. */
		virtual const SyntaxCodes& getSupportedSyntax(void) const;
		 

        /** Returns whether a given syntax code (e.g. "ps_1_3", "fp20", "arbvp1") is supported. */
        virtual bool isSyntaxSupported(const String& syntaxCode) const;
		
		/** Creates a new GpuProgramParameters instance which can be used to bind
            parameters to your programs.
        @remarks
            Program parameters can be shared between multiple programs if you wish.
        */
        virtual GpuProgramParametersSharedPtr createParameters(void);
        
        /** Create a new, unloaded GpuProgram from a file of assembly. 
        @remarks    
            Use this method in preference to the 'load' methods if you wish to define
            a GpuProgram, but not load it yet; useful for saving memory.
		@par
			This method creates a new program of the type specified as the second parameter.
			As with all types of ResourceManager, this class will search for the file in
			all resource locations it has been configured to look in. 
		@param name The name of the program
		@param groupName The name of the resource group
		@param filename The file to load
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		@param gptype The type of program to create
		*/
		virtual GpuProgramPtr createProgram(const String& name, 
			const String& groupName, const String& filename, 
			GpuProgramType gptype, const String& syntaxCode);

		/** Create a GPU program from a string of assembly code.
        @remarks    
            Use this method in preference to the 'load' methods if you wish to define
            a GpuProgram, but not load it yet; useful for saving memory.
		@par
			The assembly code must be compatible with this manager - call the 
			getSupportedSyntax method for details of the supported syntaxes 
		@param name The identifying name to give this program, which can be used to
			retrieve this program later with getByName.
		@param groupName The name of the resource group
		@param code A string of assembly code which will form the program to run
		@param gptype The type of program to create.
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgramPtr createProgramFromString(const String& name, 
			const String& groupName, const String& code, 
            GpuProgramType gptype, const String& syntaxCode);

        /** General create method, using specific create parameters
            instead of name / value pairs. 
        */
        virtual ResourcePtr create(const String& name, const String& group, 
            GpuProgramType gptype, const String& syntaxCode, bool isManual = false, 
            ManualResourceLoader* loader = 0);

        /** Overrides the standard ResourceManager getResourceByName method.
        @param name The name of the program to retrieve
        @param preferHighLevelPrograms If set to true (the default), high level programs will be
            returned in preference to low-level programs.
        */
        ResourcePtr getResourceByName(const String& name, bool preferHighLevelPrograms = true);


		/** Create a new set of shared parameters, which can be used across many 
			GpuProgramParameters objects of different structures.
		@param name The name to give the shared parameters so you can refer to them
			later.
		*/
		virtual GpuSharedParametersPtr createSharedParameters(const String& name);

		/** Retrieve a set of shared parameters, which can be used across many 
		GpuProgramParameters objects of different structures.
		*/
		virtual GpuSharedParametersPtr getSharedParameters(const String& name) const;

		/** Get (const) access to the available shared parameter sets. 
		*/
		virtual const SharedParametersMap& getAvailableSharedParameters() const;

        /** Get if the microcode of a shader should be saved to a cache
        */
		bool getSaveMicrocodesToCache();
        /** Set if the microcode of a shader should be saved to a cache
        */
		void setSaveMicrocodesToCache( const bool val );

		/** Returns true if the microcodecache changed during the run.
		*/
		bool isCacheDirty(void) const;

		bool canGetCompiledShaderBuffer();
        /** Check if a microcode is available for a program in the microcode cache.
        @param name The name of the program.
        */
		virtual bool isMicrocodeAvailableInCache( const String & name ) const;
        /** Returns a microcode for a program from the microcode cache.
        @param name The name of the program.
        */
		virtual const Microcode & getMicrocodeFromCache( const String & name ) const;

        /** Creates a microcode to be later added to the cache.
		@param size The size of the microcode in bytes
        */
		virtual Microcode createMicrocode( const uint32 size ) const;

        /** Adds a microcode for a program to the microcode cache.
        @param name The name of the program.
        */
		virtual void addMicrocodeToCache( const String & name, const Microcode & microcode );

		/** Removes a microcode for a program from the microcode cache.
        @param name The name of the program.
        */
		virtual void removeMicrocodeFromCache( const String & name );

        /** Saves the microcode cache to disk.
        @param stream The destination stream
        */
		virtual void saveMicrocodeCache( DataStreamPtr stream ) const;
        /** Loads the microcode cache from disk.
        @param stream The source stream
        */
		virtual void loadMicrocodeCache( DataStreamPtr stream );
		


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
        static GpuProgramManager& getSingleton(void);
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
        static GpuProgramManager* getSingletonPtr(void);
    


	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
