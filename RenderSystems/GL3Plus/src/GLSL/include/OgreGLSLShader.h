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
#ifndef __GLSLShader_H__
#define __GLSLShader_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreGLSLShaderCommon.h"
#include "OgreRenderOperation.h"

namespace Ogre {

    class GLUniformCache;
    class GLSLShader : public GLSLShaderCommon
    {
    public:
        GLSLShader(ResourceManager* creator,
                   const String& name, ResourceHandle handle,
                   const String& group, bool isManual, ManualResourceLoader* loader);
		~GLSLShader();

        void attachToProgramObject(const GLuint programObject) override;
        void detachFromProgramObject(const GLuint programObject) override;

        bool linkSeparable() override;

        void setSamplerBinding(bool enable) { mHasSamplerBinding = enable; }
        bool getSamplerBinding() const { return mHasSamplerBinding; }

        const HardwareBufferPtr& getDefaultBuffer() const { return mDefaultBuffer; }

        /// Overridden from GpuProgram
        const String& getLanguage(void) const override;
    protected:
        void loadFromSource() override;
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void) override;
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() override;
        /// Add boiler plate code and preprocessor extras, then
        /// submit shader source to OpenGL.
        virtual void compileSource();

        /// @param block uniform block to consider. -1 for non-UBO uniforms
        void extractUniforms(int block = -1) const;
        void extractBufferBlocks(GLenum type) const;

        mutable HardwareBufferPtr mDefaultBuffer;
        bool mHasSamplerBinding;
    };

    /** Factory class for GLSL shaders.
     */
    class GLSLShaderFactory : public HighLevelGpuProgramFactory
    {
    public:
        /// Get the name of the language this factory creates shaders for.
        const String& getLanguage(void) const override;
        /// Create an instance of GLSLProgram.
        GpuProgram* create(ResourceManager* creator, const String& name, ResourceHandle handle,
                           const String& group, bool isManual, ManualResourceLoader* loader) override;
    };
}

#endif // __GLSLShader_H__
