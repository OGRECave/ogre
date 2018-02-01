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
#include "OgreShaderGLSLProgramWriter.h"
#include "OgreShaderParameter.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

    class Function;
    class FunctionInvocation;
    class Operand;
    class Program;

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** GLSL ES target language writer implementation.
@see ProgramWriter.
*/
class GLSLESProgramWriter : public GLSLProgramWriter
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
    typedef     std::map<FunctionInvocation, String>       FunctionMap;
    typedef     std::vector<FunctionInvocation>            FunctionVector;
    typedef     FunctionMap::const_iterator                 FunctionMapIterator;
    typedef     GpuConstTypeToStringMap::const_iterator     GpuConstTypeToStringMapIterator;

    // Protected methods.
protected:

    /** Cache functions of a dependency */
    virtual void        cacheDependencyFunctions(const String & libName);


    /** Create a FunctionInvocation object from a string taken out of a shader library. */
    FunctionInvocation  *createInvocationFromString (const String & input);

    /** Write the program dependencies. */
    void                writeProgramDependencies    (std::ostream& os, Program* program);

    String processOperand(Operand op, GpuProgramType gpuType);
    
    /** Check if a string matches one of the GLSL ES basic types */
    bool                isBasicType(String &type);
    
    /** Search within a function body for non-builtin functions that a given function invocation depends on. */
    void                discoverFunctionDependencies(const FunctionInvocation &invoc, FunctionVector &depVector);

    // Attributes.
protected:

    FunctionMap                 mFunctionCacheMap;              // Map function invocation to body.  Used as a cache to reduce library file reads and for inlining
    StringMap                   mDefinesMap;                    // Map of #defines and the function library that contains them

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
