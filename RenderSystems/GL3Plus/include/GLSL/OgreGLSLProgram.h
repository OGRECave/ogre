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
#ifndef __GLSLProgram_H__
#define __GLSLProgram_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGL3PlusHardwareShaderStorageBuffer.h"
#include "OgreGL3PlusHardwareCounterBuffer.h"
#include "OgreGLSLProgramCommon.h"
#include "OgreGLSLShader.h"

namespace Ogre {
    /** Structure used to keep track of named atomic counter uniforms
        in the linked program object.  Same as GLUniformReference, but
        contains an additional offset parameter which currently only
        atomic counters feature.
    */
    struct GLAtomicCounterReference
    {
        /// GL binding handle (similar to location)
        GLint mBinding;
        /// GL offset (only used for atomic counters)
        GLint mOffset;
        /// Which type of program params will this value come from?
        GpuProgramType mSourceProgType;
        /// The constant definition it relates to
        const GpuConstantDefinition* mConstantDef;
    };

    typedef std::vector<GLAtomicCounterReference> GLAtomicCounterReferenceList;
    typedef GLAtomicCounterReferenceList::iterator GLAtomicCounterReferenceIterator;
    typedef std::map<GpuSharedParametersPtr, HardwareUniformBufferSharedPtr> SharedParamsBufferMap;
    typedef std::vector<HardwareCounterBufferSharedPtr> GLCounterBufferList;
    typedef GLCounterBufferList::iterator GLCounterBufferIterator;

    /** C++ encapsulation of GLSL program object.
     */
    class _OgreGL3PlusExport GLSLProgram : public GLSLProgramCommon
    {
    public:
        void bindFixedAttributes(GLuint program);

        GLSLShader* getVertexShader() const { return static_cast<GLSLShader*>(mVertexShader); }
        GLSLShader* getHullShader() const { return mHullShader; }
        GLSLShader* getDomainShader() const { return mDomainShader; }
        GLSLShader* getGeometryShader() const { return mGeometryShader; }
        GLSLShader* getFragmentShader() const { return mFragmentShader; }
        GLSLShader* getComputeShader() const { return mComputeShader; }

        bool isUsingShader(GLSLShaderCommon* shader) const
        {
            return mVertexShader == shader || (GLSLShaderCommon*)mGeometryShader == shader ||
                   (GLSLShaderCommon*)mFragmentShader == shader ||
                   (GLSLShaderCommon*)mHullShader == shader ||
                   (GLSLShaderCommon*)mDomainShader == shader ||
                   (GLSLShaderCommon*)mComputeShader == shader;
        }

        virtual void updateAtomicCounters(GpuProgramParametersSharedPtr params, uint16 mask,
                                          GpuProgramType fromProgType) = 0;

        void setTransformFeedbackVaryings(const std::vector<String>& nameStrings);
    protected:
        /// Constructor should only be used by GLSLMonolithicProgramManager and GLSLSeparableProgramManager
        GLSLProgram(GLSLShader* vertexProgram,
                    GLSLShader* hullProgram,
                    GLSLShader* domainProgram,
                    GLSLShader* geometryProgram,
                    GLSLShader* fragmentProgram,
                    GLSLShader* computeProgram);

        /// Container of atomic counter uniform references that are active in the program object
        GLAtomicCounterReferenceList mGLAtomicCounterReferences;
        /// Map of shared parameter blocks to uniform buffer references
        SharedParamsBufferMap mSharedParamsBufferMap;
        /// Container of counter buffer references that are active in the program object
        GLCounterBufferList mGLCounterBufferReferences;

        /// Linked hull (control) shader.
        GLSLShader* mHullShader;
        /// Linked domain (evaluation) shader.
        GLSLShader* mDomainShader;
        /// Linked geometry shader.
        GLSLShader* mGeometryShader;
        /// Linked fragment shader.
        GLSLShader* mFragmentShader;
        /// Linked compute shader.
        GLSLShader* mComputeShader;

        Ogre::String getCombinedName(void);
        /// Get the the binary data of a program from the microcode cache
        void getMicrocodeFromCache(void);
    };


}

#endif // __GLSLProgram_H__
