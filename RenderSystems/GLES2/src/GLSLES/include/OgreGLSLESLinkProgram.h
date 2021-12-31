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
#ifndef __GLSLESLinkProgram_H__
#define __GLSLESLinkProgram_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGLSLESProgramCommon.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre {
    
    class GLSLESGpuProgram;

    /** C++ encapsulation of GLSL ES Program Object

    */

    class _OgreGLES2Export GLSLESLinkProgram : public GLSLESProgramCommon
    {
    protected:
        /// Compiles and links the vertex and fragment programs
        virtual void compileAndLink(void);

        void buildGLUniformReferences(void);

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        virtual void notifyOnContextLost();
#endif
        
    public:
        /// Constructor should only be used by GLSLESLinkProgramManager
        explicit GLSLESLinkProgram(const GLShaderList& shaders);
        virtual ~GLSLESLinkProgram(void);

        /** Makes a program object active by making sure it is linked and then putting it in use.
        */
        void activate(void);

        /** Updates program object uniforms using data from GpuProgramParameters.
        normally called by GLSLESGpuProgram::bindParameters() just before rendering occurs.
        */
        virtual void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
    };

}

#endif // __GLSLESLinkProgram_H__
