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

#include "OgreShaderHLSLProgramWriter.h"
#include "OgreGpuProgramManager.h"
#include "OgreShaderProgram.h"
#include "OgreShaderGenerator.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace RTShader {

String HLSLProgramWriter::TargetLanguage = "hlsl";

//-----------------------------------------------------------------------
HLSLProgramWriter::HLSLProgramWriter()
{
    initializeStringMaps();
}

//-----------------------------------------------------------------------
HLSLProgramWriter::~HLSLProgramWriter()
{

}

//-----------------------------------------------------------------------
void HLSLProgramWriter::initializeStringMaps()
{
    mGpuConstTypeMap[GCT_SHADER_IN] = "Shader_In";
    mGpuConstTypeMap[GCT_SHADER_IN_GS_TRIANGLE] = "triangle Shader_In";
    mGpuConstTypeMap[GCT_SHADER_OUT] = "Shader_Out";
    mGpuConstTypeMap[GCT_SHADER_OUT_TRIANGLE_STREAM] = "TriangleStream<Shader_Out>";

    mGpuConstTypeMap[GCT_FLOAT1] = "float";
    mGpuConstTypeMap[GCT_FLOAT2] = "float2";
    mGpuConstTypeMap[GCT_FLOAT3] = "float3";
    mGpuConstTypeMap[GCT_FLOAT4] = "float4";
    mGpuConstTypeMap[GCT_SAMPLER1D] = "sampler1D";
    mGpuConstTypeMap[GCT_SAMPLER2D] = "sampler2D";
    mGpuConstTypeMap[GCT_SAMPLER3D] = "sampler3D";
    mGpuConstTypeMap[GCT_SAMPLERCUBE] = "samplerCUBE";
    mGpuConstTypeMap[GCT_SAMPLER_WRAPPER1D]     = "SamplerData1D";
    mGpuConstTypeMap[GCT_SAMPLER_WRAPPER2D]     = "SamplerData2D";
    mGpuConstTypeMap[GCT_SAMPLER_WRAPPER3D]     = "SamplerData3D";
    mGpuConstTypeMap[GCT_SAMPLER_WRAPPERCUBE]   = "SamplerDataCube";
    mGpuConstTypeMap[GCT_SAMPLER_WRAPPER2DARRAY] = "SamplerData2DArray";
    mGpuConstTypeMap[GCT_SAMPLER_STATE]         = "SamplerState";
    
    mGpuConstTypeMap[GCT_MATRIX_2X2] = "float2x2";
    mGpuConstTypeMap[GCT_MATRIX_2X3] = "float2x3";
    mGpuConstTypeMap[GCT_MATRIX_2X4] = "float2x4";
    mGpuConstTypeMap[GCT_MATRIX_3X2] = "float3x2";
    mGpuConstTypeMap[GCT_MATRIX_3X3] = "float3x3";
    mGpuConstTypeMap[GCT_MATRIX_3X4] = "float3x4";
    mGpuConstTypeMap[GCT_MATRIX_4X2] = "float4x2";
    mGpuConstTypeMap[GCT_MATRIX_4X3] = "float4x3";
    mGpuConstTypeMap[GCT_MATRIX_4X4] = "float4x4";
    mGpuConstTypeMap[GCT_INT1] = "int";
    mGpuConstTypeMap[GCT_INT2] = "int2";
    mGpuConstTypeMap[GCT_INT3] = "int3";
    mGpuConstTypeMap[GCT_INT4] = "int4";
    mGpuConstTypeMap[GCT_UINT1] = "uint";
    mGpuConstTypeMap[GCT_UINT2] = "uint2";
    mGpuConstTypeMap[GCT_UINT3] = "uint3";
    mGpuConstTypeMap[GCT_UINT4] = "uint4";
	mGpuConstTypeMap[GCT_SAMPLER2DARRAY] = "Texture2DArray";
	

    mParamSemanticMap[Parameter::SPS_POSITION] = "POSITION";
    mParamSemanticMap[Parameter::SPS_BLEND_WEIGHTS] = "BLENDWEIGHT";
    mParamSemanticMap[Parameter::SPS_BLEND_INDICES] = "BLENDINDICES";
    mParamSemanticMap[Parameter::SPS_NORMAL] = "NORMAL";
    mParamSemanticMap[Parameter::SPS_COLOR] = "COLOR";
    mParamSemanticMap[Parameter::SPS_TEXTURE_COORDINATES] = "TEXCOORD";
    mParamSemanticMap[Parameter::SPS_BINORMAL] = "BINORMAL";
    mParamSemanticMap[Parameter::SPS_TANGENT] = "TANGENT";
    mParamSemanticMap[Parameter::SPS_UNKNOWN] = "";

    mGpuConstTypeMapV4[GCT_SAMPLER1D] = "Texture1D";
    mGpuConstTypeMapV4[GCT_SAMPLER2D] = "Texture2D";
    mGpuConstTypeMapV4[GCT_SAMPLER3D] = "Texture3D";
    mGpuConstTypeMapV4[GCT_SAMPLERCUBE] = "TextureCube";
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeFunction(std::ostream& os, Function* function)
{
    assignFunctionParameterParents(function, false);

    writeFunctionTitle(os, function);

    writeFunctionDeclaration(os, function);

    os << "{" << std::endl;

    // Write local parameters.
    const ShaderParameterList& localParams = function->getLocalParameters();
    ShaderParameterConstIterator itParam = localParams.begin();

    for (; itParam != localParams.end(); ++itParam)
    {
        os << "\t";
        writeLocalParameter(os, *itParam);
        os << ";" << std::endl;
    }
        
    // Sort and write function atoms.
    function->sortAtomInstances();

    const FunctionAtomInstanceList& atomInstances = function->getAtomInstances();
    FunctionAtomInstanceConstIterator itAtom;

    for (itAtom = atomInstances.begin(); itAtom != atomInstances.end(); ++itAtom)
    {
        writeAtomInstance(os, *itAtom);
    }
    os << "}" << std::endl;

    assignFunctionParameterParents(function, true);
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeSourceCode(std::ostream& os, Program* program)
{
    const ShaderFunctionList& unsortedfunctionList = program->getFunctions();
    ShaderFunctionList functionList;
    ShaderFunctionConstIterator itFunction;

    const UniformParameterList& parameterList = program->getParameters();
    UniformParameterConstIterator itUniformParam = parameterList.begin();

    // Generate source code header.
    writeProgramTitle(os, program);
    os << std::endl;

    // Generate dependencies.
    writeProgramDependencies(os, program);
    os << std::endl;

    // Generate global variable code.
    writeUniformParametersTitle(os, program);
    os << std::endl;

    ParameterNameMetrics metrics;
    getParamsNameMetrics(parameterList, metrics);
    for (itUniformParam=parameterList.begin();  itUniformParam != parameterList.end(); ++itUniformParam)
    {
    
        writeUniformParameter(os, *itUniformParam, &metrics);
        os << ";" << std::endl;             
    }
    os << std::endl;
    
    // Perform insertion sort on functions, making internal functions appear first
    for (itFunction = unsortedfunctionList.begin(); itFunction != unsortedfunctionList.end(); ++itFunction)
    {
        if ((*itFunction)->getFunctionType() != Function::FFT_INTERNAL)
            functionList.push_back(*itFunction);
        else
            functionList.insert(functionList.begin(), *itFunction);
    }
    
    //Write program In and outs shaders
    if (!functionList.empty())
        writeShaderInAndOutStucts(os, *(functionList.end() - 1));
        

    // Write program function(s).
    for (itFunction = functionList.begin(); itFunction != functionList.end(); ++itFunction)
    {
        
        writeFunction(os, *itFunction);
    }

    os << std::endl;
}
//-----------------------------------------------------------------------
void HLSLProgramWriter::assignFunctionParameterParents(Function* function, bool clear)
{
    if (function == NULL)
        return;

    const ShaderParameterList& inParams = function->getInputParameters();
    const ShaderParameterList& outParams = function->getOutputParameters();
    ParameterPtr structIn = function->getParameterByContent(Parameter::SPC_SHADER_IN, Parameter::SPD_IN);
    ParameterPtr structOut = function->getParameterByContent(Parameter::SPC_SHADER_OUT, Parameter::SPD_OUT);

    if (!structIn.isNull() && !structOut.isNull())
    {
        const String shader_in_name = structIn->getName();
        const String shader_out_name = structOut->getName();
        setParentParameterName(inParams, clear ? BLANKSTRING : shader_in_name);
        setParentParameterName(outParams, clear ? BLANKSTRING : shader_out_name);
    }
}

void HLSLProgramWriter::setParentParameterName(const ShaderParameterList& params, String parentName)
{
    ShaderParameterConstIterator it;
    for (it = params.begin(); it != params.end(); ++it)
    {
        (*it)->setParenttName(parentName);
    }
}
//-----------------------------------------------------------------------
void HLSLProgramWriter::clearMainfunctionParentParameters(ShaderFunctionList& functionList)
{
    ShaderFunctionConstIterator itFunction;
    for (itFunction = functionList.begin(); itFunction != functionList.end(); ++itFunction)
    {
        Function* function = (*itFunction);
        Function::FunctionType functionType = function->getFunctionType();
        bool isMainFunction =
            (functionType == Function::FFT_GS_MAIN
            || functionType == Function::FFT_PS_MAIN
            || functionType == Function::FFT_VS_MAIN);

        if (isMainFunction)
        {
            assignFunctionParameterParents(function, true);
        }
    }
}


//-----------------------------------------------------------------------
void HLSLProgramWriter::writeProgramDependencies(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         PROGRAM DEPENDENCIES" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;


    for (unsigned int i=0; i < program->getDependencyCount(); ++i)
    {
        const String& curDependency = program->getDependency(i);

        os << "#include " << '\"' << curDependency << "." << getTargetLanguage() << '\"' << std::endl;
    }
}


//-----------------------------------------------------------------------
void HLSLProgramWriter::writeUniformParameter(std::ostream& os, UniformParameterPtr parameter, ParameterNameMetrics* metrics)
{
    const int extraPadding = 5;
    bool isHlsl4 = Ogre::RTShader::ShaderGenerator::getSingletonPtr()->IsHlsl4();

    GpuConstantType paramType = parameter->getType();
    String uniformName = parameter->getName();
    String uniformType = getParameterTypeName(paramType);
    
    os << uniformType;
    
    if (metrics != NULL)
        writePadded(uniformType, metrics->MaxParameterTypeLength + extraPadding, os);
    else
        os << "\t";

    os << uniformName;

    writeArrayParameterBrackets(os, parameter);
    
    if (parameter->isSampler())
    {
        if (metrics != NULL)
            writePadded(uniformName, metrics->MaxParameterLength + extraPadding, os);

        if (isHlsl4)
            os << " : register(t" << parameter->getIndex() << ")";      
        else
            os << " : register(s" << parameter->getIndex() << ")";      

    }
    else if (parameter->getType() == GCT_SAMPLER_STATE)
    {
        if (metrics != NULL)
            writePadded(uniformName, metrics->MaxParameterLength + extraPadding, os);

        os << " : register(s" << parameter->getIndex() << ")";      
    }

}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeFunctionParameter(std::ostream& os, ParameterPtr parameter, const char* forcedSemantic, ParameterNameMetrics* nameMetrics)
{
    const int extraPadding = 5;

    String gpuTypeName = mGpuConstTypeMap[parameter->getType()];
    os << gpuTypeName;
    if (nameMetrics != NULL)
        writePadded(gpuTypeName, nameMetrics->MaxParameterTypeLength + extraPadding, os);
    else
        os << " ";
    

    os << parameter->getName();
    writeArrayParameterBrackets(os, parameter);
    if (nameMetrics != NULL)
        writePadded(parameter->getName(), nameMetrics->MaxParameterLength + extraPadding, os);


    if(forcedSemantic)
    {
        
        os << " : " << forcedSemantic;
    }
    else if (parameter->getSemantic() != Parameter::SPS_UNKNOWN && parameter->getSemantic() != Parameter::SPS_CUSTOM)
    {
        os << " : ";
        
        os << mParamSemanticMap[parameter->getSemantic()];

        if (parameter->getSemantic() != Parameter::SPS_POSITION && 
            parameter->getSemantic() != Parameter::SPS_NORMAL &&
            parameter->getSemantic() != Parameter::SPS_BLEND_INDICES &&
            parameter->getSemantic() != Parameter::SPS_BLEND_WEIGHTS &&
            (!(parameter->getSemantic() == Parameter::SPS_COLOR && parameter->getIndex() == 0)) &&
            parameter->getIndex() >= 0)
        {           
            os << StringConverter::toString(parameter->getIndex()).c_str();
        }
    }
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeArrayParameterBrackets(std::ostream& os, ParameterPtr parameter)
{
    if (parameter->isArray() == true)
    {
        for (int i = 0; i < Parameter::RTSHADER_PARAMETER_MAX_ARRAY_DIMENSIONS; i++)
        {
            if (parameter->getSize(i) > 0)
            {
                os << "[" << parameter->getSize(i) << "]";
            }
            else
            {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeLocalParameter(std::ostream& os, ParameterPtr parameter)
{
    os << mGpuConstTypeMap[parameter->getType()];
    os << "\t"; 
    os << parameter->getName();     
    writeArrayParameterBrackets(os, parameter);
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeShaderInAndOutStucts(std::ostream& os, Function* function)
{
    
    const ShaderParameterList& inParams = function->getInputParameters();
    const ShaderParameterList& outParams = function->getOutputParameters();
    Function::FunctionType functionType = function->getFunctionType();
    bool isMainGS = functionType == Function::FFT_GS_MAIN;
    bool isMainPS = functionType == Function::FFT_PS_MAIN;
    bool isMainVS = functionType == Function::FFT_VS_MAIN;

    bool isMainFunction = isMainVS || isMainGS || isMainPS;
    bool isVs4 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");

    ParameterNameMetrics metrics;
    getParamsNameMetrics(inParams, metrics);

    //Build input struct
    ShaderParameterConstIterator it;
    writeShaderInTitle(os);
    os << "struct Shader_In" << "\n{\n";

    for (it = inParams.begin(); it != inParams.end(); ++it)
    {
        if ((*it)->isShaderStruct())
            continue;

        const char* forcedSemantic =
            (isVs4 && (isMainPS || isMainGS) && (*it)->getSemantic() == Parameter::SPS_POSITION) ? "SV_Position" : NULL;

        os << "\t";
        //We use the function 'writeFunctionParameter' to write a 'struct parameter', it's the same functionality.
        //TODO: change 'writeFunctionParameter' to 'writeFunctionOrStructParameter'
        writeFunctionParameter(os, *it, forcedSemantic, &metrics);
        
        //add prefix of the struct to the input parameter



        os << ";\n";
    }

    getParamsNameMetrics(outParams, metrics);
    os << "};\n";
    writeShaderOutTitle(os);
    os << "struct Shader_Out" << "\n{\n";
    //Build output struct
    for (it = outParams.begin(); it != outParams.end(); ++it)
    {
        if ((*it)->isShaderStruct())
            continue;

        const char* forcedSemantic =
            (isVs4 && isMainPS) ? "SV_Target" :
            (isVs4 && (isMainVS || isMainGS) && (*it)->getSemantic() == Parameter::SPS_POSITION) ? "SV_Position" : NULL;
        os << "\t";
        writeFunctionParameter(os, *it, forcedSemantic, &metrics);

        os << ";\n";
    }
    os << "};\n\n";
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeFunctionDeclaration(std::ostream& os, Function* function)
{
    const ShaderParameterList& inParams = function->getInputParameters();
    const ShaderParameterList& outParams = function->getOutputParameters();

    Function::FunctionType functionType = function->getFunctionType();

    bool isMainGS = functionType == Function::FFT_GS_MAIN;
    bool isMainPS = functionType == Function::FFT_PS_MAIN;
    bool isMainVS = functionType == Function::FFT_VS_MAIN;

    bool isMainFunction = isMainVS || isMainGS || isMainPS;
    bool isVs4 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");
    bool isGeometryShader = function->getProgramType() == GPT_GEOMETRY_PROGRAM;

    bool isShaderInAvailable = !function->getParameterByContent(Parameter::SPC_SHADER_IN, Parameter::SPD_IN).isNull();
    bool isShaderOutAvailable = !function->getParameterByContent(Parameter::SPC_SHADER_OUT, Parameter::SPD_OUT).isNull();
    


    if (isMainFunction)
    {
        if (isMainGS)
        {
            ParameterPtr structIn = function->getParameterByContent(Parameter::SPC_SHADER_IN, Parameter::SPD_IN);
            int verticesPerTriangle = structIn->getSize();
            int totalVertices = 3;
            os << "[maxvertexcount(" << totalVertices << ")]" << std::endl;
            
        }
        os << "void " << function->getName() << "(";
    }
    else
    {
        os << "void";
        os << " ";

        os << function->getName();
        os << std::endl << "\t(" << std::endl;
    }

    ShaderParameterConstIterator it;
    size_t paramsCount = inParams.size() + outParams.size();
    size_t curParamIndex = 0;
    {
        bool firstParamFlag = false;
        // Write input parameters.
        for (it = inParams.begin(); it != inParams.end(); ++it)
        {
            ParameterPtr parameter = *it;
            if (isShaderInAvailable && parameter->isValidSemantic())
                continue;

            if (firstParamFlag)
                os << ", " << std::endl;

            firstParamFlag = true;

            if (isGeometryShader && parameter->getContent() == Parameter::SPC_SHADER_OUT)
                os << "\t inout ";
            else
                os << "\t in ";

            writeFunctionParameter(os, *it, NULL);
        }
         
        // Write output parameters.
        for (it = outParams.begin(); it != outParams.end(); ++it)
        {
            
            ParameterPtr parameter = *it;

            if (isShaderOutAvailable && parameter->isValidSemantic())
                continue;

            if (firstParamFlag)
                os << ", " << std::endl;

            firstParamFlag = true;
            if (isGeometryShader && parameter->getContent() == Parameter::SPC_SHADER_OUT)
                os << "\t inout ";
            else
                os << "\t out ";

            writeFunctionParameter(os, *it, NULL);
            
        }
        os << std::endl << "\t)" << std::endl;
    }
    
}

//-----------------------------------------------------------------------
void HLSLProgramWriter::writeAtomInstance(std::ostream& os, FunctionAtom* atom)
{
    os << std::endl << "\t";
    atom->writeSourceCode(os, getTargetLanguage());
    os << std::endl;
}
//-----------------------------------------------------------------------
void HLSLProgramWriter::writePadded(String nonPaddedString, const int maxParameterTypeLength, std::ostream& os)
{
    int paddingLength = maxParameterTypeLength - nonPaddedString.length();
    if (paddingLength < 0)
        paddingLength = 0;

    for (int i = 0; i < paddingLength; i++)
        os << " ";
}
//-----------------------------------------------------------------------
void HLSLProgramWriter::getParamsNameMetrics(const ShaderParameterList& inParams, ParameterNameMetrics& metrics)
{
    metrics.MaxParameterLength = 0;
    metrics.MaxParameterTypeLength = 0;
    for (ShaderParameterList::const_iterator it = inParams.begin(); it != inParams.end(); ++it)
    {
        ParameterPtr param = *it;
        if (param->isValidSemantic())
        {
            int paramNameLength = param->getName().length();
            int paramTypeNameLength = strlen(mGpuConstTypeMap[param->getType()]);

            if (metrics.MaxParameterLength < paramNameLength)
                metrics.MaxParameterLength = paramNameLength;


            if (metrics.MaxParameterTypeLength < paramTypeNameLength)
                metrics.MaxParameterTypeLength = paramTypeNameLength;
        }
    }
}
//-----------------------------------------------------------------------
void HLSLProgramWriter::getParamsNameMetrics(const UniformParameterList& inParams, ParameterNameMetrics& metrics)
{
    metrics.MaxParameterLength = 0;
    metrics.MaxParameterTypeLength = 0;
    for (UniformParameterConstIterator it = inParams.begin(); it != inParams.end(); ++it)
    {
        ParameterPtr param = *it;

        int paramNameLength = param->getName().length();
        int paramTypeNameLength = strlen(getParameterTypeName (param->getType()));

        
        int numArrayChars = 0;
        int dim = 0;
        int size = 0;
        if (param->isArray() == true)
        {
            numArrayChars += 2;
            while (dim < Parameter::RTSHADER_PARAMETER_MAX_ARRAY_DIMENSIONS && (size = param->getSize(dim)) > 0)
            {
                while (size > 0)
                {
                    numArrayChars++;
                    size /= 10;
                }
                dim++;
            }
        }

        paramNameLength += numArrayChars;

        if (metrics.MaxParameterLength < paramNameLength)
            metrics.MaxParameterLength = paramNameLength;


        if (metrics.MaxParameterTypeLength < paramTypeNameLength)
            metrics.MaxParameterTypeLength = paramTypeNameLength;

    }
}

const char* HLSLProgramWriter::getParameterTypeName(GpuConstantType paramType)
{
    if (Ogre::RTShader::ShaderGenerator::getSingletonPtr()->IsHlsl4() && paramType >= GCT_SAMPLER1D && paramType <= GCT_SAMPLERCUBE)
        return mGpuConstTypeMapV4[paramType];
    else
        return mGpuConstTypeMap[paramType];
    
}
/** @} */
/** @} */
}
}
