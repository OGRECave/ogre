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
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup CgProgramManager
    *  @{
    */
    /** Specialisation of HighLevelGpuProgram to provide support for nVidia's CG language.

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
        /// Command object for setting profiles
        class CmdProfiles : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for setting compilation arguments
        class CmdArgs : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

    protected:

        static CmdProfiles msCmdProfiles;
        static CmdArgs msCmdArgs;

        /// The CG context to use, passed in by factory
        CGcontext mCgContext;
        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void) override;
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        void createLowLevelImpl(void) override;
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void) override;
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() override;

        /// Load the high-level part in a thread-safe way, required for delegate functionality
        void loadHighLevelSafe();

        /// Recurse down structures getting data on parameters
        void recurseParams(CGparameter param, size_t contextArraySize = 1);
        /// Turn a Cg type into a GpuConstantType and number of elements
        void mapTypeAndElementSize(CGtype cgType, bool isRegisterCombiner, GpuConstantDefinition& def) const;

        StringVector mProfiles;
        String mSelectedProfile;
        String mProgramString;
        CGprofile mSelectedCgProfile;
        // Unfortunately Cg uses char** for arguments - bleh
        // This is a null-terminated list of char* (each null terminated)
        char** mCgArguments;
        
        GpuConstantDefinitionMap mParametersMap;
        size_t mParametersMapSizeAsBuffer;
        std::map<String,int> mSamplerRegisterMap;
        CGenum mInputOp, mOutputOp;
        
        /// Internal method which works out which profile to use for this program
        void selectProfile(void);
        /// Internal method which merges manual and automatic compile arguments
        void buildArgs(void);
        /// Releases memory for the horrible Cg char**
        void freeCgArgs(void);

        void getMicrocodeFromCache(uint32 id);
        void compileMicrocode(void);
        void addMicrocodeToCache(uint32 id);

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
        /// Overridden from GpuProgram
        bool isSupported(void) const override;
        /// Overridden from GpuProgram
        const String& getLanguage(void) const override;

        GpuProgramParametersSharedPtr createParameters() override;
        GpuProgram* _getBindingDelegate() override;

        bool isSkeletalAnimationIncluded(void) const override;
        bool isMorphAnimationIncluded(void) const override;
        bool isPoseAnimationIncluded(void) const override;
        bool isVertexTextureFetchRequired(void) const override;
        const GpuProgramParametersPtr& getDefaultParameters(void) override;
        bool hasDefaultParameters(void) const override;
        bool getPassSurfaceAndLightStates(void) const override;
        bool getPassFogStates(void) const override;
        bool getPassTransformStates(void) const override;
        bool hasCompileError(void) const override;
        void resetCompileError(void) override;
        size_t getSize(void) const;
        void touch(void) override;
    };
    /** @} */
    /** @} */
}

#endif
