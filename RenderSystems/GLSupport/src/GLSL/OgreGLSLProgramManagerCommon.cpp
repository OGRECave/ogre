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

#include "OgreGLSLProgramManagerCommon.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLProgramCommon.h"

namespace Ogre {

    GLSLProgramManagerCommon::GLSLProgramManagerCommon() : mActiveProgram(NULL)
    {
        mActiveShader.fill(NULL);

        // Fill in the relationship between type names and enums
        mTypeEnumMap.emplace("float", GCT_FLOAT1);
        mTypeEnumMap.emplace("vec2", GCT_FLOAT2);
        mTypeEnumMap.emplace("vec3", GCT_FLOAT3);
        mTypeEnumMap.emplace("vec4", GCT_FLOAT4);
        mTypeEnumMap.emplace("sampler1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("sampler2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("sampler3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("samplerCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("sampler1DShadow", GCT_SAMPLER1DSHADOW);
        mTypeEnumMap.emplace("sampler2DShadow", GCT_SAMPLER2DSHADOW);
        mTypeEnumMap.emplace("int", GCT_INT1);
        mTypeEnumMap.emplace("ivec2", GCT_INT2);
        mTypeEnumMap.emplace("ivec3", GCT_INT3);
        mTypeEnumMap.emplace("ivec4", GCT_INT4);
        mTypeEnumMap.emplace("bool", GCT_BOOL1);
        mTypeEnumMap.emplace("bvec2", GCT_BOOL2);
        mTypeEnumMap.emplace("bvec3", GCT_BOOL3);
        mTypeEnumMap.emplace("bvec4", GCT_BOOL4);
        mTypeEnumMap.emplace("mat2", GCT_MATRIX_2X2);
        mTypeEnumMap.emplace("mat3", GCT_MATRIX_3X3);
        mTypeEnumMap.emplace("mat4", GCT_MATRIX_4X4);

        // GLES2 ext
        mTypeEnumMap.emplace("samplerExternalOES", GCT_SAMPLER_EXTERNAL_OES);

        // GL 2.1
        mTypeEnumMap.emplace("mat2x2", GCT_MATRIX_2X2);
        mTypeEnumMap.emplace("mat3x3", GCT_MATRIX_3X3);
        mTypeEnumMap.emplace("mat4x4", GCT_MATRIX_4X4);
        mTypeEnumMap.emplace("mat2x3", GCT_MATRIX_2X3);
        mTypeEnumMap.emplace("mat3x2", GCT_MATRIX_3X2);
        mTypeEnumMap.emplace("mat3x4", GCT_MATRIX_3X4);
        mTypeEnumMap.emplace("mat4x3", GCT_MATRIX_4X3);
        mTypeEnumMap.emplace("mat2x4", GCT_MATRIX_2X4);
        mTypeEnumMap.emplace("mat4x2", GCT_MATRIX_4X2);

        // GL 3.0
        mTypeEnumMap.emplace("uint", GCT_UINT1);
        mTypeEnumMap.emplace("uvec2", GCT_UINT2);
        mTypeEnumMap.emplace("uvec3", GCT_UINT3);
        mTypeEnumMap.emplace("uvec4", GCT_UINT4);
        mTypeEnumMap.emplace("samplerCubeShadow", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("sampler1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("sampler2DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("sampler1DArrayShadow", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("sampler2DArrayShadow", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("isampler1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("isampler2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("isampler3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("isamplerCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("isampler1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("isampler2DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("usampler1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("usampler2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("usampler3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("usamplerCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("usampler1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("usampler2DArray", GCT_SAMPLER2DARRAY);

        // GL 3.1
        mTypeEnumMap.emplace("sampler2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("sampler2DRectShadow", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("isampler2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("usampler2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("samplerBuffer", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("isamplerBuffer", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("usamplerBuffer", GCT_SAMPLER1D);

        // GL 3.2
        mTypeEnumMap.emplace("sampler2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("isampler2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("usampler2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("sampler2DMSArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("isampler2DMSArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("usampler2DMSArray", GCT_SAMPLER2DARRAY);

        // GL 4.0
        mTypeEnumMap.emplace("double", GCT_DOUBLE1);
        mTypeEnumMap.emplace("dmat2", GCT_MATRIX_DOUBLE_2X2);
        mTypeEnumMap.emplace("dmat3", GCT_MATRIX_DOUBLE_3X3);
        mTypeEnumMap.emplace("dmat4", GCT_MATRIX_DOUBLE_4X4);
        mTypeEnumMap.emplace("dmat2x2", GCT_MATRIX_DOUBLE_2X2);
        mTypeEnumMap.emplace("dmat3x3", GCT_MATRIX_DOUBLE_3X3);
        mTypeEnumMap.emplace("dmat4x4", GCT_MATRIX_DOUBLE_4X4);
        mTypeEnumMap.emplace("dmat2x3", GCT_MATRIX_DOUBLE_2X3);
        mTypeEnumMap.emplace("dmat3x2", GCT_MATRIX_DOUBLE_3X2);
        mTypeEnumMap.emplace("dmat3x4", GCT_MATRIX_DOUBLE_3X4);
        mTypeEnumMap.emplace("dmat4x3", GCT_MATRIX_DOUBLE_4X3);
        mTypeEnumMap.emplace("dmat2x4", GCT_MATRIX_DOUBLE_2X4);
        mTypeEnumMap.emplace("dmat4x2", GCT_MATRIX_DOUBLE_4X2);
        mTypeEnumMap.emplace("dvec2", GCT_DOUBLE2);
        mTypeEnumMap.emplace("dvec3", GCT_DOUBLE3);
        mTypeEnumMap.emplace("dvec4", GCT_DOUBLE4);
        mTypeEnumMap.emplace("samplerCubeArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("samplerCubeArrayShadow", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("isamplerCubeArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("usamplerCubeArray", GCT_SAMPLER2DARRAY);

        //TODO should image be its own type?
        mTypeEnumMap.emplace("image1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("iimage1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("uimage1D", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("image2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("iimage2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("uimage2D", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("image3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("iimage3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("uimage3D", GCT_SAMPLER3D);
        mTypeEnumMap.emplace("image2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("iimage2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("uimage2DRect", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("imageCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("iimageCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("uimageCube", GCT_SAMPLERCUBE);
        mTypeEnumMap.emplace("imageBuffer", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("iimageBuffer", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("uimageBuffer", GCT_SAMPLER1D);
        mTypeEnumMap.emplace("image1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("iimage1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("uimage1DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("image2DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("iimage2DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("uimage2DArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("imageCubeArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("iimageCubeArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("uimageCubeArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("image2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("iimage2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("uimage2DMS", GCT_SAMPLER2D);
        mTypeEnumMap.emplace("image2DMSArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("iimage2DMSArray", GCT_SAMPLER2DARRAY);
        mTypeEnumMap.emplace("uimage2DMSArray", GCT_SAMPLER2DARRAY);

        // GL 4.2
        mTypeEnumMap.emplace("atomic_uint", GCT_UINT1); //TODO should be its own type?
    }

    GLSLProgramManagerCommon::~GLSLProgramManagerCommon()
    {
        // iterate through map container and delete link programs
        for (auto & p : mPrograms)
        {
            delete p.second;
        }
    }

    void GLSLProgramManagerCommon::destroyAllByShader(GLSLShaderCommon* shader)
    {
        std::vector<uint32> keysToErase;
        for (auto & p : mPrograms)
        {
            GLSLProgramCommon* prgm = p.second;
            if(prgm->isUsingShader(shader))
            {
                OGRE_DELETE prgm;
                keysToErase.push_back(p.first);
            }
        }

        for(unsigned int & i : keysToErase)
        {
            mPrograms.erase(mPrograms.find(i));
        }
    }

    void GLSLProgramManagerCommon::setActiveShader(GpuProgramType type, GLSLShaderCommon* shader)
    {
        if (mActiveShader[type] != shader)
        {
            mActiveShader[type] = shader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }

    void GLSLProgramManagerCommon::parseGLSLUniform(String line, GpuNamedConstants& defs,
                                                    const String& filename)
    {
        GpuConstantDefinition def;
        String paramName = "";

        // Remove spaces before opening square braces, otherwise
        // the following split() can split the line at inappropriate
        // places (e.g. "vec3 something [3]" won't work).
        //FIXME What are valid ways of including spaces in GLSL
        // variable declarations?  May need regex.
        for (String::size_type sqp = line.find (" ["); sqp != String::npos;
             sqp = line.find (" ["))
            line.erase (sqp, 1);
        // Split into tokens
        StringVector parts = StringUtil::split(line, ", \t\r\n");

        for (auto & part : parts)
        {
            // Is this a type?
            StringToEnumMap::iterator typei = mTypeEnumMap.find(part);
            if (typei != mTypeEnumMap.end())
            {
                def.constType = typei->second;
                // GL doesn't pad
                def.elementSize = GpuConstantDefinition::getElementSize(def.constType, false);
            }
            else
            {
                // if this is not a type, and not empty, it should be a name
                StringUtil::trim(part);
                if (part.empty()) continue;

                // Skip over precision keywords
                if(StringUtil::match(part, "lowp") ||
                   StringUtil::match(part, "mediump") ||
                   StringUtil::match(part, "highp"))
                    continue;

                String::size_type arrayStart = part.find("[", 0);
                if (arrayStart != String::npos)
                {
                    // potential name (if butted up to array)
                    String name = part.substr(0, arrayStart);
                    StringUtil::trim(name);
                    if (!name.empty())
                        paramName = name;

                    def.arraySize = 1;

                    // N-dimensional arrays
                    while (arrayStart != String::npos) {
                        String::size_type arrayEnd = part.find("]", arrayStart);
                        String arrayDimTerm = part.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                        StringUtil::trim(arrayDimTerm);
                        //TODO
                        // the array term might be a simple number or it might be
                        // an expression (e.g. 24*3) or refer to a constant expression
                        // we'd have to evaluate the expression which could get nasty
                        def.arraySize *= StringConverter::parseInt(arrayDimTerm);
                        arrayStart = part.find("[", arrayEnd);
                    }
                }
                else
                {
                    paramName = part;
                    def.arraySize = 1;
                }

                // Name should be after the type, so complete def and add
                // We do this now so that comma-separated params will do
                // this part once for each name mentioned
                if (def.constType == GCT_UNKNOWN)
                {
                    LogManager::getSingleton().logMessage("Problem parsing the following GLSL Uniform: '"
                                                          + line + "' in file " + filename, LML_CRITICAL);
                    // next uniform
                    break;
                }

                // Complete def and add
                // increment physical buffer location
                def.logicalIndex = 0; // not valid in GLSL
                if (def.isFloat() || def.isDouble() || def.isInt() || def.isUnsignedInt() || def.isBool())
                {
                    def.physicalIndex = defs.bufferSize * 4;
                    defs.bufferSize += def.arraySize * def.elementSize;
                }
                else if(def.isSampler())
                {
                    def.physicalIndex = defs.registerCount;
                    defs.registerCount += def.arraySize * def.elementSize;
                }
                else
                {
                    LogManager::getSingleton().logMessage("Could not parse type of GLSL Uniform: '"
                                                          + line + "' in file " + filename);
                }
                defs.map.emplace(paramName, def);

                // warn if there is a default value, that we would overwrite
                if (line.find('=') != String::npos)
                {
                    LogManager::getSingleton().logWarning("Default value of uniform '" + paramName +
                                                          "' is ignored in " + filename);
                    break;
                }
            }
        }
    }

    //---------------------------------------------------------------------
    static bool findUniformDataSource(const String& paramName, const GpuConstantDefinitionMap* (&constantDefs)[6],
                               GLUniformReference& refToUpdate)
    {
        for (int i = 0; i < 6; i++)
        {
            if (constantDefs[i])
            {
                auto parami = constantDefs[i]->find(paramName);
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

    bool GLSLProgramManagerCommon::validateParam(String paramName, uint32 numActiveArrayElements,
                                                 const GpuConstantDefinitionMap* (&constantDefs)[6],
                                                 GLUniformReference& refToUpdate)
    {
        // Don't add built in uniforms, atomic counters, or uniform block parameters.
        if (refToUpdate.mLocation < 0)
            return false;

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
            if (paramName.compare(paramName.size() - 3, 3, "[0]") != 0)
                return false;
            paramName.resize(paramName.size() - 3);
        }

        // Find out which params object this comes from
        bool foundSource = findUniformDataSource(paramName, constantDefs, refToUpdate);

        if(!foundSource)
            return false;

        // Note that numActiveArrayElements comes from glGetActiveUniformARB()
        // which returns the index of the highest active array element (that is
        // an element actually used by the shader) plus one. That is, if some array
        // elements are not used by the shader (possibly optimized away), then
        // numActiveArrayElements can well be less than
        // newGLUniformReference.mConstantDef->arraySize.
        // Keep in mind that glUniform*v() faimily of functions does allow specifying
        // a number of elements to update greater than the number of active
        // elements in the shader.
        OgreAssertDbg(numActiveArrayElements <= refToUpdate.mConstantDef->arraySize,
                      "We provide less array elements than what shader actually uses");

        // Only add this parameter if we found the source
        return true;
    }

    void GLSLProgramManagerCommon::extractUniformsFromGLSL(const String& src,
        GpuNamedConstants& defs, const String& filename)
    {
        // Parse the output string and collect all uniforms
        // NOTE this relies on the source already having been preprocessed
        // which is done in GLSLESProgram::loadFromSource
        String line;
        String::size_type currPos = src.find("uniform");
        while (currPos != String::npos)
        {
            // Now check for using the word 'uniform' in a larger string & ignore
            bool inLargerString = false;
            if (currPos != 0)
            {
                char prev = src.at(currPos - 1);
                if (prev != ' ' && prev != '\t' && prev != '\r' && prev != '\n'
                    && prev != ';')
                    inLargerString = true;
            }
            if (!inLargerString && currPos + 7 < src.size())
            {
                char next = src.at(currPos + 7);
                if (next != ' ' && next != '\t' && next != '\r' && next != '\n')
                    inLargerString = true;
            }

            // skip 'uniform'
            currPos += 7;

            if (!inLargerString)
            {
                String::size_type endPos;
                String typeString;

                // Check for a type. If there is one, then the semicolon is missing
                // otherwise treat as if it is a uniform block
                String::size_type lineEndPos = src.find_first_of("\n\r", currPos);
                line = src.substr(currPos, lineEndPos - currPos);
                StringVector parts = StringUtil::split(line, " \t");

                // Skip over precision keywords
                if(StringUtil::match((parts.front()), "lowp") ||
                   StringUtil::match((parts.front()), "mediump") ||
                   StringUtil::match((parts.front()), "highp"))
                    typeString = parts[1];
                else
                    typeString = parts[0];

                StringToEnumMap::iterator typei = mTypeEnumMap.find(typeString);
                if (typei == mTypeEnumMap.end())
                {
                    // Gobble up the external name
                    String externalName = parts.front();

                    // Now there should be an opening brace
                    String::size_type openBracePos = src.find('{', currPos);
                    if (openBracePos != String::npos)
                    {
                        currPos = openBracePos + 1;
                    }
                    else
                    {
                        LogManager::getSingleton().logMessage("Missing opening brace in GLSL Uniform Block in file "
                                                              + filename);
                        break;
                    }

                    // First we need to find the internal name for the uniform block
                    String::size_type endBracePos = src.find('}', currPos);

                    // Find terminating semicolon
                    currPos = endBracePos + 1;
                    endPos = src.find(';', currPos);
                    if (endPos == String::npos)
                    {
                        // problem, missing semicolon, abort
                        break;
                    }
                }
                else
                {
                    // find terminating semicolon
                    endPos = src.find(';', currPos);
                    if (endPos == String::npos)
                    {
                        // problem, missing semicolon, abort
                        break;
                    }

                    parseGLSLUniform(src.substr(currPos, endPos - currPos), defs, filename);
                }
                line = src.substr(currPos, endPos - currPos);
            } // not commented or a larger symbol

            // Find next one
            currPos = src.find("uniform", currPos);
        }
    }
}
