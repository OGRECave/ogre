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

#include "OgreGLSLProgramCommon.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLSLProgram.h"
#include "OgreRoot.h"

namespace Ogre {
    
	//-----------------------------------------------------------------------
	GLSLProgramCommon::GLSLProgramCommon(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* geometryProgram, GLSLGpuProgram* fragmentProgram, GLSLGpuProgram* hullProgram, GLSLGpuProgram* domainProgram, GLSLGpuProgram* computeProgram)
    : mVertexProgram(vertexProgram)
    , mFragmentProgram(fragmentProgram)
    , mGeometryProgram(geometryProgram)
    , mHullProgram(hullProgram)
    , mDomainProgram(domainProgram)
    , mComputeProgram(computeProgram)
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
	}

	//-----------------------------------------------------------------------
	GLSLProgramCommon::~GLSLProgramCommon(void)
	{
		OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));

        delete mVertexArrayObject;
        mVertexArrayObject = 0;
    }
    
	//-----------------------------------------------------------------------
	Ogre::String GLSLProgramCommon::getCombinedName()
	{
		String name;
		if (mVertexProgram)
		{
			name += "Vertex Program: ";
			name += mVertexProgram->getName();
			name += "\n";
		}
		if (mFragmentProgram)
		{
			name += "Fragment Program: ";
			name += mFragmentProgram->getName();
			name += "\n";
		}
		if (mGeometryProgram)
		{
			name += "Geometry Program: ";
			name += mGeometryProgram->getName();
			name += "\n";
		}
		if (mHullProgram)
		{
			name += "Tessellation Control Program: ";
			name += mHullProgram->getName();
			name += "\n";
		}
		if (mDomainProgram)
		{
			name += "Tessellation Evaluation Program: ";
			name += mDomainProgram->getName();
			name += "\n";
		}
		if (mComputeProgram)
		{
			name += "Compute Program: ";
			name += mComputeProgram->getName();
			name += "\n";
		}

		return name;
	}

	//-----------------------------------------------------------------------
    VertexElementSemantic GLSLProgramCommon::getAttributeSemanticEnum(String type)
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
	const char * GLSLProgramCommon::getAttributeSemanticString(VertexElementSemantic semantic)
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
	GLint GLSLProgramCommon::getAttributeIndex(VertexElementSemantic semantic, uint index)
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
	bool GLSLProgramCommon::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		return getAttributeIndex(semantic, index) != NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX;
	}
    //-----------------------------------------------------------------------
	void GLSLProgramCommon::getMicrocodeFromCache(void)
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

        GLint binaryLength = static_cast<GLint>(cacheMicrocode->size() - sizeof(GLenum));

        // load binary
		OGRE_CHECK_GL_ERROR(glProgramBinary(mGLProgramHandle,
                                            binaryFormat, 
                                            cacheMicrocode->getPtr(),
                                            binaryLength));

		GLint success = 0;
		OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &success));
		if (!success)
		{
			//
			// Something must have changed since the program binaries
			// were cached away. Fallback to source shader loading path,
			// and then retrieve and cache new program binaries once again.
			//
			compileAndLink();
		}
	}
    //-----------------------------------------------------------------------
    void GLSLProgramCommon::extractLayoutQualifiers(void)
    {
        // Format is:
        //      layout(location = 0) attribute vec4 vertex;

        if(mVertexProgram)
        {
            String shaderSource = mVertexProgram->getGLSLProgram()->getSource();
            String::size_type currPos = shaderSource.find("layout");
            while (currPos != String::npos)
            {
                VertexElementSemantic semantic;
                GLint index = 0;

                String::size_type endPos = shaderSource.find(";", currPos);
				if (endPos == String::npos)
				{
					// Problem, missing semicolon, abort
					break;
				}

				String line = shaderSource.substr(currPos, endPos - currPos);

                // Skip over 'layout'
                currPos += 6;

                // Skip until '='
                String::size_type eqPos = line.find("=");
                String::size_type parenPos = line.find(")");

                // Skip past '=' up to a ')' which contains an integer(the position).  This could be a definition, does the preprocessor do replacement?
                String attrLocation = line.substr(eqPos + 1, parenPos - eqPos - 1);
                StringUtil::trim(attrLocation);
                GLint attrib = StringConverter::parseInt(attrLocation);

                // The rest of the line is a standard attribute definition.
                // Erase up to it then split the remainder by spaces.
                line.erase (0, parenPos + 1);
                StringUtil::trim(line);
				StringVector parts = StringUtil::split(line, " ");

                if(parts.size() < 3)
                {
                    // This is a malformed attribute
                    // It should contain 3 parts, i.e. "attribute vec4 vertex"
                    break;
                }

                String attrName = parts[2];

                // Special case for attribute named position
                if(attrName == "position")
                    semantic = getAttributeSemanticEnum("vertex");
                else
                    semantic = getAttributeSemanticEnum(attrName);

                // Find the texture unit index
                String::size_type uvPos = attrName.find("uv");
                if(uvPos != String::npos)
                {
                    String uvIndex = attrName.substr(uvPos + 2, attrName.length() - 2);
                    index = StringConverter::parseInt(uvIndex);
                }

                mCustomAttributesIndexes[semantic-1][index] = attrib;

                currPos = shaderSource.find("layout", currPos);
            }
        }
    }

} // namespace Ogre
