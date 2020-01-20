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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
    namespace RTShader {

        String GLSLESProgramWriter::TargetLanguage =  "glsles";

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::GLSLESProgramWriter()
        {
            mIsGLSLES = true;
            auto* rs = Root::getSingleton().getRenderSystem();
            mGLSLVersion = rs ? rs->getNativeShadingLanguageVersion() : 100;
            initializeStringMaps();
            mFunctionCacheMap.clear();
        }

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::~GLSLESProgramWriter()
        {
        }

        FunctionInvocation * GLSLESProgramWriter::createInvocationFromString(const String & input)
        {
            String functionName, returnType;
            FunctionInvocation *invoc = NULL;

            // Get the function name and return type
            StringVector leftTokens = StringUtil::split(input, "(");
            StringVector leftTokens2 = StringUtil::split(leftTokens[0], " ");
            StringUtil::trim(leftTokens2[0]);
            StringUtil::trim(leftTokens2[1]);
            returnType      = leftTokens2[0];
            functionName    = leftTokens2[1];

            invoc = OGRE_NEW FunctionInvocation(functionName, 0, returnType);

            // Split out the parameters
            StringVector parameters;
            String::size_type lparen_pos = input.find('(', 0);
            if(lparen_pos != String::npos)
            {
                StringVector tokens = StringUtil::split(input, "(");
                parameters = StringUtil::split(tokens[1], ",");
            }
            else
            {
                parameters = StringUtil::split(input, ",");
            }

            StringVector::iterator itParam;
            int i = 0;
            for(itParam = parameters.begin(); itParam != parameters.end(); ++itParam, i++)
            {
                *itParam = StringUtil::replaceAll(*itParam, ")", "");
                StringVector paramTokens = StringUtil::split(*itParam, " ");

                // There should be three parts for each token
                // 1. The operand type(in, out, inout)
                // 2. The type
                // 3. The name
                if(paramTokens.size() == 3)
                {
                    StringUtil::trim(paramTokens[0]);
                    StringUtil::trim(paramTokens[1]);
                    StringUtil::trim(paramTokens[2]);

                    Operand::OpSemantic semantic = Operand::OPS_IN;
                    GpuConstantType gpuType = GCT_UNKNOWN;

                    if(paramTokens[0] == "in")
                    {
                        semantic = Operand::OPS_IN;
                    }
                    else if(paramTokens[0] == "out")
                    {
                        semantic = Operand::OPS_OUT;
                    }
                    else if(paramTokens[0] == "inout")
                    {
                        semantic = Operand::OPS_INOUT;
                    }

                    // Find the internal type based on the string that we're given.
                    GpuConstTypeToStringMapIterator typeMapIterator;
                    for(typeMapIterator = mGpuConstTypeMap.begin(); typeMapIterator != mGpuConstTypeMap.end(); ++typeMapIterator)
                    {
                        if((*typeMapIterator).second == paramTokens[1])
                        {
                            gpuType = (*typeMapIterator).first;
                            break;
                        }
                    }

                    // We need a valid type otherwise glsl compilation will not work
                    if (gpuType == GCT_UNKNOWN)
                    {
                        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                            "Can not convert '"+paramTokens[1]+"' to GpuConstantType",
                            "GLSLESProgramWriter::createInvocationFromString" );    
                    }

                    if(gpuType == GCT_SAMPLER1D)
                        gpuType = GCT_SAMPLER2D;

                    ParameterPtr p = ParameterPtr(OGRE_NEW Parameter(gpuType, paramTokens[2],
                                                                     Parameter::SPS_UNKNOWN, i,
                                                                     Parameter::SPC_UNKNOWN));

                    invoc->pushOperand(p, semantic);
                }
            }

            return invoc;
        }
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeFunctionDeclaration(std::ostream& os, FunctionInvocation& func,
                                                         bool writeParamName)
        {
            os << func.getReturnType() << " " << func.getFunctionName() << "(";

            FunctionInvocation::OperandVector::iterator itOperand    = func.getOperandList().begin();
            FunctionInvocation::OperandVector::iterator itOperandEnd = func.getOperandList().end();
            for (; itOperand != itOperandEnd;)
            {
              const ParameterPtr& param = itOperand->getParameter();
              Operand::OpSemantic opSemantic = itOperand->getSemantic();
              int opMask = itOperand->getMask();
              GpuConstantType gpuType = GCT_UNKNOWN;

              switch(opSemantic)
              {
              case Operand::OPS_IN:
                  os << "in ";
                  break;

              case Operand::OPS_OUT:
                  os << "out ";
                  break;

              case Operand::OPS_INOUT:
                  os << "inout ";
                  break;

              default:
                  break;
              }

              // Swizzle masks are only defined for types like vec2, vec3, vec4.
              if (opMask == Operand::OPM_ALL)
              {
                  gpuType = param->getType();
              }
              else
              {
                  // Now we have to convert the mask to operator
                  gpuType = Operand::getGpuConstantType(opMask);
              }

              // We need a valid type otherwise glsl compilation will not work
              if (gpuType == GCT_UNKNOWN)
              {
                  OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                      "Can not convert Operand::OpMask to GpuConstantType",
                      "GLSLProgramWriter::writeFunctionDeclaration" );
              }

              // Write the operand type.
              os << mGpuConstTypeMap[gpuType];

              if(writeParamName)
                  os << " " << param->getName();

              ++itOperand;
              //move over all operators with indirection
              while ((itOperand != itOperandEnd) && (itOperand->getIndirectionLevel() != 0))
              {
                  ++itOperand;
              }

              // Prepare for the next operand
              if (itOperand != itOperandEnd)
              {
                  os << ", ";
              }
            }
            os << ")";
        }
        
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::discoverFunctionDependencies(const FunctionInvocation &invoc, FunctionVector &depVector)
        {
            // Uses recursion to find any functions that the supplied function invocation depends on
            FunctionMap::const_iterator itCache = mFunctionCacheMap.begin();
            String body;

            // Find the function in the cache and retrieve the body
            for (; itCache != mFunctionCacheMap.end(); ++itCache)
            {
                if(!(invoc == (*itCache).first))
                    continue;

                body = (*itCache).second;
                break;
            }

            if(!body.empty())
            {
                // Trim whitespace
                StringUtil::trim(body);
                StringVector tokens = StringUtil::split(body, "(");

                for (StringVector::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
                {
                    StringVector moreTokens = StringUtil::split(*it, " \n");

                    if (!moreTokens.empty())
                    {
                    FunctionMap::const_iterator itFuncCache = mFunctionCacheMap.begin();

                        for (; itFuncCache != mFunctionCacheMap.end(); ++itFuncCache)
                        {

                            const FunctionInvocation& fi = itFuncCache->first;
                        
                            if(fi.getFunctionName() == moreTokens.back())
                            {
                                // Add the function declaration
                                depVector.push_back(FunctionInvocation((*itFuncCache).first));

                                discoverFunctionDependencies(itFuncCache->first, depVector);
                            }
                        }
                    }
                }
                
            }
            else
            {
                LogManager::getSingleton().logError("Cached function not found " + invoc.getFunctionName());
            }
        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeSourceCode(
            std::ostream& os,
            Program* program)
        {
            // Write the current version (this forces the driver to fulfill the glsl es standard)
            os << "#version "<< mGLSLVersion;

            // Starting with ES 3.0 the version must contain the string "es" after the version number with a space separating them
            if(mGLSLVersion > 100)
                os << " es";

            os << std::endl;

            for(const auto& p : program->getParameters())
            {
                if(p->getType() != GCT_SAMPLER_EXTERNAL_OES)
                    continue;
                if(mGLSLVersion > 100)
                    os << "#extension GL_OES_EGL_image_external_essl3 : require\n";
                else
                    os << "#extension GL_OES_EGL_image_external : require\n";

                break;
            }

            // Default precision declaration is required in fragment and vertex shaders.
            os << "precision highp float;" << std::endl;
            os << "precision highp int;" << std::endl;

            if(mGLSLVersion > 100)
            {
                // sampler3D has no default precision
                os << "precision highp sampler3D;" << std::endl;
                // Redefine texture functions to maintain reusability
                os << "#define texture2D texture" << std::endl;
                os << "#define texture3D texture" << std::endl;
                os << "#define textureCube texture" << std::endl;
                os << "#define texture2DLod textureLod" << std::endl;
            }

            // Generate source code header.
            writeProgramTitle(os, program);
            os<< std::endl;

            // Embed dependencies.
            copyProgramDependencies(os, program);
            os << std::endl;
            writeMainSourceCode(os, program);
        }

        //-----------------------------------------------------------------------
        // Here's the gist of how and what we're doing here.
        // First, identify which fixed function libraries we need.  We already have a list of functions that we need to find.
        // Then for each library file we perform the following steps:
        // 1. Go through the source line by line to find function signatures.
        // 2. Once we have found one, compare it to the list of functions that we are searching for
        // 3. If a match is found then continue reading through the file until we reach the end of the function.
        // 4. When we reach the last closing brace, write the function and its signature to the output stream
        // 5. Go back to step 1 until we have found all the functions
        //
        void GLSLESProgramWriter::copyProgramDependencies(
            std::ostream& os, 
            Program* program)
        {
            for(unsigned int i = 0; i < program->getDependencyCount(); ++i)
            {
                const String& curDependency = program->getDependency(i);
                cacheDependencyFunctions(curDependency);
            }


            os << "//-----------------------------------------------------------------------------" << std::endl;
            os << "//                         PROGRAM DEPENDENCIES"                                 << std::endl;
            os << "//-----------------------------------------------------------------------------" << std::endl;

            FunctionVector forwardDecl; // Holds all function declarations
            const ShaderFunctionList& functionList = program->getFunctions();

            Function* curFunction = *(functionList.begin());
            const FunctionAtomInstanceList& atomInstances = curFunction->getAtomInstances();
            FunctionAtomInstanceConstIterator itAtom = atomInstances.begin();
            FunctionAtomInstanceConstIterator itAtomEnd = atomInstances.end();
            // Now iterate over all function atoms
            for ( ; itAtom != itAtomEnd; ++itAtom)
            {   
                // Skip non function invocation atoms.
                if (!dynamic_cast<const FunctionInvocation*>(*itAtom))
                    continue;

                FunctionInvocation pFuncInvoc = *(static_cast<FunctionInvocation *>(*itAtom));
                forwardDecl.push_back(pFuncInvoc);
                
                // Now look into that function for other non-builtin functions and add them to the declaration list
                // Look for non-builtin functions
                // Do so by assuming that these functions do not have several variations.
                // Also, because GLSL is C based, functions must be defined before they are used
                // so we can make the assumption that we already have this function cached.
                //
                // If we find a function, look it up in the map and write it out
                discoverFunctionDependencies(pFuncInvoc, forwardDecl);
            }

            // Now remove duplicate declarations, first we have to sort the vector.
            std::sort(forwardDecl.begin(), forwardDecl.end(), FunctionInvocation::FunctionInvocationLessThan());
            forwardDecl.erase(std::unique(forwardDecl.begin(), forwardDecl.end(), FunctionInvocation::FunctionInvocationCompare()), forwardDecl.end());

            // Write forward declarations as we did not sort by dependency
            for (auto& decl : forwardDecl)
            {
                writeFunctionDeclaration(os, decl, false);
                os << ";\n";
            }

            for(unsigned int i = 0; i < program->getDependencyCount(); ++i)
            {
                const String& curDependency = program->getDependency(i);

                // Write out #defines
                StringMap::const_iterator itDefines = mDefinesMap.begin();
                StringMap::const_iterator itDefinesEnd = mDefinesMap.end();
                for (; itDefines != itDefinesEnd; ++itDefines)
                {
                    if((*itDefines).second == curDependency)
                    {
                        os << (*itDefines).first;
                        os << "\n";
                    }
                }
            }

            // Parse the source shader and write out only the needed functions
            for (FunctionVector::const_iterator it = forwardDecl.begin(); it != forwardDecl.end(); ++it)
            {
                FunctionMap::const_iterator itCache = mFunctionCacheMap.begin();
                FunctionInvocation invoc = FunctionInvocation("", 0);
                String body;

                // Find the function in the cache
                for (; itCache != mFunctionCacheMap.end(); ++itCache)
                {
                    if(!((*it) == (*itCache).first))
                        continue;

                    invoc = (*itCache).first;
                    body = (*itCache).second;
                    break;
                }

                if(invoc.getFunctionName().length())
                {
                    writeFunctionDeclaration(os, invoc);
                    os << std::endl << "{" << std::endl << body << std::endl << "}" << std::endl;
                }
            }
        }

        bool GLSLESProgramWriter::isBasicType(String &type)
        {
            if(type == "void" ||
               type == "bool" ||
               type == "float" ||
               type == "vec2" ||
               type == "vec3" ||
               type == "vec4" ||
               type == "sampler2D" ||
               type == "samplerCube" ||
               type == "mat2" ||
               type == "mat3" ||
               type == "mat4" ||
               type == "int" ||
               type == "int2" ||
               type == "int3" ||
               type == "int4")
                return true;
            else
                return false;

        }

        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::cacheDependencyFunctions(const String & libName)
        {
            if(mCachedFunctionLibraries.find(libName) != mCachedFunctionLibraries.end())
            {
                // this mean that the lib is in the cache
                return;
            }

            mCachedFunctionLibraries[libName] = "";

            String libFileName =  libName + ".glsl";

            DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(libFileName);

            StringMap functionCache;
            functionCache.clear();

            String line;
            while(!stream->eof())
            {
                // Grab a line
                line = stream->getLine();

                // Ignore empty lines and comments
                if(line.length() > 0)
                {
                    // Strip whitespace
                    StringUtil::trim(line);

                    // If we find a multiline comment, run through till we get to the end 
                    if(line.at(0) == '/' && line.at(1) == '*')
                    {
                        bool endFound = false;

                        while(!endFound)
                        {
                            // Get the next line
                            line = stream->getLine();

                            // Skip empties
                            if(line.length() > 0)
                            {
                                // Look for the ending sequence.
                                String::size_type comment_pos = line.find("*/", 0);
                                if(comment_pos != String::npos)
                                {
                                    endFound = true;
                                }
                            }
                        }
                    }
                    else if(line.length() > 1 && line.at(0) != '/' && line.at(1) != '/')
                    {
                        // Break up the line.
                        StringVector tokens = StringUtil::tokenise(line, " (\n\r");

                        // Cache #defines
                        if(tokens[0] == "#define")
                        {
                            // Add the line in
                            mDefinesMap[line] = libName;

                            // Move on to the next line in the shader
                            continue;
                        }

                        // Try to identify a function definition
                        // First, look for a return type
                        if(isBasicType(tokens[0]) && ((tokens.size() < 3) || (tokens[2] != "=")) )
                        {
                            String functionSig = "";
                            String functionBody = "";
                            FunctionInvocation *functionInvoc = NULL;

                            // Return type
                            functionSig = tokens[0];
                            functionSig += " ";

                            // Function name
                            functionSig += tokens[1];
                            functionSig += "(";

                            bool foundEndOfSignature = false;
                            // Now look for all the parameters, they may span multiple lines
                            while(!foundEndOfSignature)
                            {
                                // Trim whitespace from both sides of the line
                                StringUtil::trim(line);

                                // First we want to get everything right of the paren
                                StringVector paramTokens;
                                String::size_type lparen_pos = line.find('(', 0);
                                if(lparen_pos != String::npos)
                                {
                                    StringVector lineTokens = StringUtil::split(line, "(");
                                    if(lineTokens.size() == 2) {
                                        paramTokens = StringUtil::split(lineTokens[1], ",");
                                    }
                                }
                                else
                                {
                                    paramTokens = StringUtil::split(line, ",");
                                }

                                StringVector::const_iterator itParam;
                                for(itParam = paramTokens.begin(); itParam != paramTokens.end(); ++itParam)
                                {
                                    functionSig += *itParam;

                                    String::size_type rparen_pos = itParam->find(')', 0);
                                    if(rparen_pos == String::npos)
                                        functionSig += ",";
                                }

                                String::size_type space_pos = line.find(')', 0);
                                if(space_pos != String::npos)
                                {
                                    foundEndOfSignature = true;
                                }
                                line = stream->getLine();
                            }

                            functionInvoc = createInvocationFromString(functionSig);

                            // Ok, now if we have found the signature, iterate through the file until we find the end
                            // of the function.
                            bool foundEndOfBody = false;
                            size_t braceCount = 0;
                            while(!foundEndOfBody)
                            {
                                functionBody += line;

                                String::size_type brace_pos = line.find('{', 0);
                                if(brace_pos != String::npos)
                                {
                                    braceCount++;
                                }

                                brace_pos = line.find('}', 0);
                                if(brace_pos != String::npos)
                                    braceCount--;

                                if(braceCount == 0)
                                {
                                    foundEndOfBody = true;

                                    // Remove first and last braces
                                    size_t pos = functionBody.find('{');
                                    functionBody.erase(pos, 1);
                                    pos = functionBody.rfind('}');
                                    functionBody.erase(pos, 1);
                                    mFunctionCacheMap.emplace(*functionInvoc, functionBody);
                                }
                                functionBody += "\n";
                                line = stream->getLine();
                            }
                        }
                    }
                }
            }

            stream->close();
        }

    }
}
