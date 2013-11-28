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

#ifndef __GL3PlusShader_H__
#define __GL3PlusShader_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre {

    /** Generalized OpenGL shader. Allows for the possibility of
        OpenGL assembly languages and alternatives to GLSL. */
    class _OgreGL3PlusExport GL3PlusShader : public GpuProgram
    {
    public:
        GL3PlusShader(ResourceManager* creator, const String& name, ResourceHandle handle,
                          const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        virtual ~GL3PlusShader();

        /// Execute the binding functions for this shader
        virtual void bindShader(void) {}
        /// Execute the binding functions for this shader
        virtual void unbindShader(void) {}

        /// Execute the param binding functions for this shader
        virtual void bindShaderParameters(GpuProgramParametersSharedPtr params, uint16 mask) {}
        /// Bind just the pass iteration parameters
        virtual void bindShaderPassIterationParameters(GpuProgramParametersSharedPtr params) {}
        /// Execute the shared param binding functions for this shader
        virtual void bindShaderSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask) {}

        /// @copydoc Resource::calculateSize
        virtual size_t calculateSize(void) const;

        //TODO can these be removed?
        /// Get the assigned GL shader ID
        GLuint getShaderID(void) const
        { return mShaderID; }

        /// Get shader type
        GLenum getShaderType(GpuProgramType shaderType);

    protected:
        /** Overridden from GpuProgram, do nothing */
        void loadFromSource(void) {}
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void) {}

        //TODO can these be removed?
        /// 
        GLuint mShaderID;
        /// 
        GLenum mShaderType;
    };

} // namespace Ogre

#endif // __GL3PlusShader_H__
