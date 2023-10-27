/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _ShaderProgramWriterGLSL_
#define _ShaderProgramWriterGLSL_

#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderProgramWriter.h"
#include "OgreShaderParameter.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

    class Function;
    class Program;

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** GLSL target language writer implementation.
@see ProgramWriter.
*/
class GLSLProgramWriter : public ProgramWriter
{
    // Interface.
public:

    /** Class constructor. 
    */
    GLSLProgramWriter();

    /** Class destructor */
    virtual ~GLSLProgramWriter();


    /** 
    @see ProgramWriter::writeSourceCode.
    */
    void writeSourceCode(std::ostream& os, Program* program) override;

    /** 
    @see ProgramWriter::getTargetLanguage.
    */
    const String& getTargetLanguage() const override { return TargetLanguage; }

    static String TargetLanguage;

    // Protected methods.
protected:
    void writeMainSourceCode(std::ostream& os, Program* program);

    /** Initialize string maps. */
    void initializeStringMaps();

    /** Write the input params of the function */
    void writeInputParameters(std::ostream& os, Function* function, GpuProgramType gpuType);
    
    /** Write the output params of the function */
    void writeOutParameters(std::ostream& os, Function* function, GpuProgramType gpuType);

    void writeUniformBlock(std::ostream& os, const String& name, int binding, const UniformParameterList& uniforms);

protected:
    typedef std::map<Parameter::Semantic, const char*>  ParamSemanticToStringMap;

    // Map parameter content to vertex attributes 
    ParamSemanticToStringMap mParamSemanticToNameMap;
    // Holds the current glsl version
    int mGLSLVersion;
    // set by derived class
    bool mIsGLSLES;
    bool mIsVulkan;
};
/** @} */
/** @} */

}
}

#endif
