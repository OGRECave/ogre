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

#include "OgreGLSLESProgramCommon.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLES2Util.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {
    
    //-----------------------------------------------------------------------
    GLSLESProgramCommon::GLSLESProgramCommon(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram)
    : mVertexProgram(vertexProgram)
    , mFragmentProgram(fragmentProgram)
    , mUniformRefsBuilt(false)
    , mLinked(false)
    , mTriedToLinkAndFailed(false)
    {
        // init mCustomAttributesIndexes
        for(size_t i = 0 ; i < VES_COUNT; i++)
            for(size_t j = 0 ; j < OGRE_MAX_TEXTURE_COORD_SETS; j++)
            {
                mCustomAttributesIndexes[i][j] = NULL_CUSTOM_ATTRIBUTES_INDEX;
            }

        // Initialize the attribute to semantic map
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("vertex", VES_POSITION));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("blendWeights", VES_BLEND_WEIGHTS));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("normal", VES_NORMAL));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("colour", VES_DIFFUSE));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("secondary_colour", VES_SPECULAR));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("blendIndices", VES_BLEND_INDICES));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("tangent", VES_TANGENT));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("binormal", VES_BINORMAL));
        mSemanticTypeMap.insert(SemanticToStringMap::value_type("uv", VES_TEXTURE_COORDINATES));

        if ((!mVertexProgram || !mFragmentProgram) && 
            !Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Attempted to create a shader program without both a vertex and fragment program.",
                        "GLSLESProgramCommon::GLSLESProgramCommon");
        }

        // Initialise uniform cache
        mUniformCache = new GLES2UniformCache();
    }
    
    //-----------------------------------------------------------------------
    GLSLESProgramCommon::~GLSLESProgramCommon(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));

        delete mUniformCache;
        mUniformCache = 0;
    }
    
    //-----------------------------------------------------------------------
    Ogre::String GLSLESProgramCommon::getCombinedName()
    {
        String name;
        if (mVertexProgram)
        {
            name += "Vertex Program:" ;
            name += mVertexProgram->getName();
        }
        if (mFragmentProgram)
        {
            name += " Fragment Program:" ;
            name += mFragmentProgram->getName();
        }
        name += "\n";

        return name;
    }

    //-----------------------------------------------------------------------
    VertexElementSemantic GLSLESProgramCommon::getAttributeSemanticEnum(String type)
    {
        VertexElementSemantic semantic = mSemanticTypeMap[type];
        if(semantic > 0)
        {
            return semantic;
        }
        else
        {
            assert(false && "Missing attribute!");
            return (VertexElementSemantic)0;
        }
    }
    
    //-----------------------------------------------------------------------
    const char * GLSLESProgramCommon::getAttributeSemanticString(VertexElementSemantic semantic)
    {
        for (SemanticToStringMap::iterator i = mSemanticTypeMap.begin(); i != mSemanticTypeMap.end(); ++i)
        {
            if((*i).second == semantic)
                return (*i).first.c_str();
        }

        assert(false && "Missing attribute!");
        return 0;
    }
    
    //-----------------------------------------------------------------------
    GLint GLSLESProgramCommon::getAttributeIndex(VertexElementSemantic semantic, uint index)
    {
        GLint res = mCustomAttributesIndexes[semantic-1][index];
        if (res == NULL_CUSTOM_ATTRIBUTES_INDEX)
        {
            const char * attString = getAttributeSemanticString(semantic);
            GLint attrib;
            OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(mGLProgramHandle, attString));

            // sadly position is a special case 
            if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX && semantic == VES_POSITION)
            {
                OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(mGLProgramHandle, "position"));
            }

            // for uv and other case the index is a part of the name
            if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX)
            {
                String attStringWithSemantic = String(attString) + StringConverter::toString(index);
                OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(mGLProgramHandle, attStringWithSemantic.c_str()));
            }

            // update mCustomAttributesIndexes with the index we found (or didn't find) 
            mCustomAttributesIndexes[semantic-1][index] = attrib;
            res = attrib;
        }
        return res;
    }
    //-----------------------------------------------------------------------
    bool GLSLESProgramCommon::isAttributeValid(VertexElementSemantic semantic, uint index)
    {
        return getAttributeIndex(semantic, index) != NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX;
    }
    //-----------------------------------------------------------------------
    void GLSLESProgramCommon::getMicrocodeFromCache(void)
    {
        GpuProgramManager::Microcode cacheMicrocode =
            GpuProgramManager::getSingleton().getMicrocodeFromCache(getCombinedName());

        // add to the microcode to the cache
        String name;
        name = getCombinedName();

        // turns out we need this param when loading
        GLenum binaryFormat = 0;

        cacheMicrocode->seek(0);

        // get size of binary
        cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

        if(getGLES2SupportRef()->checkExtension("GL_OES_get_program_binary") || gleswIsSupported(3, 0))
        {
            GLint binaryLength = static_cast<GLint>(cacheMicrocode->size() - sizeof(GLenum));

            // load binary
            OGRE_CHECK_GL_ERROR(glProgramBinaryOES(mGLProgramHandle,
                               binaryFormat, 
                               cacheMicrocode->getPtr(),
                               binaryLength));
        }

        GLint success = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &success));
        if (!success)
        {
            //
            // Something must have changed since the program binaries
            // were cached away.  Fallback to source shader loading path,
            // and then retrieve and cache new program binaries once again.
            //
            compileAndLink();
        }
    }

} // namespace Ogre
