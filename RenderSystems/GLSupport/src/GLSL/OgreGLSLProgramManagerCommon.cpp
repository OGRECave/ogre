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

    GLSLProgramManagerCommon::~GLSLProgramManagerCommon()
    {
        // iterate through map container and delete link programs
        for (ProgramIterator currentProgram = mPrograms.begin();
             currentProgram != mPrograms.end(); ++currentProgram)
        {
            delete currentProgram->second;
        }
    }

    void GLSLProgramManagerCommon::destroyAllByShader(GLSLShaderCommon* shader)
    {
        std::vector<uint32> keysToErase;
        for (ProgramIterator currentProgram = mPrograms.begin();
            currentProgram != mPrograms.end(); ++currentProgram)
        {
            GLSLProgramCommon* prgm = currentProgram->second;
            if(prgm->isUsingShader(shader))
            {
                OGRE_DELETE prgm;
                keysToErase.push_back(currentProgram->first);
            }
        }

        for(size_t i = 0; i < keysToErase.size(); ++i)
        {
            mPrograms.erase(mPrograms.find(keysToErase[i]));
        }
    }

    void GLSLProgramManagerCommon::parseGLSLUniform(
        String line, GpuNamedConstants& defs,
        const String& filename, const GpuSharedParametersPtr& sharedParams)
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

        for (StringVector::iterator i = parts.begin(); i != parts.end(); ++i)
        {
            // Is this a type?
            StringToEnumMap::iterator typei = mTypeEnumMap.find(*i);
            if (typei != mTypeEnumMap.end())
            {
                convertGLUniformtoOgreType(typei->second, def);
            }
            else
            {
                // if this is not a type, and not empty, it should be a name
                StringUtil::trim(*i);
                if (i->empty()) continue;

                // Skip over precision keywords
                if(StringUtil::match((*i), "lowp") ||
                   StringUtil::match((*i), "mediump") ||
                   StringUtil::match((*i), "highp"))
                    continue;

                String::size_type arrayStart = i->find("[", 0);
                if (arrayStart != String::npos)
                {
                    // potential name (if butted up to array)
                    String name = i->substr(0, arrayStart);
                    StringUtil::trim(name);
                    if (!name.empty())
                        paramName = name;

                    def.arraySize = 1;

                    // N-dimensional arrays
                    while (arrayStart != String::npos) {
                        String::size_type arrayEnd = i->find("]", arrayStart);
                        String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                        StringUtil::trim(arrayDimTerm);
                        //TODO
                        // the array term might be a simple number or it might be
                        // an expression (e.g. 24*3) or refer to a constant expression
                        // we'd have to evaluate the expression which could get nasty
                        def.arraySize *= StringConverter::parseInt(arrayDimTerm);
                        arrayStart = i->find("[", arrayEnd);
                    }
                }
                else
                {
                    paramName = *i;
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

                // Special handling for shared parameters
                if(!sharedParams)
                {
                    // Complete def and add
                    // increment physical buffer location
                    def.logicalIndex = 0; // not valid in GLSL
                    if (def.isFloat())
                    {
                        def.physicalIndex = defs.floatBufferSize;
                        defs.floatBufferSize += def.arraySize * def.elementSize;
                    }
                    else if (def.isDouble())
                    {
                        def.physicalIndex = defs.doubleBufferSize;
                        defs.doubleBufferSize += def.arraySize * def.elementSize;
                    }
                    else if (def.isInt() || def.isSampler() || def.isUnsignedInt() || def.isBool())
                    {
                        def.physicalIndex = defs.intBufferSize;
                        defs.intBufferSize += def.arraySize * def.elementSize;
                    }
                    else
                    {
                        LogManager::getSingleton().logMessage("Could not parse type of GLSL Uniform: '"
                                                              + line + "' in file " + filename);
                    }
                    defs.map.emplace(paramName, def);

                    // Generate array accessors
                    defs.generateConstantDefinitionArrayEntries(paramName, def);
                }
                else
                {
                    const GpuConstantDefinitionMap& map = sharedParams->getConstantDefinitions().map;

                    if(map.find(paramName) == map.end()) {
                        // This constant doesn't exist so we'll create a new one
                        sharedParams->addConstantDefinition(paramName, def.constType);
                    }
                }
            }
        }
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
                GpuSharedParametersPtr blockSharedParams;

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

                    parseGLSLUniform(src.substr(currPos, endPos - currPos), defs, filename, blockSharedParams);
                }
                line = src.substr(currPos, endPos - currPos);
            } // not commented or a larger symbol

            // Find next one
            currPos = src.find("uniform", currPos);
        }
    }
}
