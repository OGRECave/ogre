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
#ifndef _ShaderProgramWriterGLSLES_
#define _ShaderProgramWriterGLSLES_

#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderProgramWriter.h"
#include "OgreShaderParameter.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

    class Function;
    class FunctionInvocation;
    class Operand;
    class Program;

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** GLSL ES target language writer implementation.
@see ProgramWriter.
*/
class GLSLESProgramWriter : public ProgramWriter
{
    // Interface.
public:

    /** Class constructor. 
    */
    GLSLESProgramWriter ();

    /** Class destructor */
    virtual ~GLSLESProgramWriter    ();


    /** 
    @see ProgramWriter::writeSourceCode.
    */
    virtual void            writeSourceCode         (std::ostream& os, Program* program);

    /** 
    @see ProgramWriter::getTargetLanguage.
    */
    virtual const String&   getTargetLanguage       () const { return TargetLanguage; }

    static String TargetLanguage;

    protected:
    typedef     map<GpuConstantType, const char*>::type     GpuConstTypeToStringMap;
    typedef     map<Parameter::Semantic, const char*>::type ParamSemanticToStringMap;
    typedef     map<Parameter::Content, const char*>    ::type  ParamContentToStringMap;
    typedef     map<String, String>::type                   StringMap;
    typedef     map<FunctionInvocation, String>::type       FunctionMap;
    typedef     vector<FunctionInvocation>::type            FunctionVector;
    typedef     FunctionMap::const_iterator                 FunctionMapIterator;
    typedef     FunctionVector::const_iterator              FunctionVectorIterator;
    typedef     GpuConstTypeToStringMap::const_iterator     GpuConstTypeToStringMapIterator;

    // Protected methods.
protected:

    /** Initialize string maps. */
    void                initializeStringMaps        ();

    /** Cache functions of a dependency */
    virtual void        cacheDependencyFunctions(const String & libName);


    /** Create a FunctionInvocation object from a string taken out of a shader library. */
    FunctionInvocation  *createInvocationFromString (const String & input);

    /** Write the program dependencies. */
    void                writeProgramDependencies    (std::ostream& os, Program* program);

    /** Write a local parameter. */
    void                writeLocalParameter         (std::ostream& os, ParameterPtr parameter);

    /** Write the input params of the function */
    void                writeInputParameters        (std::ostream& os, Function* function, GpuProgramType gpuType);
    
    /** Write the output params of the function */
    void                writeOutParameters          (std::ostream& os, Function* function, GpuProgramType gpuType);

    String processOperand(Operand op, GpuProgramType gpuType);
    
    /** Check if a string matches one of the GLSL ES basic types */
    bool                isBasicType(String &type);
    
    /** Search within a function body for non-builtin functions that a given function invocation depends on. */
    void                discoverFunctionDependencies(const FunctionInvocation &invoc, FunctionVector &depVector);

    // Attributes.
protected:
    GpuConstTypeToStringMap     mGpuConstTypeMap;               // Map between GPU constant type to string value.
    ParamSemanticToStringMap    mParamSemanticMap;              // Map between parameter semantic to string value.

    StringMap                   mInputToGLStatesMap;            // Map parameter name to a new parameter name (sometimes renaming is required to match names between vertex and fragment shader)
    FunctionMap                 mFunctionCacheMap;              // Map function invocation to body.  Used as a cache to reduce library file reads and for inlining
    StringMap                   mDefinesMap;                    // Map of #defines and the function library that contains them
    ParamContentToStringMap     mContentToPerVertexAttributes;  // Map parameter content to vertex attributes
    int                         mGLSLVersion;                   // Holds the current glsl es version
    StringVector                mFragInputParams;               // Holds the fragment input params
    StringMap                   mCachedFunctionLibraries;       // Holds the cached function libraries
};

/** GLSL ES program writer factory implementation.
@see ProgramWriterFactory
*/
class ShaderProgramWriterGLSLESFactory : public ProgramWriterFactory
{
public:
    ShaderProgramWriterGLSLESFactory() : mLanguage("glsles")
    {
    }
    virtual ~ShaderProgramWriterGLSLESFactory() {}

    /** 
    @see ProgramWriterFactory::getTargetLanguage
    */
    virtual const String& getTargetLanguage(void) const
    {
        return mLanguage;
    }

    /** 
    @see ProgramWriterFactory::create
    */
    virtual ProgramWriter* create(void)
    {
        return OGRE_NEW GLSLESProgramWriter();
    }

private:
    String mLanguage;
};

/** @} */
/** @} */

}
}

#endif
