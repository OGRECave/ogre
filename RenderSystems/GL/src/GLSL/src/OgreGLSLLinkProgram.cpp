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

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreStringConverter.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreException.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLSLProgramCommon.h"

namespace Ogre {
    namespace GLSL {

    static GLint getGLGeometryInputPrimitiveType(RenderOperation::OperationType operationType)
    {
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return GL_POINTS;
        case RenderOperation::OT_LINE_LIST:
        case RenderOperation::OT_LINE_STRIP:
			return GL_LINES;
        case RenderOperation::OT_LINE_LIST_ADJ:
        case RenderOperation::OT_LINE_STRIP_ADJ:
			return GL_LINES_ADJACENCY_EXT;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            return GL_TRIANGLES_ADJACENCY_EXT;
        default:
        case RenderOperation::OT_TRIANGLE_LIST:
        case RenderOperation::OT_TRIANGLE_STRIP:
        case RenderOperation::OT_TRIANGLE_FAN:
            return GL_TRIANGLES;
		}
    }
    //-----------------------------------------------------------------------
    static GLint getGLGeometryOutputPrimitiveType(RenderOperation::OperationType operationType)
    {
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return GL_POINTS;
        case RenderOperation::OT_LINE_STRIP:
            return GL_LINE_STRIP;
        default:
        case RenderOperation::OT_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        }
    }

    //-----------------------------------------------------------------------
    GLSLLinkProgram::GLSLLinkProgram(const GLShaderList& shaders) : GLSLProgramCommon(shaders)
    {
        // Initialise uniform cache
        mUniformCache = new GLUniformCache();
    }

    //-----------------------------------------------------------------------
    GLSLLinkProgram::~GLSLLinkProgram(void)
    {
        glDeleteObjectARB(mGLProgramHandle);

        delete mUniformCache;
        mUniformCache = 0;
    }

    //-----------------------------------------------------------------------
    void GLSLLinkProgram::activate(void)
    {
        if (!mLinked)
        {           
            glGetError(); //Clean up the error. Otherwise will flood log.

            mGLProgramHandle = glCreateProgramObjectARB();

            GLenum glErr = glGetError();
            if(glErr != GL_NO_ERROR)
            {
                reportGLSLError( glErr, "GLSLLinkProgram::activate", "Error Creating GLSL Program Object", 0 );
            }

            uint32 hash = getCombinedHash();

            if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
                 GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(hash) &&
                 !mShaders[GPT_GEOMETRY_PROGRAM])
            {
                getMicrocodeFromCache(hash);
            }
            else
            {
                compileAndLink();

            }
            buildGLUniformReferences();
            extractAttributes();
        }
        if (mLinked)
        {
            GLenum glErr = glGetError();
            if(glErr != GL_NO_ERROR)
            {
                reportGLSLError( glErr, "GLSLLinkProgram::Activate",
                    "Error prior to using GLSL Program Object : ", mGLProgramHandle, false, false);
            }

            glUseProgramObjectARB( mGLProgramHandle );

            glErr = glGetError();
            if(glErr != GL_NO_ERROR)
            {
                reportGLSLError( glErr, "GLSLLinkProgram::Activate",
                    "Error using GLSL Program Object : ", mGLProgramHandle, false, false);
            }
        }
    }
    //-----------------------------------------------------------------------
    void GLSLLinkProgram::getMicrocodeFromCache(uint32 id)
    {
        GpuProgramManager::Microcode cacheMicrocode = 
            GpuProgramManager::getSingleton().getMicrocodeFromCache(id);
        
        GLenum binaryFormat = *((GLenum *)(cacheMicrocode->getPtr()));
        uint8 * programBuffer = cacheMicrocode->getPtr() + sizeof(GLenum);
        size_t sizeOfBuffer = cacheMicrocode->size() - sizeof(GLenum);
        glProgramBinary(mGLProgramHandle,
                        binaryFormat, 
                        programBuffer,
                        static_cast<GLsizei>(sizeOfBuffer)
                        );

        glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &mLinked);
        if (!mLinked)
        {
            //
            // Something must have changed since the program binaries
            // were cached away.  Fallback to source shader loading path,
            // and then retrieve and cache new program binaries once again.
            //
            compileAndLink();
        }
    }
    //-----------------------------------------------------------------------
    void GLSLLinkProgram::extractAttributes(void)
    {
        size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);

        for (size_t i = 0; i < numAttribs; ++i)
        {
            const CustomAttribute& a = msCustomAttributes[i];
            GLint attrib = glGetAttribLocationARB(mGLProgramHandle, a.name);

            if (attrib != -1)
            {
                mValidAttributes.insert(a.attrib);

                if(a.semantic != VES_TEXTURE_COORDINATES) continue;

                // also enable next 4 attributes to allow matrix types in texcoord semantic
                // might cause problems with mixing builtin and custom names,
                // but then again you should not
                for(int j = 0; j < 4; j++)
                    mValidAttributes.insert(msCustomAttributes[i + j].attrib);
            }
        }
    }
    //-----------------------------------------------------------------------
    bool GLSLLinkProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
    {
        return mValidAttributes.find(getFixedAttributeIndex(semantic, index)) != mValidAttributes.end();
    }
    //-----------------------------------------------------------------------
    void GLSLLinkProgram::buildGLUniformReferences(void)
    {
        if (!mUniformRefsBuilt)
        {
            const GpuConstantDefinitionMap* vertParams = 0;
            const GpuConstantDefinitionMap* fragParams = 0;
            const GpuConstantDefinitionMap* geomParams = 0;
            if (mShaders[GPT_VERTEX_PROGRAM])
            {
                vertParams = &(mShaders[GPT_VERTEX_PROGRAM]->getConstantDefinitions().map);
            }
            if (mShaders[GPT_GEOMETRY_PROGRAM])
            {
                geomParams = &(mShaders[GPT_GEOMETRY_PROGRAM]->getConstantDefinitions().map);
            }
            if (mShaders[GPT_FRAGMENT_PROGRAM])
            {
                fragParams = &(mShaders[GPT_FRAGMENT_PROGRAM]->getConstantDefinitions().map);
            }

            GLSLLinkProgramManager::extractUniforms(
                    mGLProgramHandle, vertParams, geomParams, fragParams, mGLUniformReferences);

            mUniformRefsBuilt = true;
        }
    }

    //-----------------------------------------------------------------------
    void GLSLLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params, 
        uint16 mask, GpuProgramType fromProgType)
    {
        // iterate through uniform reference list and update uniform values
        GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
        GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

        // determine if we need to transpose matrices when binding
        bool transpose = !mShaders[fromProgType] || mShaders[fromProgType]->getColumnMajorMatrices();

        for (;currentUniform != endUniform; ++currentUniform)
        {
            // Only pull values from buffer it's supposed to be in (vertex or fragment)
            // This method will be called twice, once for vertex program params, 
            // and once for fragment program params.
            if (fromProgType == currentUniform->mSourceProgType)
            {
                const GpuConstantDefinition* def = currentUniform->mConstantDef;
                if (def->variability & mask)
                {

                    GLsizei glArraySize = (GLsizei)def->arraySize;

                    bool shouldUpdate = true;

                    switch (def->constType)
                    {
                        case GCT_INT1:
                        case GCT_INT2:
                        case GCT_INT3:
                        case GCT_INT4:
                        case GCT_SAMPLER1D:
                        case GCT_SAMPLER1DSHADOW:
                        case GCT_SAMPLER2D:
                        case GCT_SAMPLER2DSHADOW:
                        case GCT_SAMPLER2DARRAY:
                        case GCT_SAMPLER3D:
                        case GCT_SAMPLERCUBE:
                            shouldUpdate = mUniformCache->updateUniform(currentUniform->mLocation,
                                                                        params->getIntPointer(def->physicalIndex),
                                                                        static_cast<GLsizei>(def->elementSize * def->arraySize * sizeof(int)));
                            break;
                        default:
                            shouldUpdate = mUniformCache->updateUniform(currentUniform->mLocation,
                                                                        params->getFloatPointer(def->physicalIndex),
                                                                        static_cast<GLsizei>(def->elementSize * def->arraySize * sizeof(float)));
                            break;

                    }

                    if(!shouldUpdate)
                        continue;

                    // get the index in the parameter real list
                    switch (def->constType)
                    {
                    case GCT_FLOAT1:
                        glUniform1fvARB(currentUniform->mLocation, glArraySize, 
                            params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_FLOAT2:
                        glUniform2fvARB(currentUniform->mLocation, glArraySize, 
                            params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_FLOAT3:
                        glUniform3fvARB(currentUniform->mLocation, glArraySize, 
                            params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_FLOAT4:
                        glUniform4fvARB(currentUniform->mLocation, glArraySize, 
                            params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_MATRIX_2X2:
                        glUniformMatrix2fvARB(currentUniform->mLocation, glArraySize, 
                            transpose, params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_MATRIX_2X3:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix2x3fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_2X4:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix2x4fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_3X2:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix3x2fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_3X3:
                        glUniformMatrix3fvARB(currentUniform->mLocation, glArraySize, 
                            transpose, params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_MATRIX_3X4:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix3x4fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_4X2:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix4x2fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_4X3:
                        if (GLEW_VERSION_2_1)
                        {
                            glUniformMatrix4x3fv(currentUniform->mLocation, glArraySize, 
                                GL_FALSE, params->getFloatPointer(def->physicalIndex));
                        }
                        break;
                    case GCT_MATRIX_4X4:
                        glUniformMatrix4fvARB(currentUniform->mLocation, glArraySize, 
                            transpose, params->getFloatPointer(def->physicalIndex));
                        break;
                    case GCT_SAMPLER1D:
                    case GCT_SAMPLER1DSHADOW:
                    case GCT_SAMPLER2D:
                    case GCT_SAMPLER2DSHADOW:
                    case GCT_SAMPLER2DARRAY:
                    case GCT_SAMPLER3D:
                    case GCT_SAMPLERCUBE:
                        // samplers handled like 1-element ints
                    case GCT_INT1:
                        glUniform1ivARB(currentUniform->mLocation, glArraySize, 
                            (GLint*)params->getIntPointer(def->physicalIndex));
                        break;
                    case GCT_INT2:
                        glUniform2ivARB(currentUniform->mLocation, glArraySize, 
                            (GLint*)params->getIntPointer(def->physicalIndex));
                        break;
                    case GCT_INT3:
                        glUniform3ivARB(currentUniform->mLocation, glArraySize, 
                            (GLint*)params->getIntPointer(def->physicalIndex));
                        break;
                    case GCT_INT4:
                        glUniform4ivARB(currentUniform->mLocation, glArraySize, 
                            (GLint*)params->getIntPointer(def->physicalIndex));
                        break;
                    case GCT_UNKNOWN:
                    default:
                        break;

                    } // end switch
    #if OGRE_DEBUG_MODE
                    GLenum glErr = glGetError();
                    if(glErr != GL_NO_ERROR)
                    {
                        reportGLSLError( glErr, "GLSLLinkProgram::updateUniforms", "Error updating uniform", 0 );
                    }
    #endif
                } // variability & mask
            } // fromProgType == currentUniform->mSourceProgType
  
        } // end for
    }
    //-----------------------------------------------------------------------
    void GLSLLinkProgram::compileAndLink()
    {
        uint32 hash = 0;
        if (mShaders[GPT_VERTEX_PROGRAM])
        {
            // attach Vertex Program
            mShaders[GPT_VERTEX_PROGRAM]->attachToProgramObject(mGLProgramHandle);

            // Some drivers (e.g. OS X on nvidia) incorrectly determine the attribute binding automatically

            // and end up aliasing existing built-ins. So avoid! 
            // Bind all used attribs - not all possible ones otherwise we'll get 
            // lots of warnings in the log, and also may end up aliasing names used
            // as varyings by accident
            // Because we can't ask GL whether an attribute is used in the shader
            // until it is linked (chicken and egg!) we have to parse the source

            size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
            const String& vpSource = mShaders[GPT_VERTEX_PROGRAM]->getSource();
            
            hash = mShaders[GPT_VERTEX_PROGRAM]->_getHash(hash);
            for (size_t i = 0; i < numAttribs; ++i)
            {
                const CustomAttribute& a = msCustomAttributes[i];

                // we're looking for either: 
                //   attribute vec<n> <semantic_name>
                //   in vec<n> <semantic_name>
                // The latter is recommended in GLSL 1.3 onwards 
                // be slightly flexible about formatting
                String::size_type pos = vpSource.find(a.name);
                bool foundAttr = false;
                while (pos != String::npos && !foundAttr)
                {
                    String::size_type startpos = vpSource.find("attribute", pos < 20 ? 0 : pos-20);
                    if (startpos == String::npos)
                        startpos = vpSource.find("in", pos-20);
                    if (startpos != String::npos && startpos < pos)
                    {
                        // final check 
                        String expr = vpSource.substr(startpos, pos + strlen(a.name) - startpos);
                        StringVector vec = StringUtil::split(expr);
                        if ((vec[0] == "in" || vec[0] == "attribute") && vec[2] == a.name)
                        {
                            glBindAttribLocationARB(mGLProgramHandle, a.attrib, a.name);
                            foundAttr = true;
                        }
                    }
                    // Find the position of the next occurrence if needed
                    pos = vpSource.find(a.name, pos + strlen(a.name));
                }
            }
        }

        if (auto gshader = static_cast<GLSLProgram*>(mShaders[GPT_GEOMETRY_PROGRAM]))
        {
            hash = mShaders[GPT_GEOMETRY_PROGRAM]->_getHash(hash);
            // attach Geometry Program
            mShaders[GPT_GEOMETRY_PROGRAM]->attachToProgramObject(mGLProgramHandle);

            //Don't set adjacency flag. We handle it internally and expose "false"

            RenderOperation::OperationType inputOperationType = gshader->getInputOperationType();
            glProgramParameteriEXT(mGLProgramHandle, GL_GEOMETRY_INPUT_TYPE_EXT,
                getGLGeometryInputPrimitiveType(inputOperationType));

            RenderOperation::OperationType outputOperationType = gshader->getOutputOperationType();

            glProgramParameteriEXT(mGLProgramHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT,
                getGLGeometryOutputPrimitiveType(outputOperationType));

            glProgramParameteriEXT(mGLProgramHandle, GL_GEOMETRY_VERTICES_OUT_EXT,
                gshader->getMaxOutputVertices());
        }

        if (mShaders[GPT_FRAGMENT_PROGRAM])
        {
            hash = mShaders[GPT_FRAGMENT_PROGRAM]->_getHash(hash);
            // attach Fragment Program
            mShaders[GPT_FRAGMENT_PROGRAM]->attachToProgramObject(mGLProgramHandle);
        }

        
        // now the link

        glLinkProgramARB( mGLProgramHandle );
        glGetObjectParameterivARB( mGLProgramHandle, GL_OBJECT_LINK_STATUS_ARB, &mLinked );

        // force logging and raise exception if not linked
        GLenum glErr = glGetError();
        if(glErr != GL_NO_ERROR)
        {
            reportGLSLError( glErr, "GLSLLinkProgram::compileAndLink",
                "Error linking GLSL Program Object : ", mGLProgramHandle, !mLinked, !mLinked );
        }
        
        if(mLinked)
        {
            logObjectInfo(  getCombinedName() + String(" GLSL link result : "), mGLProgramHandle );
        }

        if (mLinked)
        {
            if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
            {
                // add to the microcode to the cache

                // get buffer size
                GLint binaryLength = 0;
                glGetProgramiv(mGLProgramHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

                // turns out we need this param when loading
                // it will be the first bytes of the array in the microcode
                GLenum binaryFormat = 0; 

                // create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode(binaryLength + sizeof(GLenum));

                // get binary
                uint8 * programBuffer = newMicrocode->getPtr() + sizeof(GLenum);
                glGetProgramBinary(mGLProgramHandle, binaryLength, NULL, &binaryFormat, programBuffer);

                // save binary format
                memcpy(newMicrocode->getPtr(), &binaryFormat, sizeof(GLenum));

                // add to the microcode to the cache
                GpuProgramManager::getSingleton().addMicrocodeToCache(hash, newMicrocode);
            }
        }
    }
    //-----------------------------------------------------------------------
} // namespace GLSL
} // namespace Ogre
