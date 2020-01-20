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

#include "OgreGLSLProgramManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLShader.h"
#include "OgreGpuProgramManager.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"

#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLSeparableProgram.h"

namespace Ogre {

    template<> GLSLProgramManager* Singleton<GLSLProgramManager>::msSingleton = 0;


    GLSLProgramManager* GLSLProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }


    GLSLProgramManager& GLSLProgramManager::getSingleton(void)
    {
        assert(msSingleton);
        return (*msSingleton);
    }
    
    GLSLProgramManager::GLSLProgramManager(GL3PlusRenderSystem* renderSystem)
        : mActiveProgram(NULL), mRenderSystem(renderSystem)
    {
    }

    GLSLProgramManager::~GLSLProgramManager(void) {}

    GL3PlusStateCacheManager* GLSLProgramManager::getStateCacheManager()
    {
        return mRenderSystem->_getStateCacheManager();
    }

    GLSLProgram* GLSLProgramManager::getActiveProgram(void)
    {
        // If there is an active link program then return it.
        if (mActiveProgram)
            return mActiveProgram;

        // No active link program so find one or make a new one.
        // Is there an active key?
        uint32 activeKey = 0;
        for(auto shader : mActiveShader)
        {
            if(!shader) continue;

            // overwrite as compute shaders are not part of the pipeline
            if(shader->getType() == GPT_COMPUTE_PROGRAM)
                activeKey = 0;

            activeKey = HashCombine(activeKey, shader->getShaderID());
        }

        // Only return a link program object if a program exists.
        if (activeKey > 0)
        {
            // Find the key in the hash map.
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it.
            if (programFound == mPrograms.end())
            {
                if (mRenderSystem->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
                {
                    mActiveProgram = new GLSLSeparableProgram(mActiveShader);
                }
                else
                {
                    mActiveProgram = new GLSLMonolithicProgram(mActiveShader);
                }

                mPrograms[activeKey] = mActiveProgram;
            }
            else
            {
                // Found a link program in map container so make it active.
                mActiveProgram = static_cast<GLSLProgram*>(programFound->second);
            }
        }

        // Make the program object active.
        if (mActiveProgram)
            mActiveProgram->activate();

        return mActiveProgram;
    }

    void GLSLProgramManager::setActiveShader(GpuProgramType type, GLSLShader* shader)
    {
        if (mActiveShader[type] != shader)
        {
            mActiveShader[type] = shader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }

    bool GLSLProgramManager::findUniformDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
        GLUniformReference& refToUpdate)
    {
        for(int i = 0; i < 6; i++) {
            if (constantDefs[i])
            {
                GpuConstantDefinitionMap::const_iterator parami =
                        constantDefs[i]->find(paramName);
                if (parami != constantDefs[i]->end())
                {
                    refToUpdate.mSourceProgType = static_cast<GpuProgramType>(i);
                    refToUpdate.mConstantDef = &(parami->second);
                    return true;
                }
            }
        }
        return false;
    }

    
    //FIXME This is code bloat...either template or unify UniformReference
    // and AtomicCounterReference
    bool GLSLProgramManager::findAtomicCounterDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
        GLAtomicCounterReference& refToUpdate)
    {
        for(int i = 0; i < 6; i++) {
            if (constantDefs[i])
            {
                GpuConstantDefinitionMap::const_iterator parami =
                        constantDefs[i]->find(paramName);
                if (parami != constantDefs[i]->end())
                {
                    refToUpdate.mSourceProgType = static_cast<GpuProgramType>(i);
                    refToUpdate.mConstantDef = &(parami->second);
                    return true;
                }
            }
        }
        return false;
    }


    
    void GLSLProgramManager::extractUniformsFromProgram(
        GLuint programObject,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
        GLUniformReferenceList& uniformList,
        GLAtomicCounterReferenceList& counterList,
        GLCounterBufferList& counterBufferList)
    {
#define uniformLength 200
        //              GLint uniformLength = 0;
        //        glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformLength);

        char uniformName[uniformLength] = {0};
        GLAtomicCounterReference newGLAtomicCounterReference;
        GLUniformReference newGLUniformReference;


        // Get the number of active uniforms, including atomic
        // counters and uniforms contained in uniform blocks.
        GLint uniformCount = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount));

        // Scan through the active uniforms and add them to the reference list.
        for (GLuint index = 0; index < (GLuint)uniformCount; index++)
        {
            GLint arraySize;
            GLenum glType;
            OGRE_CHECK_GL_ERROR(glGetActiveUniform(programObject, index, uniformLength, NULL,
                                                   &arraySize, &glType, uniformName));

            // Don't add built in uniforms, atomic counters, or uniform block parameters.
            OGRE_CHECK_GL_ERROR(newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName));

            // User defined uniform found, add it to the reference list.
            String paramName = String(uniformName);

            // ATI drivers (Catalyst 7.2 and earlier) and
            // older NVidia drivers will include all array
            // elements as uniforms but we only want the root
            // array name and location. Also note that ATI Catalyst
            // 6.8 to 7.2 there is a bug with glUniform that does
            // not allow you to update a uniform array past the
            // first uniform array element ie you can't start
            // updating an array starting at element 1, must
            // always be element 0.

            // If the uniform name ends with "]" then its an array element uniform
            if (paramName.back() == ']')
            {
                // if not the first array element then skip it and continue to the next uniform
                if (paramName.compare(paramName.size() - 3, 3, "[0]") != 0) continue;
                paramName.resize(paramName.size() - 3);
            }

            if (newGLUniformReference.mLocation >= 0)
            {
                // Find out which params object this comes from
                bool foundSource = findUniformDataSource(paramName, constantDefs, newGLUniformReference);

                // Only add this parameter if we found the source
                if (foundSource)
                {
                    assert(size_t (arraySize) == newGLUniformReference.mConstantDef->arraySize
                           && "GL doesn't agree with our array size!");
                    uniformList.push_back(newGLUniformReference);
                }

                // Don't bother adding individual array params, they will be
                // picked up in the 'parent' parameter can copied all at once
                // anyway, individual indexes are only needed for lookup from
                // user params
            }
            else if (glType == GL_UNSIGNED_INT_ATOMIC_COUNTER)
            {
                // Handle atomic counters. Currently atomic counters
                // cannot be in uniform blocks and are always unsigned
                // integers.
                GLint binding, offset;
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, 1, &index, GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX, &binding));
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, 1, &index, GL_UNIFORM_OFFSET, &offset));

                newGLAtomicCounterReference.mBinding = binding;
                newGLAtomicCounterReference.mOffset = offset;

                // increment the total number of atomic counters
                // including size of array if applicable
                //atomicCounterCount += arraySize;
                // actually, this should not be necessary since
                // parameters are processed one by one

                printf("ATOMIC COUNTER FOUND: %s %d", paramName.c_str(), arraySize);

                // Find out which params object this comes from
                bool foundSource = findAtomicCounterDataSource(
                    paramName, constantDefs,newGLAtomicCounterReference);

                // Only add this parameter if we found the source
                if (foundSource)
                {
                    // size_t adjustedArraySize = 0;
                    // if (arraySize == 2 && newGLAtomicCounterReference.mConstantDef->arraySize == 1) {
                    //     adjustedArraySize = 1;
                    // }
                    // else {
                    //     adjustedArraySize = (size_t) arraySize;
                    // }

                    //FIXME On Linux AMD Catalyst 13.4, OpenGL reports
                    // a single atomic counter as having size 2.  Bug
                    // or feature?
                    // assert((size_t)arraySize == newGLAtomicCounterReference.mConstantDef->arraySize
                    //        && "GL doesn't agree with our array size!");

                    counterList.push_back(newGLAtomicCounterReference);
                    printf("ATOMIC COUNTER SOURCE FOUND\n");
                }
            }
        } // end for


        // FIXME uniform buffers need to be created during material script parsing of shared params

        // FIXME Ogre materials need a new shared param that is associated with an entity.
        // This could be impemented as a switch-like statement inside shared_params:

        // Now deal with uniform blocks
        auto& hbm = static_cast<GL3PlusHardwareBufferManager&>(HardwareBufferManager::getSingleton());
        GLint blockCount = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

        for (int index = 0; index < blockCount; index++)
        {
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockName(programObject, index, uniformLength, NULL, uniformName));

            // Map uniform block to binding point of GL buffer of
            // shared param bearing the same name.
            GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

            HardwareBufferPtr hwGlBuffer = blockSharedParams->_getHardwareBuffer();
            if (!hwGlBuffer)
            {
                // Create buffer and add entry to buffer map.
                GLint blockSize;
                OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));

                auto binding = hbm.getUniformBufferCount();
                hwGlBuffer = hbm.createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);
                static_cast<GL3PlusHardwareUniformBuffer*>(hwGlBuffer.get())->setGLBufferBinding(int(binding));

                blockSharedParams->_setHardwareBuffer(hwGlBuffer);
            }

            OGRE_CHECK_GL_ERROR(glUniformBlockBinding(
                programObject, index,
                static_cast<GL3PlusHardwareUniformBuffer*>(hwGlBuffer.get())->getGLBufferBinding()));
        }

        // Now deal with shader storage blocks
        if (mRenderSystem->hasMinGLVersion(4, 3) || mRenderSystem->checkExtension("GL_ARB_program_interface_query"))
        {
            OGRE_CHECK_GL_ERROR(glGetProgramInterfaceiv(programObject, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &blockCount));

            //TODO error if GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS > # shader_storage_blocks
            // do same for other buffers

            for (int index = 0; index < blockCount; index++)
            {
                OGRE_CHECK_GL_ERROR(glGetProgramResourceName(programObject, GL_SHADER_STORAGE_BLOCK, index, uniformLength, NULL, uniformName));

                // Map uniform block to binding point of GL buffer of
                // shared param bearing the same name.

                GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

                HardwareBufferPtr hwGlBuffer = blockSharedParams->_getHardwareBuffer();
                if (!hwGlBuffer)
                {
                    // Create buffer and add entry to buffer map.
                    GLint blockSize;
                    // const GLenum properties [2] = {GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING};
                    GLenum properties[] = {GL_BUFFER_DATA_SIZE};
                    OGRE_CHECK_GL_ERROR(glGetProgramResourceiv(programObject, GL_SHADER_STORAGE_BLOCK, index, 1, properties, 1, NULL, &blockSize));
                    //TODO Implement shared param access param in materials (R, W, R+W)

                    auto binding = hbm.getShaderStorageBufferCount();
                    hwGlBuffer = hbm.createShaderStorageBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC, false, uniformName);
                    static_cast<GL3PlusHardwareUniformBuffer*>(hwGlBuffer.get())->setGLBufferBinding(binding);

                    blockSharedParams->_setHardwareBuffer(hwGlBuffer);
                }

                OGRE_CHECK_GL_ERROR(glShaderStorageBlockBinding(
                    programObject, index,
                    static_cast<GL3PlusHardwareUniformBuffer*>(hwGlBuffer.get())->getGLBufferBinding()));
            }
        }

        if (mRenderSystem->hasMinGLVersion(4, 2) || mRenderSystem->checkExtension("GL_ARB_shader_atomic_counters"))
        {
            // Now deal with atomic counters buffers
            OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &blockCount));

            for (int index = 0; index < blockCount; index++)
            {
                //TODO Is this necessary?
                //GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

                //TODO We could build list of atomic counters here or above,
                // whichever is most efficient.
                // GLint * active_indices;
                // OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, active_indices));

                GLint bufferSize, bufferBinding;
                OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE, &bufferSize));
                OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_BINDING, &bufferBinding));
                //TODO check parameters of this GL call
                HardwareCounterBufferSharedPtr newCounterBuffer = HardwareBufferManager::getSingleton().createCounterBuffer(bufferSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);

                GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(newCounterBuffer.get());
                hwGlBuffer->setGLBufferBinding(bufferBinding);
                counterBufferList.push_back(newCounterBuffer);
            }
        }
    }
}
