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
#ifndef __GLSLProgramManager_H__
#define __GLSLProgramManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGLSLProgramManagerCommon.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLExtSupport.h"

namespace Ogre {

    class _OgreGL3PlusExport GLSLProgramManager : public GLSLProgramManagerCommon
    {
    protected:
        /// Active shader objects defining the active program object.
        GLSLShader* mActiveVertexShader;
        GLSLShader* mActiveHullShader;
        GLSLShader* mActiveDomainShader;
        GLSLShader* mActiveGeometryShader;
        GLSLShader* mActiveFragmentShader;
        GLSLShader* mActiveComputeShader;

        GL3PlusRenderSystem* mRenderSystem;

        /**  Convert GL uniform size and type to OGRE constant types
             and associate uniform definitions together. */
        void convertGLUniformtoOgreType(GLenum gltype,
                                        GpuConstantDefinition& defToUpdate);
        /** Find the data source definition for a given uniform name
            and reference. Return true if found and pair the reference
            with its data source. */
        static bool findUniformDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLUniformReference& refToUpdate);
        /** Find the data source definition for a given atomic counter
            uniform name and reference. Return true if found and pair
            the reference with its data source. */
        static bool findAtomicCounterDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLAtomicCounterReference& refToUpdate);
    public:

        GLSLProgramManager(GL3PlusRenderSystem* renderSystem);

        /** Populate a list of uniforms based on an OpenGL program object.
        */
        void extractUniformsFromProgram(
            GLuint programObject,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLUniformReferenceList& uniformList,
            GLAtomicCounterReferenceList& counterList,
            GLUniformBufferList& uniformBufferList,
            SharedParamsBufferMap& sharedParamsBufferMap,
            //GLShaderStorageBufferList& shaderStorageBufferList,
            GLCounterBufferList& counterBufferList);

        GL3PlusStateCacheManager* getStateCacheManager();
    };

}

#endif // __GLSLProgramManager_H__
