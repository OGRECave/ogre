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

        /// Load the high-level part in a thread-safe way, required for delegate functionality
        void loadHighLevelSafe();

        /// Recurse down structures getting data on parameters
        void recurseParams(CGparameter param, size_t contextArraySize = 1);
        /// Turn a Cg type into a GpuConstantType and number of elements
        void mapTypeAndElementSize(CGtype cgType, bool isRegisterCombiner, GpuConstantDefinition& def) const;

        StringVector mProfiles;
        String mEntryPoint;
        String mSelectedProfile;
        String mProgramString;
        CGprofile mSelectedCgProfile;
        String mCompileArgs;
        // Unfortunately Cg uses char** for arguments - bleh
        // This is a null-terminated list of char* (each null terminated)
        char** mCgArguments;
        
        GpuConstantDefinitionMap mParametersMap;
        size_t mParametersMapSizeAsBuffer;
        map<String,int>::type mSamplerRegisterMap;
        CGenum mInputOp, mOutputOp;
        
        /// Internal method which works out which profile to use for this program
        void selectProfile(void);
        /// Internal method which merges manual and automatic compile arguments
        void buildArgs(void);
        /// Releases memory for the horrible Cg char**
        void freeCgArgs(void);

        void getMicrocodeFromCache(void);
        void compileMicrocode(void);
        void addMicrocodeToCache();

    private:
        HighLevelGpuProgramPtr mDelegate;
        String getHighLevelLanguage() const;
        String getHighLevelTarget() const;
        void fixHighLevelOutput(String& hlSource);


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

        GpuProgramParametersSharedPtr createParameters();
        GpuProgram* _getBindingDelegate();

        bool isSkeletalAnimationIncluded(void) const;
        bool isMorphAnimationIncluded(void) const;
        bool isPoseAnimationIncluded(void) const;
        bool isVertexTextureFetchRequired(void) const;
        GpuProgramParametersSharedPtr getDefaultParameters(void);
        bool hasDefaultParameters(void) const;
        bool getPassSurfaceAndLightStates(void) const;
        bool getPassFogStates(void) const;
        bool getPassTransformStates(void) const;
        bool hasCompileError(void) const;
        void resetCompileError(void);
        size_t getSize(void) const;
        void touch(void);

        /// Scan the file for #include and replace with source from the OGRE resources
        static String resolveCgIncludes(const String& source, Resource* resourceBeingLoaded, const String& fileName);
    };
}

#endif
