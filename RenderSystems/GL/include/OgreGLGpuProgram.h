/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __GLGpuProgram_H__
#define __GLGpuProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre {

    /** Generalised low-level GL program, can be applied to multiple types (eg ARB and NV) */
    class _OgrePrivate GLGpuProgram : public GpuProgram
    {
    public:
        GLGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        virtual ~GLGpuProgram();

        /// Execute the binding functions for this program
        virtual void bindProgram(void) {}
        /// Execute the binding functions for this program
        virtual void unbindProgram(void) {}

        /// Execute the param binding functions for this program
		virtual void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask) {}
		/// Bind just the pass iteration parameters
		virtual void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params) {}


        /// Get the assigned GL program id
        const GLuint getProgramID(void) const
        { return mProgramID; }

		/** Get the attribute index for a given semantic. 
		@remarks
			This can be used to identify the attribute index to bind non-builtin
			attributes like tangent and binormal.
		*/
		virtual GLuint getAttributeIndex(VertexElementSemantic semantic, uint index);
		/** Test whether attribute index for a given semantic is valid. 
		*/
		virtual bool isAttributeValid(VertexElementSemantic semantic, uint index);

		/** Get the fixed attribute bindings normally used by GL for a semantic. */
		static GLuint getFixedAttributeIndex(VertexElementSemantic semantic, uint index);

    protected:
		/** Overridden from GpuProgram, do nothing */
		void loadFromSource(void) {}
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void) {}

        GLuint mProgramID;
        GLenum mProgramType;
    };

    /** Specialisation of the GL low-level program for ARB programs. */
    class _OgrePrivate GLArbGpuProgram : public GLGpuProgram
    {
    public:
        GLArbGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        virtual ~GLArbGpuProgram();

        /// @copydoc GpuProgram::setType
        void setType(GpuProgramType t);


        /// Execute the binding functions for this program
        void bindProgram(void);
        /// Execute the unbinding functions for this program
        void unbindProgram(void);
        /// Execute the param binding functions for this program
		void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask);
		/// Bind just the pass iteration parameters
		void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params);

        /// Get the GL type for the program
        const GLuint getProgramType(void) const
        { return mProgramType; }

    protected:
        void loadFromSource(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);

    };



}; // namespace Ogre

#endif // __GLGpuProgram_H__
