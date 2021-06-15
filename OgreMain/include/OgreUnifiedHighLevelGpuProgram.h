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
#ifndef __UnifiedHighLevelGpuProgram_H__
#define __UnifiedHighLevelGpuProgram_H__

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** Specialisation of HighLevelGpuProgram which just delegates its implementation
        to one other GpuProgram, allowing a single program definition
        to represent one supported program from a number of options
    @remarks
        Whilst you can use Technique to implement several ways to render an object
        depending on hardware support, if the only reason to need multiple paths is
        because of the shader language supported, this can be
        cumbersome. For example you might want to implement the same shader 
        in HLSL and GLSL for portability but apart from the implementation detail,
        the shaders do the same thing and take the same parameters. If the materials
        in question are complex, duplicating the techniques just to switch language
        is not optimal, so instead you can define high-level programs with a 
        syntax of 'unified', and list the actual implementations in order of
        preference via repeated use of the 'delegate' parameter, which just points
        at another program name. The first one which has a supported syntax 
        will be used.
    */
    class _OgreExport UnifiedHighLevelGpuProgram final : public GpuProgram
    {
        /// Ordered list of potential delegates
        StringVector mDelegateNames;
        /// The chosen delegate
        mutable GpuProgramPtr mChosenDelegate;

        /// Choose the delegate to use
        void chooseDelegate() const;

        void createLowLevelImpl(void);
        void unloadHighLevelImpl(void);
        void loadFromSource(void);

        void unloadImpl() { resetCompileError(); }
    public:
        /** Constructor, should be used only by factory classes. */
        UnifiedHighLevelGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        ~UnifiedHighLevelGpuProgram();

        virtual size_t calculateSize(void) const;

        /** Adds a new delegate program to the list.
        @remarks
            Delegates are tested in order so earlier ones are preferred.
        */
        void addDelegateProgram(const String& name);

        /// Remove all delegate programs
        void clearDelegatePrograms();

        /// Get the chosen delegate
        const GpuProgramPtr& _getDelegate() const;

        /** @copydoc GpuProgram::getLanguage */
        const String& getLanguage(void) const;

        /** Creates a new parameters object compatible with this program definition. 
        @remarks
        Unlike low-level assembly programs, parameters objects are specific to the
        program and therefore must be created from it rather than by the 
        HighLevelGpuProgramManager. This method creates a new instance of a parameters
        object containing the definition of the parameters this program understands.
        */
        GpuProgramParametersSharedPtr createParameters(void);
        /** @copydoc GpuProgram::_getBindingDelegate */
        GpuProgram* _getBindingDelegate(void);

        // All the following methods must delegate to the implementation

        /** @copydoc GpuProgram::isSupported */
        bool isSupported(void) const;

        const String& getSource(void) const override
        {
            return _getDelegate() ? _getDelegate()->getSource() : BLANKSTRING;
        }

        /** @copydoc GpuProgram::isSkeletalAnimationIncluded */
        bool isSkeletalAnimationIncluded(void) const;

        bool isMorphAnimationIncluded(void) const;

        bool isPoseAnimationIncluded(void) const;
        ushort getNumberOfPosesIncluded(void) const;

        bool isVertexTextureFetchRequired(void) const;
        const GpuProgramParametersPtr& getDefaultParameters(void) override;
        bool hasDefaultParameters(void) const;
        bool getPassSurfaceAndLightStates(void) const;
        bool getPassFogStates(void) const;
        bool getPassTransformStates(void) const;
        bool hasCompileError(void) const;
        void resetCompileError(void);

        void load(bool backgroundThread = false);
        void reload(LoadingFlags flags = LF_DEFAULT);
        bool isReloadable(void) const;
        bool isLoaded(void) const;
        bool isLoading() const;
        LoadingState getLoadingState() const;
        void unload(void);
        size_t getSize(void) const;
        void touch(void);
        bool isBackgroundLoaded(void) const;
        void setBackgroundLoaded(bool bl);
        void escalateLoading();
        void addListener(Listener* lis);
        void removeListener(Listener* lis);

    };

    /** Factory class for Unified programs. */
    class _OgreExport UnifiedHighLevelGpuProgramFactory : public GpuProgramFactory
    {
    public:
        UnifiedHighLevelGpuProgramFactory();
        ~UnifiedHighLevelGpuProgramFactory();
        /// Get the name of the language this factory creates programs for
        const String& getLanguage(void) const;
        GpuProgram* create(ResourceManager* creator,
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
