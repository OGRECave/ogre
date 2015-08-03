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
#ifndef __ShaderProgramWriterHLSL_H__
#define __ShaderProgramWriterHLSL_H__

#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderProgramWriter.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"

namespace Ogre {
    namespace RTShader {

        class Function;
        class FunctionAtom;
        class Program;

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** HLSL target language writer implementation.
@see ProgramWriter.
*/
class _OgreRTSSExport HLSLProgramWriter : public ProgramWriter
{

    // Interface.
public:

    /** Class constructor. 
    */
    HLSLProgramWriter();

    /** Class destructor */
    virtual ~HLSLProgramWriter();

    /** 
    @see ProgramWriter::writeSourceCode.
    */
    virtual void writeSourceCode(std::ostream& os, Program* program);
    
    /** Writes a function to the output stream
    @param os The output stream to write to.
    @param function The function to populate.
    @param useInstanceViewport determines whether to add an index parameter for the uniform parameters to get the correct
    "Instance viewport slice" gpu parameters
    */
    void writeFunction(std::ostream& os, Function* function);

    /** 
    @see ProgramWriter::getTargetLanguage.
    */
    virtual const String& getTargetLanguage() const { return TargetLanguage; }

    static String TargetLanguage;

    // Protected methods.
protected:

    /** Initialize string maps. */
    void initializeStringMaps();

    /** Write the program dependencies. */
    void writeProgramDependencies(std::ostream& os, Program* program);

    /** Write a uniform parameter. */
    void writeUniformParameter(std::ostream& os, UniformParameterPtr parameter);

    /** Write a function parameter. */
    void writeFunctionParameter(std::ostream& os, ParameterPtr parameter, const char* forcedSemantic);

    /** Write a local parameter. */
    void writeLocalParameter(std::ostream& os, ParameterPtr parameter);
    
    /** Writes open and closed square bracket for an array parameter
    @param os The output stream to write to.
    @param parameter The parameter to process
    */
    void writeArrayParameterBrackets(std::ostream& os, ParameterPtr parameter);

    /** Write a function declaration. */
    void writeFunctionDeclaration(std::ostream& os, Function* function);

    /** Writes Shader stage input and shader stage output structs for the GPU program
    @param os The output stream to write to.
    @param function The function to process
    */
    void writeShaderInAndOutStucts(std::ostream& os, Function* function);

    /** Write function atom instance. */
    void writeAtomInstance(std::ostream& os, FunctionAtom* atom);   
    /** Assign/Clears parent input/output stage struct for the parameters used in 'function'.
    @param function The function to process.
    @param clear Determines whether to clear or to assign parent struct.
    */
    void assignFunctionParameterParents(Function* function,bool clear);
    
    /** Clears parent input/output stage struct from any shader stage main's function.
    @param funtcionsList The functions list from which to find a main function.
    */
    void clearMainfunctionParentParameters(ShaderFunctionList& funtcionsList);

    /** Sets a parent struct name for a list of shader parameter list.
    @param ShaderParameterList The parameters list to modify.
    @parentName The name of the parent struct name.
    */
    void setParentParameterName(const ShaderParameterList& params, String parentName);
    
    /** Assign indices to uniform parameters when using instance viewport.
    @param os The output stream to write to.
    @param function The function to process .
    @param indexParameter The parameter used to index an array.
    */
    void assignUniformIndices(std::ostream& os, Function* function, ParameterPtr indexParameter);

    /** Get the correct shader profile GPU parameter type name.
    @param GpuConstantType The type of the desired parameter.
    Return a string representing the parameter type.
    */
    const char* getParameterTypeName(GpuConstantType paramType);

protected:
    typedef map<GpuConstantType, const char*>::type     GpuConstTypeToStringMap;
    typedef map<Parameter::Semantic, const char*>::type ParamSemanticToStringMap;

    // Attributes.
protected:
    // Map between GPU constant type to string value.
    GpuConstTypeToStringMap mGpuConstTypeMap;
    // Map between GPU constant type v4 to string value.
    //TODO : add abstraction per version
    GpuConstTypeToStringMap mGpuConstTypeMapV4;
    // Map between parameter semantic to string value.
    ParamSemanticToStringMap mParamSemanticMap;
};

/** HLSL program writer factory implementation.
@see ProgramWriterFactory
*/
class _OgreRTSSExport ShaderProgramWriterHLSLFactory : public ProgramWriterFactory
{
public:
    ShaderProgramWriterHLSLFactory() : mLanguage("hlsl") {}
    virtual ~ShaderProgramWriterHLSLFactory() {}

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
        return OGRE_NEW HLSLProgramWriter();
    }

private:
    String mLanguage;

};


/** @} */
/** @} */
}
}

#endif
