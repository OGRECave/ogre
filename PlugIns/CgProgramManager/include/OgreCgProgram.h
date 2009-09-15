/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __CgProgram_H__
#define __CgProgram_H__

#include "OgreCgPrerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreStringVector.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for nVidia's CG language.
    @remarks
        Cg can be used to compile common, high-level, C-like code down to assembler
        language for both GL and Direct3D, for multiple graphics cards. You must
        supply a list of profiles which your program must support using
        setProfiles() before the program is loaded in order for this to work. The
        program will then negotiate with the renderer to compile the appropriate program
        for the API and graphics card capabilities.
    */
    class CgProgram : public HighLevelGpuProgram
    {
    public:
        /// Command object for setting entry point
        class CmdEntryPoint : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting profiles
        class CmdProfiles : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting compilation arguments
        class CmdArgs : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

    protected:

        static CmdEntryPoint msCmdEntryPoint;
        static CmdProfiles msCmdProfiles;
        static CmdArgs msCmdArgs;

        /// The CG context to use, passed in by factory
        CGcontext mCgContext;
        /// Program handle
        CGprogram mCgProgram;
        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() const;

		/// Recurse down structures getting data on parameters
		void recurseParams(CGparameter param, size_t contextArraySize = 1) const;
		/// Turn a Cg type into a GpuConstantType and number of elements
		void mapTypeAndElementSize(CGtype cgType, bool isRegisterCombiner, GpuConstantDefinition& def) const;

        StringVector mProfiles;
        String mEntryPoint;
        String mSelectedProfile;
        CGprofile mSelectedCgProfile;
        String mCompileArgs;
        // Unfortunately Cg uses char** for arguments - bleh
        // This is a null-terminated list of char* (each null terminated)
        char** mCgArguments;

        /// Internal method which works out which profile to use for this program
        void selectProfile(void);
        /// Internal method which merges manual and automatic compile arguments
        void buildArgs(void);
        /// Releases memory for the horrible Cg char**
        void freeCgArgs(void);


    public:
        CgProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, 
            CGcontext context);
        ~CgProgram();

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) const { return mEntryPoint; }
        /** Sets the Cg profiles which can be supported by the program. */
        void setProfiles(const StringVector& profiles);
        /** Gets the Cg profiles which can be supported by the program. */
        const StringVector& getProfiles(void) const { return mProfiles; }
        /** Sets the compilation arguments for this program ie the first method called. */
        void setCompileArguments(const String& args) { mCompileArgs = args; }
        /** Gets the entry point defined for this program. */
        const String& getCompileArguments(void) const { return mCompileArgs; }
        /// Overridden from GpuProgram
        bool isSupported(void) const;
        /// Overridden from GpuProgram
        const String& getLanguage(void) const;

		/// scan the file for #include and replace with source from the OGRE resources
		static String resolveCgIncludes(const String& source, Resource* resourceBeingLoaded, const String& fileName);

    };
}

#endif
