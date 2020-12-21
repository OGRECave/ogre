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
#include "OgreD3D11HLSLProgram.h"
#include "OgreException.h"
#include "OgreRenderSystem.h"
#include "OgreD3D11Device.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D11Mappings.h"
#include "OgreGpuProgramManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreStringConverter.h"

#define  HLSL_PROGRAM_DEFINE_VS "HLSL_VS"
#define  HLSL_PROGRAM_DEFINE_PS "HLSL_PS"
#define  HLSL_PROGRAM_DEFINE_GS "HLSL_GS"
#define  HLSL_PROGRAM_DEFINE_HS "HLSL_HS"
#define  HLSL_PROGRAM_DEFINE_CS "HLSL_CS"
#define  HLSL_PROGRAM_DEFINE_DS "HLSL_DS"
namespace Ogre {
    //-----------------------------------------------------------------------
    D3D11HLSLProgram::CmdEntryPoint D3D11HLSLProgram::msCmdEntryPoint;
    D3D11HLSLProgram::CmdTarget D3D11HLSLProgram::msCmdTarget;
    D3D11HLSLProgram::CmdColumnMajorMatrices D3D11HLSLProgram::msCmdColumnMajorMatrices;
    D3D11HLSLProgram::CmdEnableBackwardsCompatibility D3D11HLSLProgram::msCmdEnableBackwardsCompatibility;
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::notifyDeviceLost(D3D11Device* device)
    {
        if(mHighLevelLoaded)
            unloadHighLevelImpl();
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::notifyDeviceRestored(D3D11Device* device)
    {
        if(mHighLevelLoaded)
            loadFromSource();
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::createConstantBuffer(const UINT ByteWidth)
    {

        // Create a constant buffer
        D3D11_BUFFER_DESC cbDesc;
        cbDesc.ByteWidth = ByteWidth;
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        OGRE_CHECK_DX_ERROR(mDevice->CreateBuffer(&cbDesc, NULL, mConstantBuffer.ReleaseAndGetAddressOf()));
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::fixVariableNameFromCg(const ShaderVarWithPosInBuf& newVar)
    {
        String varForSearch = String(" :  : ") + newVar.name;
        size_t startPosOfVarOrgNameInSource = 0;
        size_t endPosOfVarOrgNameInSource = mSource.find(varForSearch + " ");
        if(endPosOfVarOrgNameInSource == -1)
        {
            endPosOfVarOrgNameInSource = mSource.find(varForSearch + "[");
        }
        if(endPosOfVarOrgNameInSource != -1)
        {
            // find space before var;
            for (size_t i = endPosOfVarOrgNameInSource - 1 ; i > 0 ; i-- )
            {
                if (mSource[i] == ' ')
                {
                    startPosOfVarOrgNameInSource = i + 1;
                    break;
                }
            }
            if (startPosOfVarOrgNameInSource > 0)
            {
                newVar.name = mSource.substr(startPosOfVarOrgNameInSource, endPosOfVarOrgNameInSource - startPosOfVarOrgNameInSource);
            }
        }

        // hack for cg parameter with strange prefix
        if (newVar.name.size() > 0 && newVar.name[0] == '_')
        {
            newVar.name.erase(0,1);
        }
    }

    void D3D11HLSLProgram::getDefines(String& stringBuffer, std::vector<D3D_SHADER_MACRO>& defines, const String& definesString)
    {
        // Populate preprocessor defines
        stringBuffer = definesString;

        defines.clear();

        for(const auto& def : parseDefines(stringBuffer))
        {
            defines.push_back({def.first, def.second});
        }

        //Add D3D11 define to all program, compiled with D3D11 RenderSystem
        D3D_SHADER_MACRO macro = {"D3D11","1"};
        defines.push_back(macro);       
		
        // Using different texture sampling instructions, tex2D for D3D9 and SampleXxx for D3D11,
        // declaring type of BLENDINDICES as float4 for D3D9 but as uint4 for D3D11 -  all those
        // small but annoying differences that otherwise would require declaring separate programs.
        macro.Name = "SHADER_MODEL_4";
        defines.push_back(macro);

		switch (this->mType)
		{
			case GPT_VERTEX_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_VS;
			break;
			case GPT_FRAGMENT_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_PS;
			break;
			case GPT_GEOMETRY_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_GS;
			break;
			case GPT_DOMAIN_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_DS;
			break;
			case GPT_HULL_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_HS;
			break;
			case GPT_COMPUTE_PROGRAM:
				macro.Name = HLSL_PROGRAM_DEFINE_CS;
			break;
			default:
				OGRE_EXCEPT(Exception::ERR_INVALID_STATE,"Could not find a compatible HLSL program type",
					"D3D11HLSLProgram::getDefines");
		}
		defines.push_back(macro);
		
        // Add NULL terminator
        macro.Name = 0;
        macro.Definition = 0;
        defines.push_back(macro);
    }       
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::prepareImpl()
    {
        HighLevelGpuProgram::prepareImpl();

        uint32 hash = getNameForMicrocodeCache();
        if ( GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(hash) )
        {
            getMicrocodeFromCache(hash);
        }
        else
        {
            compileMicrocode();
        }
    }

    void D3D11HLSLProgram::loadFromSource(void)
    {
        if(!mCompileError)
            analizeMicrocode();
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::getMicrocodeFromCache(uint32 id)
    {
        GpuProgramManager::Microcode cacheMicrocode = 
            GpuProgramManager::getSingleton().getMicrocodeFromCache(id);

        cacheMicrocode->seek(0);

#define READ_START(curlist, memberType) {                           \
    uint16 listSize = (uint16)curlist.size();                       \
    cacheMicrocode->read(&listSize, sizeof(uint16));                \
    if(listSize > 0){                                               \
        curlist.resize(listSize);                                   \
        for(unsigned i = 0 ; i < curlist.size() ; i++){             \
            memberType & curItem = curlist[i];

#define READ_END }}}

#define READ_UINT(member) {                                     \
    uint32 tmpVal;                                              \
    cacheMicrocode->read(&tmpVal, sizeof(uint32));              \
    curItem.member = tmpVal;                                    \
    }

#define READ_ENUM(member, enumType) {                           \
    uint32 tmpVal;                                              \
    cacheMicrocode->read(&tmpVal, sizeof(uint32));              \
    curItem.member = (enumType)tmpVal;                          \
    }

#define READ_BYTE(member) {                                 \
    cacheMicrocode->read(&curItem.member, sizeof(BYTE));    \
    }
                
#define READ_NAME(member) {                                 \
    uint16 length = 0;                                      \
    cacheMicrocode->read(&length, sizeof(uint16));          \
    curItem.member = "";                                    \
    if(length > 0)                                          \
    {                                                       \
        String * inString = new String();                   \
        inString->resize(length);                           \
        cacheMicrocode->read(&(*inString)[0], length);      \
        mSerStrings.push_back(inString);                    \
        curItem.member = &(*inString)[0];                   \
    }                                                       \
        }


        uint32 microCodeSize = 0;
        cacheMicrocode->read(&microCodeSize, sizeof(uint32));
        mMicroCode.resize(microCodeSize);
        cacheMicrocode->read(&mMicroCode[0], microCodeSize);

        cacheMicrocode->read(&mConstantBufferSize, sizeof(uint32));        
        cacheMicrocode->read(&mConstantBufferNr, sizeof(uint32));
        cacheMicrocode->read(&mNumSlots, sizeof(uint32));
        
        READ_START(mD3d11ShaderInputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
        READ_NAME(SemanticName)
        READ_UINT(SemanticIndex)
        READ_UINT(Register)
        READ_ENUM(SystemValueType,D3D_NAME)
        READ_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
        READ_BYTE(Mask)
        READ_BYTE(ReadWriteMask)
        READ_UINT(Stream)
//      READ_ENUM(MinPrecision, D3D_MIN_PRECISION) // not needed and doesn't exist in June 2010 SDK
        READ_END

        READ_START(mD3d11ShaderOutputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
        READ_NAME(SemanticName)
        READ_UINT(SemanticIndex)
        READ_UINT(Register)
        READ_ENUM(SystemValueType, D3D_NAME)
        READ_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
        READ_BYTE(Mask)
        READ_BYTE(ReadWriteMask)
        READ_UINT(Stream)
//      READ_ENUM(MinPrecision, D3D_MIN_PRECISION) // not needed and doesn't exist in June 2010 SDK
        READ_END

        READ_START(mD3d11ShaderVariables, D3D11_SHADER_VARIABLE_DESC)
        READ_NAME(Name)
        READ_UINT(StartOffset)
        READ_UINT(Size)
        READ_UINT(uFlags)
        //todo DefaultValue
        READ_UINT(StartTexture)
        READ_UINT(TextureSize)
        READ_UINT(StartSampler)
        READ_UINT(SamplerSize)
        READ_END

        READ_START(mD3d11ShaderVariableSubparts, GpuConstantDefinitionWithName)
        READ_NAME(Name)
        READ_ENUM(constType, GpuConstantType)
        READ_UINT(physicalIndex)
        READ_UINT(logicalIndex)
        READ_UINT(elementSize)
        READ_UINT(arraySize)
        READ_UINT(variability)
        READ_END

        READ_START(mD3d11ShaderBufferDescs, D3D11_SHADER_BUFFER_DESC)
        READ_NAME(Name)
        READ_ENUM(Type, D3D_CBUFFER_TYPE)
        READ_UINT(Variables)
        READ_UINT(Size)
        READ_UINT(uFlags)
        READ_END

        READ_START(mVarDescBuffer, D3D11_SHADER_VARIABLE_DESC)
        READ_NAME(Name)
        READ_UINT(StartOffset)
        READ_UINT(Size)
        READ_UINT(uFlags)
        //todo DefaultValue
        READ_UINT(StartTexture)
        READ_UINT(TextureSize)
        READ_UINT(StartSampler)
        READ_UINT(SamplerSize)
        READ_END

        READ_START(mVarDescPointer, D3D11_SHADER_VARIABLE_DESC)
        READ_NAME(Name)
        READ_UINT(StartOffset)
        READ_UINT(Size)
        READ_UINT(uFlags)
        //todo DefaultValue
        READ_UINT(StartTexture)
        READ_UINT(TextureSize)
        READ_UINT(StartSampler)
        READ_UINT(SamplerSize)
        READ_END

        READ_START(mD3d11ShaderTypeDescs, D3D11_SHADER_TYPE_DESC)
        READ_NAME(Name)
        READ_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
        READ_UINT(Rows)
        READ_UINT(Columns)
        READ_UINT(Elements)
        READ_UINT(Members)
        READ_UINT(Offset)
        READ_END

        READ_START(mMemberTypeDesc, D3D11_SHADER_TYPE_DESC)
        READ_NAME(Name)
        READ_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
        READ_UINT(Rows)
        READ_UINT(Columns)
        READ_UINT(Elements)
        READ_UINT(Members)
        READ_UINT(Offset)
        READ_END

        READ_START(mMemberTypeName, MemberTypeName)
        READ_NAME(Name)
        READ_END

        uint16 mInterfaceSlotsSize = 0;
        cacheMicrocode->read(&mInterfaceSlotsSize, sizeof(uint16));
        if(mInterfaceSlotsSize > 0)
        {
            mInterfaceSlots.resize(mInterfaceSlotsSize);
            cacheMicrocode->read(&mInterfaceSlots[0], mInterfaceSlotsSize * sizeof(UINT));
        }
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::compileMicrocode(void)
    {
        // If we are running from the cache, we should not be trying to compile/reflect on shaders.
#if defined(ENABLE_SHADERS_CACHE_LOAD) && (ENABLE_SHADERS_CACHE_LOAD == 1)
        String message = "Cannot compile/reflect D3D11 shader: " + mName + " in shipping code\n";
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, message,
            "D3D11HLSLProgram::compileMicrocode");
#else
#pragma comment(lib, "d3dcompiler.lib")

        String stringBuffer;
        std::vector<D3D_SHADER_MACRO> defines;
        const D3D_SHADER_MACRO* pDefines = NULL;
        getDefines(stringBuffer, defines, appendBuiltinDefines(mPreprocessorDefines));
        pDefines = defines.empty() ? NULL : &defines[0];


        UINT compileFlags=0;
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
#if OGRE_DEBUG_MODE
        compileFlags |= D3DCOMPILE_DEBUG;
        // Skip optimization only if we have enough instruction slots (>=256) and not feature level 9 hardware
        if (mTarget != "ps_2_0" && mTarget != "ps_4_0_level_9_1" && rsys->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_0)
            compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        if (mColumnMajorMatrices)
        {
            compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
        }
        else
        {
            compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
        }

        if (mEnableBackwardsCompatibility)
        {
            compileFlags |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
        }

        const char* target = getCompatibleTarget();

        ComPtr<ID3DBlob> pMicroCode;
        ComPtr<ID3DBlob> errors;

        // handle includes
        mSource = _resolveIncludes(mSource, this, mFilename, true);

        HRESULT hr = D3DCompile(
            mSource.c_str(),      // [in] Pointer to the shader in memory. 
            mSource.size(),       // [in] Size of the shader in memory.  
            mFilename.c_str(),    // [in] Optional. You can use this parameter for strings that specify error messages.
            pDefines,             // [in] Optional. Pointer to a NULL-terminated array of macro definitions. See D3D_SHADER_MACRO. If not used, set this to NULL. 
            NULL,                 // [in] Optional. Pointer to an ID3DInclude Interface interface for handling include files. Setting this to NULL will cause a compile error if a shader contains a #include.
            mEntryPoint.c_str(),  // [in] Name of the shader-entrypoint function where shader execution begins. 
            target,               // [in] A string that specifies the shader model; can be any profile in shader model 4 or higher. 
            compileFlags,         // [in] Effect compile flags - no D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY at the first try...
            NULL,                 // [in] Effect compile flags
            pMicroCode.GetAddressOf(),// [out] A pointer to an ID3DBlob Interface which contains the compiled shader, as well as any embedded debug and symbol-table information. 
            errors.GetAddressOf() // [out] A pointer to an ID3DBlob Interface which contains a listing of errors and warnings that occurred during compilation. These errors and warnings are identical to the the debug output from a debugger.
            );

#if 0 // this is how you disassemble
        LPCSTR commentString = NULL;
        ComPtr<ID3DBlob> pIDisassembly;
        const char* pDisassembly = NULL;
        if( pMicroCode )
        {
            D3DDisassemble( (UINT*) pMicroCode->GetBufferPointer(), 
                pMicroCode->GetBufferSize(), D3D_DISASM_ENABLE_COLOR_CODE, commentString, pIDisassembly.GetAddressOf() );
        }

        if (pIDisassembly)
        {
            pDisassembly = static_cast<const char*>(pIDisassembly->GetBufferPointer());
        }
#endif
        if (FAILED(hr))
        {
            mErrorsInCompile = true;
            String message = "Cannot compile D3D11 high-level shader " + mName + " Errors:\n" +
                static_cast<const char*>(errors ? errors->GetBufferPointer() : "<null>");
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, message,
                "D3D11HLSLProgram::compileMicrocode");
        }
        else
        {
#if OGRE_DEBUG_MODE
            // Log warnings if any
            const char* warnings = static_cast<const char*>(errors ? errors->GetBufferPointer() : 0);
            if(warnings && LogManager::getSingletonPtr())
            {
                String message = "Warnings while compiling D3D11 high-level shader " + mName + ":\n" + warnings;
                LogManager::getSingleton().logMessage(message, LML_NORMAL);
            }
#endif
            mMicroCode.resize(pMicroCode->GetBufferSize());
            memcpy(&mMicroCode[0], pMicroCode->GetBufferPointer(), pMicroCode->GetBufferSize());

            // get the parameters and variables from the shader reflection
            SIZE_T BytecodeLength = mMicroCode.size();
            ComPtr<ID3D11ShaderReflection> shaderReflection;
            HRESULT hr = D3DReflect( (void*) &mMicroCode[0], BytecodeLength,
                IID_ID3D11ShaderReflection, // can't do __uuidof(ID3D11ShaderReflection) here...
                (void**)shaderReflection.GetAddressOf() );

            if (FAILED(hr))
            {
                String message = "Cannot reflect D3D11 high-level shader " + mName;
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, message,
                    "D3D11HLSLProgram::compileMicrocode");
            }

            D3D11_SHADER_DESC shaderDesc;
            hr = shaderReflection->GetDesc( &shaderDesc );

            if (FAILED(hr))
            {
                String message = "Cannot get reflect info for D3D11 high-level shader " + mName;
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, message,
                    "D3D11HLSLProgram::compileMicrocode");
            }

            // get the input parameters
            mD3d11ShaderInputParameters.resize(shaderDesc.InputParameters);
            for (UINT i=0; i<shaderDesc.InputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC & curParam = mD3d11ShaderInputParameters[i];
                shaderReflection->GetInputParameterDesc( i, &curParam);
                String * name = new String(curParam.SemanticName);
                mSerStrings.push_back(name);
                curParam.SemanticName = &(*name)[0]; 
            }

            // get the output parameters
            mD3d11ShaderOutputParameters.resize(shaderDesc.OutputParameters);
            for (UINT i=0; i<shaderDesc.OutputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC & curParam = mD3d11ShaderOutputParameters[i];
                shaderReflection->GetOutputParameterDesc( i, &curParam);
                String * name = new String(curParam.SemanticName);
                mSerStrings.push_back(name);
                curParam.SemanticName = &(*name)[0]; 
            }
            /*
            if (shaderDesc.ConstantBuffers > 1)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "Multi constant buffers are not supported for now.",
                    "D3D11HLSLProgram::compileMicrocode");
            }*/
            
            mConstantBufferNr = shaderDesc.ConstantBuffers;
            mNumSlots = shaderReflection->GetNumInterfaceSlots();

            if (shaderDesc.ConstantBuffers > 0)
            {
                for (unsigned int v=0; v < shaderDesc.ConstantBuffers; v++)
                {
                    ID3D11ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer;
                    shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(v);

                    D3D11_SHADER_BUFFER_DESC constantBufferDesc;
                    hr = shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);
                    if (FAILED(hr))
                    {
						String message = "Cannot reflect constant buffer of D3D11 high-level shader " + mName;
						OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, message,
							"D3D11HLSLProgram::compileMicrocode");
                    }

                    String * name = new String(constantBufferDesc.Name);
                    mSerStrings.push_back(name);
                    constantBufferDesc.Name = &(*name)[0]; 
                    mD3d11ShaderBufferDescs.push_back(constantBufferDesc);

                    mConstantBufferSize += constantBufferDesc.Size;
                    if (v == 0)
                        mD3d11ShaderVariables.resize(constantBufferDesc.Variables);
                    else
                        mD3d11ShaderVariables.resize(mD3d11ShaderVariables.size() + constantBufferDesc.Variables);

                    for(unsigned int i = 0; i < constantBufferDesc.Variables ; i++)
                    {
                        D3D11_SHADER_VARIABLE_DESC & curVar = mD3d11ShaderVariables[i];
                        ID3D11ShaderReflectionVariable* varRef;
                        varRef = shaderReflectionConstantBuffer->GetVariableByIndex(i);
                        HRESULT hr = varRef->GetDesc(&curVar);
                        if (FAILED(hr))
                        {
							String message = "Cannot reflect constant buffer variable of D3D11 high-level shader " + mName;
							OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, message,
								"D3D11HLSLProgram::compileMicrocode");
                        }

                        String * name = new String(curVar.Name);
                        mSerStrings.push_back(name);
                        curVar.Name = &(*name)[0]; 

                        ID3D11ShaderReflectionType* varRefType;
                        varRefType = varRef->GetType();

                        // Recursively descend through the structure levels
                        processParamElement( "", curVar.Name, varRefType);
                    }

                    switch (constantBufferDesc.Type)
                    {
                    case D3D_CT_INTERFACE_POINTERS:
                        {
                            for(UINT k = 0; k < constantBufferDesc.Variables; k++)
                            {
                                D3D11_SHADER_VARIABLE_DESC varDesc;
                                ID3D11ShaderReflectionVariable* var = shaderReflectionConstantBuffer->GetVariableByIndex(k);
                                var->GetDesc(&varDesc);
                                String * name = new String(varDesc.Name);
                                mSerStrings.push_back(name);
                                varDesc.Name = &(*name)[0]; 
                                mVarDescPointer.push_back(varDesc);
                                mInterfaceSlots.push_back(var->GetInterfaceSlot(0));
                            }
                        }
                        break;
                    case D3D_CT_CBUFFER:
                    case D3D_CT_TBUFFER:
                        {
                            for(unsigned int k = 0; k < constantBufferDesc.Variables ; k++)
                            {
                                D3D11_SHADER_VARIABLE_DESC varDesc;
                                ID3D11ShaderReflectionVariable* varRef = shaderReflectionConstantBuffer->GetVariableByIndex(k);
                                varRef->GetDesc(&varDesc);
                                String * name = new String(varDesc.Name);
                                mSerStrings.push_back(name);
                                varDesc.Name = &(*name)[0]; 
                                mVarDescBuffer.push_back(varDesc);

                                // Only parse if variable is used
                                if (varDesc.uFlags & D3D_SVF_USED)
                                {
                                    D3D11_SHADER_TYPE_DESC varTypeDesc;
                                    ID3D11ShaderReflectionType* varType = varRef->GetType();
                                    varType->GetDesc(&varTypeDesc);
                                    if(varTypeDesc.Name == NULL)
                                    {
                                        mSerStrings.push_back(new String());
                                    }
                                    else
                                    {
                                        String * name = new String(varTypeDesc.Name);
                                        mSerStrings.push_back(name);
                                        varTypeDesc.Name = &(*name)[0]; 
                                    }

                                    mD3d11ShaderTypeDescs.push_back(varTypeDesc);


                                    if (varTypeDesc.Class == D3D_SVC_STRUCT)
                                    {
                                        const UINT parentOffset = varDesc.StartOffset;
                                        for(UINT m = 0; m < varTypeDesc.Members; m++)
                                        {
                                            D3D11_SHADER_TYPE_DESC memberTypeDesc;
                                            ID3D11ShaderReflectionType* memberType = varType->GetMemberTypeByIndex(m);
                                            memberType->GetDesc(&memberTypeDesc);

                                            {
                                                String * name = new String(memberTypeDesc.Name);
                                                mSerStrings.push_back(name);
                                                memberTypeDesc.Name = &(*name)[0]; 
                                                mMemberTypeDesc.push_back(memberTypeDesc);
                                            }
                                            {
                                                String * name = new String(varType->GetMemberTypeName(m));
                                                mSerStrings.push_back(name);
                                                MemberTypeName memberTypeName;
                                                memberTypeName.Name = &(*name)[0];
                                                mMemberTypeName.push_back(memberTypeName);
                                            }
                                            
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
            {

#define SIZE_OF_DATA_START(curlist, memberType) + sizeof(uint16) +  curlist.size() * ( 0
#define SIZE_OF_DATA_END )

#define SIZE_OF_DATA_UINT(member) + sizeof(uint32)
#define SIZE_OF_DATA_ENUM(member, memberType) SIZE_OF_DATA_UINT(member)

#define SIZE_OF_DATA_BYTE(member) +  sizeof(BYTE)

#define SIZE_OF_DATA_NAME(member) // we get the size in GET_SIZE_OF_NAMES - so do nothing

#define GET_SIZE_OF_NAMES(result, list, member)                     \
                uint32 result = 0;                                  \
                {                                                   \
                    for(unsigned i = 0 ; i < list.size() ; i++)          \
                    {                                               \
                        if (list[i].member != NULL)                 \
                            result += strlen(list[i].member);       \
                        result += sizeof(uint16);                   \
                    }                                               \
                }

                GET_SIZE_OF_NAMES(inputNamesSize, mD3d11ShaderInputParameters, SemanticName);
                GET_SIZE_OF_NAMES(outputNamesSize, mD3d11ShaderOutputParameters, SemanticName);
                GET_SIZE_OF_NAMES(varNamesSize, mD3d11ShaderVariables, Name);
                GET_SIZE_OF_NAMES(varSubpartSize, mD3d11ShaderVariableSubparts, Name);
                GET_SIZE_OF_NAMES(d3d11ShaderBufferDescsSize, mD3d11ShaderBufferDescs, Name);
                GET_SIZE_OF_NAMES(varDescBufferSize, mVarDescBuffer, Name);
                GET_SIZE_OF_NAMES(varDescPointerSize, mVarDescPointer, Name);
                GET_SIZE_OF_NAMES(d3d11ShaderTypeDescsSize, mD3d11ShaderTypeDescs, Name);
                GET_SIZE_OF_NAMES(memberTypeDescSize, mMemberTypeDesc, Name);
                GET_SIZE_OF_NAMES(memberTypeNameSize, mMemberTypeName, Name);

                int sizeOfData =   sizeof(uint32) +  mMicroCode.size()
                                 + sizeof(uint32) // mConstantBufferSize
                                 + sizeof(uint32) // mConstantBufferNr
                                 + sizeof(uint32) // mNumSlots
                                SIZE_OF_DATA_START(mD3d11ShaderInputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
                                SIZE_OF_DATA_NAME(SemanticName)
                                SIZE_OF_DATA_UINT(SemanticIndex)
                                SIZE_OF_DATA_UINT(Register)
                                SIZE_OF_DATA_ENUM(SystemValueType,D3D_NAME)
                                SIZE_OF_DATA_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
                                SIZE_OF_DATA_BYTE(Mask)
                                SIZE_OF_DATA_BYTE(ReadWriteMask)
                                SIZE_OF_DATA_UINT(Stream)
//                              SIZE_OF_DATA_ENUM(MinPrecision, D3D_MIN_PRECISION)  // not needed and doesn't exist in June 2010 SDK
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mD3d11ShaderOutputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
                                SIZE_OF_DATA_NAME(SemanticName)
                                SIZE_OF_DATA_UINT(SemanticIndex)
                                SIZE_OF_DATA_UINT(Register)
                                SIZE_OF_DATA_ENUM(SystemValueType, D3D_NAME)
                                SIZE_OF_DATA_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
                                SIZE_OF_DATA_BYTE(Mask)
                                SIZE_OF_DATA_BYTE(ReadWriteMask)
                                SIZE_OF_DATA_UINT(Stream)
//                              SIZE_OF_DATA_ENUM(MinPrecision, D3D_MIN_PRECISION)  // not needed and doesn't exist in June 2010 SDK
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mD3d11ShaderVariables, D3D11_SHADER_VARIABLE_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_UINT(StartOffset)
                                SIZE_OF_DATA_UINT(Size)
                                SIZE_OF_DATA_UINT(uFlags)
                                //todo DefaultValue
                                SIZE_OF_DATA_UINT(StartTexture)
                                SIZE_OF_DATA_UINT(TextureSize)
                                SIZE_OF_DATA_UINT(StartSampler)
                                SIZE_OF_DATA_UINT(SamplerSize)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mD3d11ShaderVariableSubparts, GpuConstantDefinitionWithName)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_ENUM(constType, GpuConstantType)
                                SIZE_OF_DATA_UINT(physicalIndex)
                                SIZE_OF_DATA_UINT(logicalIndex)
                                SIZE_OF_DATA_UINT(elementSize)
                                SIZE_OF_DATA_UINT(arraySize)
                                SIZE_OF_DATA_UINT(variability)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mD3d11ShaderBufferDescs, D3D11_SHADER_BUFFER_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_ENUM(Type, D3D_CBUFFER_TYPE)
                                SIZE_OF_DATA_UINT(Variables)
                                SIZE_OF_DATA_UINT(Size)
                                SIZE_OF_DATA_UINT(uFlags)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mVarDescBuffer, D3D11_SHADER_VARIABLE_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_UINT(StartOffset)
                                SIZE_OF_DATA_UINT(Size)
                                SIZE_OF_DATA_UINT(uFlags)
                                //todo DefaultValue
                                SIZE_OF_DATA_UINT(StartTexture)
                                SIZE_OF_DATA_UINT(TextureSize)
                                SIZE_OF_DATA_UINT(StartSampler)
                                SIZE_OF_DATA_UINT(SamplerSize)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mVarDescPointer, D3D11_SHADER_VARIABLE_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_UINT(StartOffset)
                                SIZE_OF_DATA_UINT(Size)
                                SIZE_OF_DATA_UINT(uFlags)
                                //todo DefaultValue
                                SIZE_OF_DATA_UINT(StartTexture)
                                SIZE_OF_DATA_UINT(TextureSize)
                                SIZE_OF_DATA_UINT(StartSampler)
                                SIZE_OF_DATA_UINT(SamplerSize)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mD3d11ShaderTypeDescs, D3D11_SHADER_TYPE_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
                                SIZE_OF_DATA_UINT(Rows)
                                SIZE_OF_DATA_UINT(Columns)
                                SIZE_OF_DATA_UINT(Elements)
                                SIZE_OF_DATA_UINT(Members)
                                SIZE_OF_DATA_UINT(Offset)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mMemberTypeDesc, D3D11_SHADER_TYPE_DESC)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
                                SIZE_OF_DATA_UINT(Rows)
                                SIZE_OF_DATA_UINT(Columns)
                                SIZE_OF_DATA_UINT(Elements)
                                SIZE_OF_DATA_UINT(Members)
                                SIZE_OF_DATA_UINT(Offset)
                                SIZE_OF_DATA_END

                                SIZE_OF_DATA_START(mMemberTypeName, MemberTypeName)
                                SIZE_OF_DATA_NAME(Name)
                                SIZE_OF_DATA_END


                                 + inputNamesSize
                                 + outputNamesSize
                                 + varNamesSize
                                 + varSubpartSize
                                 + d3d11ShaderBufferDescsSize
                                 + varDescBufferSize
                                 + varDescPointerSize
                                 + d3d11ShaderTypeDescsSize
                                 + memberTypeDescSize
                                 + memberTypeNameSize

                                 + sizeof(uint16) +  mInterfaceSlots.size() * sizeof(UINT)
                                 ;


                // create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode(sizeOfData);


#define WRITE_START(curlist, memberType) {                          \
    uint16 listSize = (uint16)curlist.size();                       \
    newMicrocode->write(&listSize, sizeof(uint16));                 \
    if(listSize > 0){                                               \
    for(unsigned i = 0 ; i < curlist.size() ; i++){                 \
        memberType & curItem = curlist[i];

#define WRITE_END }}}

#define WRITE_UINT(member) {                                        \
    uint32 tmpVal;                                                  \
    tmpVal = (uint32)curItem.member;                                \
    newMicrocode->write(&tmpVal, sizeof(uint32));                   \
    }

#define WRITE_ENUM(member, memberType) WRITE_UINT(member)

#define WRITE_BYTE(member) {                                        \
    newMicrocode->write(&curItem.member, sizeof(BYTE)); \
    }

#define WRITE_NAME(member) {                         \
    uint16 length = 0;                               \
    if(curItem.member != NULL)                       \
        length = (uint16)strlen(curItem.member);     \
    newMicrocode->write(&length, sizeof(uint16));    \
    if(length != 0)                                  \
    newMicrocode->write(curItem.member, length);     \
                }

                uint32 microCodeSize = mMicroCode.size();
                newMicrocode->write(&microCodeSize, sizeof(uint32));
                newMicrocode->write(&mMicroCode[0], microCodeSize);

                newMicrocode->write(&mConstantBufferSize, sizeof(uint32));        
                newMicrocode->write(&mConstantBufferNr, sizeof(uint32));
                newMicrocode->write(&mNumSlots, sizeof(uint32));

                WRITE_START(mD3d11ShaderInputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
                WRITE_NAME(SemanticName)
                WRITE_UINT(SemanticIndex)
                WRITE_UINT(Register)
                WRITE_ENUM(SystemValueType,D3D_NAME)
                WRITE_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
                WRITE_BYTE(Mask)
                WRITE_BYTE(ReadWriteMask)
                WRITE_UINT(Stream)
//              WRITE_ENUM(MinPrecision, D3D_MIN_PRECISION)  // not needed and doesn't exist in June 2010 SDK
                WRITE_END

                WRITE_START(mD3d11ShaderOutputParameters, D3D11_SIGNATURE_PARAMETER_DESC)
                WRITE_NAME(SemanticName)
                WRITE_UINT(SemanticIndex)
                WRITE_UINT(Register)
                WRITE_ENUM(SystemValueType, D3D_NAME)
                WRITE_ENUM(ComponentType, D3D_REGISTER_COMPONENT_TYPE)
                WRITE_BYTE(Mask)
                WRITE_BYTE(ReadWriteMask)
                WRITE_UINT(Stream)
//              WRITE_ENUM(MinPrecision, D3D_MIN_PRECISION)  // not needed and doesn't exist in June 2010 SDK
                WRITE_END

                WRITE_START(mD3d11ShaderVariables, D3D11_SHADER_VARIABLE_DESC)
                WRITE_NAME(Name)
                WRITE_UINT(StartOffset)
                WRITE_UINT(Size)
                WRITE_UINT(uFlags)
                //todo DefaultValue
                WRITE_UINT(StartTexture)
                WRITE_UINT(TextureSize)
                WRITE_UINT(StartSampler)
                WRITE_UINT(SamplerSize)
                WRITE_END

                WRITE_START(mD3d11ShaderVariableSubparts, GpuConstantDefinitionWithName)
                WRITE_NAME(Name)
                WRITE_ENUM(constType, GpuConstantType)
                WRITE_UINT(physicalIndex)
                WRITE_UINT(logicalIndex)
                WRITE_UINT(elementSize)
                WRITE_UINT(arraySize)
                WRITE_UINT(variability)
                WRITE_END

                WRITE_START(mD3d11ShaderBufferDescs, D3D11_SHADER_BUFFER_DESC)
                WRITE_NAME(Name)
                WRITE_ENUM(Type, D3D_CBUFFER_TYPE)
                WRITE_UINT(Variables)
                WRITE_UINT(Size)
                WRITE_UINT(uFlags)
                WRITE_END

                WRITE_START(mVarDescBuffer, D3D11_SHADER_VARIABLE_DESC)
                WRITE_NAME(Name)
                WRITE_UINT(StartOffset)
                WRITE_UINT(Size)
                WRITE_UINT(uFlags)
                //todo DefaultValue
                WRITE_UINT(StartTexture)
                WRITE_UINT(TextureSize)
                WRITE_UINT(StartSampler)
                WRITE_UINT(SamplerSize)
                WRITE_END

                WRITE_START(mVarDescPointer, D3D11_SHADER_VARIABLE_DESC)
                WRITE_NAME(Name)
                WRITE_UINT(StartOffset)
                WRITE_UINT(Size)
                WRITE_UINT(uFlags)
                //todo DefaultValue
                WRITE_UINT(StartTexture)
                WRITE_UINT(TextureSize)
                WRITE_UINT(StartSampler)
                WRITE_UINT(SamplerSize)
                WRITE_END

                WRITE_START(mD3d11ShaderTypeDescs, D3D11_SHADER_TYPE_DESC)
                WRITE_NAME(Name)
                WRITE_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
                WRITE_UINT(Rows)
                WRITE_UINT(Columns)
                WRITE_UINT(Elements)
                WRITE_UINT(Members)
                WRITE_UINT(Offset)
                WRITE_END

                WRITE_START(mMemberTypeDesc, D3D11_SHADER_TYPE_DESC)
                WRITE_NAME(Name)
                WRITE_ENUM(Type, D3D_SHADER_VARIABLE_TYPE)
                WRITE_UINT(Rows)
                WRITE_UINT(Columns)
                WRITE_UINT(Elements)
                WRITE_UINT(Members)
                WRITE_UINT(Offset)
                WRITE_END

                WRITE_START(mMemberTypeName, MemberTypeName)
                WRITE_NAME(Name)
                WRITE_END

                uint16 mInterfaceSlotsSize = (uint16)mInterfaceSlots.size();
                newMicrocode->write(&mInterfaceSlotsSize, sizeof(uint16));
                if(mInterfaceSlotsSize > 0)
                {
                    newMicrocode->write(&mInterfaceSlots[0],  mInterfaceSlotsSize * sizeof(UINT));
                }


                // add to the microcode to the cache
                GpuProgramManager::getSingleton().addMicrocodeToCache(getNameForMicrocodeCache(), newMicrocode);
            }
        }
#endif // else defined(ENABLE_SHADERS_CACHE_LOAD) && (ENABLE_SHADERS_CACHE_LOAD == 1)
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::analizeMicrocode()
    {
        UINT bufferCount = 0;
        UINT pointerCount = 0;
        UINT typeCount = 0;
        UINT memberCount = 0;
        UINT interCount = 0;
        UINT nameCount = 0;

        for(UINT b = 0; b < mConstantBufferNr; b++)
        {           
            switch (mD3d11ShaderBufferDescs[b].Type)
            {
            // For this buffer type, all variables are interfaces, 
            // so just parse and store interface slots
            case D3D_CT_INTERFACE_POINTERS:
                {
                    for(UINT v = 0; v < mD3d11ShaderBufferDescs[b].Variables; v++)
                    {
                        interCount++;
                        pointerCount++;
                        // Only parse if is used
                        if (mVarDescPointer[pointerCount-1].uFlags & D3D_SVF_USED)
                        {
                            mSlotMap.insert(std::make_pair(mVarDescPointer[pointerCount-1].Name, mInterfaceSlots[interCount-1]));

                            /*
                            D3D11_SHADER_TYPE_DESC typeDesc;
                            ID3D11ShaderReflectionType* varType = var->GetType();
                            varType->GetDesc(&typeDesc);
                                    
                            // Get all interface slots if inside an array
                            //unsigned int numInterface = varType->GetNumInterfaces();
                            for(UINT ifs = 0; ifs < typeDesc.Elements; ifs++)
                            {
                                std::string name = varDesc.Name;
                                name += "[";
                                name += StringConverter::toString(ifs);
                                name += "]";
                                mSlotMap.insert(std::make_pair(name, ifs));
                            }
                            */
                        }
                    }
                }
                break;

            // These buffers store variables and class instances, 
            // so parse all.
            case D3D_CT_CBUFFER:
            case D3D_CT_TBUFFER:
                {
                    // Insert buffer info
                    BufferInfoIterator it = mBufferInfoMap.insert(BufferInfo(0, mD3d11ShaderBufferDescs[b].Name)).first;

                    // Guard to create uniform buffer only once
                    if (!it->mUniformBuffer)
                    {
                        HardwareUniformBufferSharedPtr uBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(mD3d11ShaderBufferDescs[b].Size, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);
                        it->mUniformBuffer = HardwareUniformBufferSharedPtr(uBuffer);
                    }
                    else
                    {
                        assert (it->mUniformBuffer->getSizeInBytes() == mD3d11ShaderBufferDescs[b].Size);
                    }

                    // Now, parse variables for this buffer
                    for(unsigned int i = 0; i < mD3d11ShaderBufferDescs[b].Variables ; i++)
                    {
                        // Only parse if variable is used
                        bufferCount++;
                        if (mVarDescBuffer[bufferCount-1].uFlags & D3D_SVF_USED)
                        {
                            typeCount++;
                            switch (mD3d11ShaderTypeDescs[typeCount-1].Class)
                            {
                            // Class instance variables
                            case D3D_SVC_STRUCT:
                                {
                                    // Offset if relative to parent struct, so store offset
                                    // of class instance, and add to offset of inner variables
                                    const UINT parentOffset = mVarDescBuffer[bufferCount - 1].StartOffset;
                                    for(UINT m = 0; m < mD3d11ShaderTypeDescs[typeCount-1].Members; m++)
                                    {
                                        ShaderVarWithPosInBuf newVar;
                                        newVar.name = mVarDescBuffer[bufferCount - 1].Name;
                                        newVar.name += "."; 
                                        newVar.name += mMemberTypeName[nameCount++].Name;
                                        newVar.size = mMemberTypeDesc[memberCount].Rows * mMemberTypeDesc[memberCount].Columns * 
                                                                    (mMemberTypeDesc[memberCount].Type == D3D_SVT_FLOAT ||
                                                                        mMemberTypeDesc[memberCount].Type == D3D_SVT_INT ? 4 : 1);
                                        newVar.startOffset = parentOffset + mMemberTypeDesc[memberCount].Offset;
                                        memberCount++;
                                        fixVariableNameFromCg(newVar);
                                        it->mShaderVars.push_back(newVar);
                                    }
                                }
                                break;

                            // scalar, vector or matrix variables
                            case D3D_SVC_SCALAR:
                            case D3D_SVC_VECTOR:
                            case D3D_SVC_MATRIX_ROWS:
                            case D3D_SVC_MATRIX_COLUMNS:
                                {
                                    ShaderVarWithPosInBuf newVar;
                                    newVar.name = mVarDescBuffer[bufferCount-1].Name;
                                    newVar.size = mVarDescBuffer[bufferCount-1].Size;
                                    newVar.startOffset = mVarDescBuffer[bufferCount-1].StartOffset;

                                    fixVariableNameFromCg(newVar);
                                    it->mShaderVars.push_back(newVar);
                                }
                                break;
                            };

                        }
                    }
                }
                break;
            }
        }
        
        switch(mType)
        {
        case GPT_VERTEX_PROGRAM:
            CreateVertexShader();
            break;
        case GPT_FRAGMENT_PROGRAM:
            CreatePixelShader();
            break;
        case GPT_GEOMETRY_PROGRAM:
            CreateGeometryShader();
            break;
        case GPT_DOMAIN_PROGRAM:
            CreateDomainShader();
            break;
        case GPT_HULL_PROGRAM:
            CreateHullShader();
            break;
        case GPT_COMPUTE_PROGRAM:
            CreateComputeShader();
            break;
        }
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::unloadHighLevelImpl(void)
    {
        mSlotMap.clear();
        mBufferInfoMap.clear();

        mVertexShader.Reset();
        mPixelShader.Reset();
        mGeometryShader.Reset();
        mDomainShader.Reset();
        mHullShader.Reset();
        mComputeShader.Reset();
        mConstantBuffer.Reset();

        for(unsigned int i = 0 ; i < mSerStrings.size() ; i++)
        {
            delete mSerStrings[i];
        }
        mSerStrings.clear();
        mD3d11ShaderInputParameters.clear();
        mD3d11ShaderOutputParameters.clear();
        mD3d11ShaderBufferDescs.clear();
        mD3d11ShaderVariables.clear();
        mD3d11ShaderVariableSubparts.clear();
        mVarDescBuffer.clear();
        mD3d11ShaderTypeDescs.clear();
    }

    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::buildConstantDefinitions() const
    {
        createParameterMappingStructures(true);

        for(unsigned int i = 0 ; i < mD3d11ShaderVariableSubparts.size() ; i++)
        {
            GpuConstantDefinitionWithName def = mD3d11ShaderVariableSubparts[i];
            int paramIndex = def.logicalIndex;
            GpuLogicalBufferStruct* currentBuffer = NULL;
            size_t* currentBufferSize = NULL;
            if (def.isFloat())
            {
                currentBuffer = mFloatLogicalToPhysical.get();
                currentBufferSize = &mConstantDefs->floatBufferSize;
            }
            else if (def.isInt() || def.isUnsignedInt())
            {
                currentBuffer = mIntLogicalToPhysical.get();
                currentBufferSize = &mConstantDefs->intBufferSize;
            }

            if (currentBuffer != NULL && currentBufferSize != NULL)
            {
                def.physicalIndex = currentBuffer->bufferSize;
                OGRE_LOCK_MUTEX(currentBuffer->mutex);
                currentBuffer->map.emplace(
                    paramIndex, GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL));
                currentBuffer->bufferSize += def.arraySize * def.elementSize;
                *currentBufferSize = currentBuffer->bufferSize;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
                            "Currently the only supported variables for Direct3D11 hlsl program are: 'float', 'int' and ' unsigned int'", 
                            "D3D11HLSLProgram::getConstantBuffer");
            }

            mConstantDefs->map.emplace(def.Name, def);
        }
    }
    //-----------------------------------------------------------------------

    void D3D11HLSLProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        // Call superclass method
        HighLevelGpuProgram::populateParameterNames(params);
        
        /*
        // Construct uniform buffers for shared parameters, if any
        // Also, construct uniform buffers for non-shared parameters (will be hosted inside program)
        const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();
        GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();
        for (it = sharedParams.begin(); it != end; ++it)
        {
            ID3D11ShaderReflectionConstantBuffer* bufferInfo = mIShaderReflection->GetConstantBufferByName(it->getName().c_str());
            if (!bufferInfo)
            {

            }

            D3D11_SHADER_BUFFER_DESC bufferDesc;
            bufferInfo->GetDesc(&bufferDesc);

            HardwareUniformBufferSharedPtr uniformBuffer = 
                        HardwareBufferManager::getSingleton()
                                              .createUniformBuffer(bufferDesc.Size, 
                                                                   HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, 
                                                                   false, it->getName());
            it->_setRenderSystemData(Any(uniformBuffer));
        }
        */

    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::processParamElement(String prefix, LPCSTR pName, ID3D11ShaderReflectionType* varRefType)
    {
        D3D11_SHADER_TYPE_DESC varRefTypeDesc;
        HRESULT hr = varRefType->GetDesc(&varRefTypeDesc);

        // Since D3D HLSL doesn't deal with naming of array and struct parameters
        // automatically, we have to do it by hand
        if (FAILED(hr))
        {
			OGRE_EXCEPT_EX(Exception::ERR_INTERNAL_ERROR, hr,
                "Cannot retrieve constant description from HLSL program.", 
                "D3D11HLSLProgram::processParamElement");
        }

        String paramName = pName;
        // trim the odd '$' which appears at the start of the names in HLSL
        if (paramName.at(0) == '$' || paramName.at(0) == '_')
            paramName.erase(paramName.begin());

        // Also trim the '[0]' suffix if it exists, we will add our own indexing later
        if (StringUtil::endsWith(paramName, "[0]", false))
        {
            paramName.erase(paramName.size() - 3);
        }

        if (varRefTypeDesc.Class == D3D_SVC_STRUCT)
        {
            // work out a new prefix for nested members, if it's an array, we need an index
            prefix = prefix + paramName + ".";
            // Cascade into struct
            for (unsigned int i = 0; i < varRefTypeDesc.Members; ++i)
            {
                processParamElement(prefix, varRefType->GetMemberTypeName(i), varRefType->GetMemberTypeByIndex(i));
            }
        }
        else
        {
            // Process params
            if (   varRefTypeDesc.Type == D3D_SVT_FLOAT 
                || varRefTypeDesc.Type == D3D_SVT_INT 
                || varRefTypeDesc.Type == D3D_SVT_UINT 
                || varRefTypeDesc.Type == D3D_SVT_BOOL)
            {
                GpuConstantDefinitionWithName def;
                String * name = new String(prefix + paramName);
                mSerStrings.push_back(name);
                def.Name = &(*name)[0]; 

                GpuConstantDefinitionWithName* prev_def = mD3d11ShaderVariableSubparts.empty() ? NULL : &mD3d11ShaderVariableSubparts.back();
                def.logicalIndex = prev_def ? prev_def->logicalIndex + prev_def->elementSize / 4 : 0;

                // populate type, array size & element size
                populateDef(varRefTypeDesc, def);

                mD3d11ShaderVariableSubparts.push_back(def);
            }
        }

    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::populateDef(D3D11_SHADER_TYPE_DESC& d3dDesc, GpuConstantDefinition& def) const
    {
        def.arraySize = std::max<unsigned int>(d3dDesc.Elements, 1);
        switch(d3dDesc.Type)
        {
        case D3D10_SVT_INT:
            switch(d3dDesc.Columns)
            {
            case 1:
                def.constType = GCT_INT1;
                break;
            case 2:
                def.constType = GCT_INT2;
                break;
            case 3:
                def.constType = GCT_INT3;
                break;
            case 4:
                def.constType = GCT_INT4;
                break;
            } // columns
            break;
        case D3D10_SVT_UINT:
            switch (d3dDesc.Columns)
            {
            case 1:
                def.constType = GCT_UINT1;
                break;
            case 2:
                def.constType = GCT_UINT2;
                break;
            case 3:
                def.constType = GCT_UINT3;
                break;
            case 4:
                def.constType = GCT_UINT4;
                break;
            } // columns
            break;
        case D3D10_SVT_FLOAT:
            switch(d3dDesc.Rows)
            {
            case 1:
                switch(d3dDesc.Columns)
                {
                case 1:
                    def.constType = GCT_FLOAT1;
                    break;
                case 2:
                    def.constType = GCT_FLOAT2;
                    break;
                case 3:
                    def.constType = GCT_FLOAT3;
                    break;
                case 4:
                    def.constType = GCT_FLOAT4;
                    break;
                } // columns
                break;
            case 2:
                switch(d3dDesc.Columns)
                {
                case 2:
                    def.constType = GCT_MATRIX_2X2;
                    break;
                case 3:
                    def.constType = GCT_MATRIX_2X3;
                    break;
                case 4:
                    def.constType = GCT_MATRIX_2X4;
                    break;
                } // columns
                break;
            case 3:
                switch(d3dDesc.Columns)
                {
                case 2:
                    def.constType = GCT_MATRIX_3X2;
                    break;
                case 3:
                    def.constType = GCT_MATRIX_3X3;
                    break;
                case 4:
                    def.constType = GCT_MATRIX_3X4;
                    break;
                } // columns
                break;
            case 4:
                switch(d3dDesc.Columns)
                {
                case 2:
                    def.constType = GCT_MATRIX_4X2;
                    break;
                case 3:
                    def.constType = GCT_MATRIX_4X3;
                    break;
                case 4:
                    def.constType = GCT_MATRIX_4X4;
                    break;
                } // columns
                break;

            } // rows
            break;

        case D3D_SVT_INTERFACE_POINTER:
            def.constType = GCT_SUBROUTINE;
            break;

        default:
            // not mapping samplers, don't need to take the space 
            break;
        };

        // HLSL pads to 4 elements
        def.elementSize = GpuConstantDefinition::getElementSize(def.constType, true);


    }
    //-----------------------------------------------------------------------
    D3D11HLSLProgram::D3D11HLSLProgram(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, D3D11Device & device)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
        , mEntryPoint("main"), mErrorsInCompile(false), mDevice(device), mConstantBufferSize(0)
        , mColumnMajorMatrices(true), mEnableBackwardsCompatibility(false), mReinterpretingGS(false)
    {
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
		mEnableBackwardsCompatibility = true;
#endif

        if (createParamDictionary("D3D11HLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("entry_point", 
                "The entry point for the HLSL program.",
                PT_STRING),&msCmdEntryPoint);
            dict->addParameter(ParameterDef("target", 
                "Name of the assembler target to compile down to.",
                PT_STRING),&msCmdTarget);
            dict->addParameter(ParameterDef("column_major_matrices", 
                "Whether matrix packing in column-major order.",
                PT_BOOL),&msCmdColumnMajorMatrices);
            dict->addParameter(ParameterDef("enable_backwards_compatibility", 
                "enable backwards compatibility.",
                PT_BOOL),&msCmdEnableBackwardsCompatibility);
        }

    }
    //-----------------------------------------------------------------------
    D3D11HLSLProgram::~D3D11HLSLProgram()
    {
        //mConstantBuffer.Reset();
        mBufferInfoMap.clear();

        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if ( isLoaded() )
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //-----------------------------------------------------------------------
    bool D3D11HLSLProgram::isSupported(void) const
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Get the supported syntaxed from RenderSystemCapabilities 
        return rs->getCapabilities()->isShaderProfileSupported(getCompatibleTarget()) && GpuProgram::isSupported();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr D3D11HLSLProgram::createParameters(void)
    {
        // Call superclass
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();

        // D3D HLSL uses column-major matrices
        params->setTransposeMatrices(mColumnMajorMatrices);

        return params;
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::setTarget(const String& target)
    {
        mTarget = "";
        std::vector<String> profiles = StringUtil::split(target, " ");
        for(unsigned int i = 0 ; i < profiles.size() ; i++)
        {
            String & currentProfile = profiles[i];
            if(GpuProgramManager::getSingleton().isSyntaxSupported(currentProfile))
            {
                mTarget = currentProfile;
                break;
            }
        }

        if(mTarget == "")
        {
            LogManager::getSingleton().logMessage(
                "Invalid target for D3D11 shader '" + mName + "' - '" + target + "'");
        }


    }
    //-----------------------------------------------------------------------
    const char* D3D11HLSLProgram::getCompatibleTarget(void) const
    {
        if(mTarget.empty())
        {
            return mType == GPT_VERTEX_PROGRAM ? "vs_4_0_level_9_1" : "ps_4_0_level_9_1";
        }

        if(mEnableBackwardsCompatibility)
        {
            if(mTarget == "vs_2_0") return "vs_4_0_level_9_1";
            if(mTarget == "vs_2_a") return "vs_4_0_level_9_3";
            if(mTarget == "vs_3_0") return "vs_4_0";

            if(mTarget == "ps_2_0") return "ps_4_0_level_9_1";
            if(mTarget == "ps_2_a") return "ps_4_0_level_9_3";
            if(mTarget == "ps_2_b") return "ps_4_0_level_9_3";
            if(mTarget == "ps_3_0") return "ps_4_0";
        }

        return mTarget.c_str();
    }
    //-----------------------------------------------------------------------
    const String& D3D11HLSLProgram::getLanguage(void) const
    {
        static const String language = "hlsl";

        return language;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String D3D11HLSLProgram::CmdEntryPoint::doGet(const void *target) const
    {
        return static_cast<const D3D11HLSLProgram*>(target)->getEntryPoint();
    }
    void D3D11HLSLProgram::CmdEntryPoint::doSet(void *target, const String& val)
    {
        static_cast<D3D11HLSLProgram*>(target)->setEntryPoint(val);
    }
    //-----------------------------------------------------------------------
    String D3D11HLSLProgram::CmdTarget::doGet(const void *target) const
    {
        return static_cast<const D3D11HLSLProgram*>(target)->getTarget();
    }
    void D3D11HLSLProgram::CmdTarget::doSet(void *target, const String& val)
    {
        static_cast<D3D11HLSLProgram*>(target)->setTarget(val);
    }
    //-----------------------------------------------------------------------
    String D3D11HLSLProgram::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const D3D11HLSLProgram*>(target)->getColumnMajorMatrices());
    }
    void D3D11HLSLProgram::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<D3D11HLSLProgram*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String D3D11HLSLProgram::CmdEnableBackwardsCompatibility::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const D3D11HLSLProgram*>(target)->getEnableBackwardsCompatibility());
    }
    void D3D11HLSLProgram::CmdEnableBackwardsCompatibility::doSet(void *target, const String& val)
    {
        static_cast<D3D11HLSLProgram*>(target)->setEnableBackwardsCompatibility(StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreateVertexShader()
    {
        if (isSupported())
        {
            // Create the shader
            HRESULT hr = mDevice->CreateVertexShader( 
                &mMicroCode[0],
                mMicroCode.size(),
                mDevice.GetClassLinkage(),
                mVertexShader.ReleaseAndGetAddressOf());

            assert(mVertexShader);

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, 
                    "Cannot create D3D11 vertex shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreateVertexShader");
            }
        }
        else
        {
            assert(false);
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 vertex shader '" + mName + "' was not loaded.");
        }
    }

    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreatePixelShader()
    {
        if (isSupported())
        {
            // Create the shader
            HRESULT hr = mDevice->CreatePixelShader( 
                &mMicroCode[0], 
                mMicroCode.size(),
                mDevice.GetClassLinkage(),
                mPixelShader.ReleaseAndGetAddressOf());

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                    "Cannot create D3D11 Pixel shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreatePixelShader");
            }
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 Pixel shader '" + mName + "' was not loaded.");
        }
    }

    void D3D11HLSLProgram::reinterpretGSForStreamOut(void)
    {
        assert(mGeometryShader);
        unloadHighLevel();
        mReinterpretingGS = true;
        prepareImpl();
        loadHighLevel();
        mReinterpretingGS = false;
    }

    const D3D11_SIGNATURE_PARAMETER_DESC & D3D11HLSLProgram::getInputParamDesc(unsigned int index) const
    {
        assert(index<mD3d11ShaderInputParameters.size());
        return mD3d11ShaderInputParameters[index];
    }
    const D3D11_SIGNATURE_PARAMETER_DESC & D3D11HLSLProgram::getOutputParamDesc(unsigned int index) const
    {
        assert(index<mD3d11ShaderOutputParameters.size());
        return mD3d11ShaderOutputParameters[index];
    }

    static unsigned int getComponentCount(BYTE mask)
    {
        unsigned int compCount = 0;
        if (mask&1)
            ++compCount;
        if (mask&2)
            ++compCount;
        if (mask&4)
            ++compCount;
        if (mask&8)
            ++compCount;
        return compCount;
    }

    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreateGeometryShader()
    {
        if (isSupported())
        {
            HRESULT hr;
            if (mReinterpretingGS)
            {
                D3D11_SO_DECLARATION_ENTRY* soDeclarations = new D3D11_SO_DECLARATION_ENTRY[mD3d11ShaderOutputParameters.size()];
                int totalComp = 0;
                for(unsigned int i = 0; i < getNumOutputs(); ++i)
                {
                    D3D11_SIGNATURE_PARAMETER_DESC pDesc = getOutputParamDesc(i);

                    soDeclarations[i].Stream = 0;
                    soDeclarations[i].SemanticName = pDesc.SemanticName;
                    soDeclarations[i].SemanticIndex= pDesc.SemanticIndex;

                    int compCount = getComponentCount(pDesc.Mask);
                    soDeclarations[i].StartComponent = 0;
                    soDeclarations[i].ComponentCount = compCount;
                    soDeclarations[i].OutputSlot = 0;

                    totalComp += compCount;
                }

                // Create the shader
                UINT bufferStrides[1];
                bufferStrides[0] = totalComp*sizeof(float);
                hr = mDevice->CreateGeometryShaderWithStreamOutput( 
                    &mMicroCode[0], 
                    mMicroCode.size(),
                    soDeclarations,
                    mD3d11ShaderOutputParameters.size(),
                    bufferStrides,
                    1,
                    D3D11_SO_NO_RASTERIZED_STREAM,
                    mDevice.GetClassLinkage(),
                    mGeometryShader.ReleaseAndGetAddressOf());

                delete [] soDeclarations;

            }
            else
            {
                // Create the shader
                hr = mDevice->CreateGeometryShader( 
                    &mMicroCode[0], 
                    mMicroCode.size(),
                    mDevice.GetClassLinkage(),
                    mGeometryShader.ReleaseAndGetAddressOf());
            }


            assert(mGeometryShader);

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, 
                    "Cannot create D3D11 Geometry shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreateGeometryShader");
            }
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 Geometry shader '" + mName + "' was not loaded.");
        }
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreateHullShader()
    {
        if (isSupported())
        {
            // Create the shader
            HRESULT hr = mDevice->CreateHullShader( 
                &mMicroCode[0], 
                mMicroCode.size(),
                mDevice.GetClassLinkage(),
                mHullShader.ReleaseAndGetAddressOf());

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 Hull shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreateHullShader");
            }
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 Hull shader '" + mName + "' was not loaded.");
        }
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreateDomainShader()
    {
        if (isSupported())
        {
            // Create the shader
            HRESULT hr = mDevice->CreateDomainShader( 
                &mMicroCode[0], 
                mMicroCode.size(),
                mDevice.GetClassLinkage(),
                mDomainShader.ReleaseAndGetAddressOf());

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 Domain shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreateDomainShader");
            }
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 Domain shader '" + mName + "' was not loaded.");
        }
    }
    //-----------------------------------------------------------------------
    void D3D11HLSLProgram::CreateComputeShader()
    {
        if (isSupported())
        {
            // Create the shader
            HRESULT hr = mDevice->CreateComputeShader( 
                &mMicroCode[0], 
                mMicroCode.size(),
                mDevice.GetClassLinkage(),
                mComputeShader.ReleaseAndGetAddressOf());

            if (FAILED(hr) || mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 Compute shader " + mName + " from microcode.\nError Description:" + errorDescription,
                    "D3D11HLSLProgram::CreateComputeShader");
            }
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D11 Compute shader '" + mName + "' was not loaded.");
        }
    }
    //-----------------------------------------------------------------------------
    unsigned int D3D11HLSLProgram::getSubroutineSlot(const String& subroutineSlotName) const
    {
        SlotIterator it = mSlotMap.find(subroutineSlotName);
        if (it == mSlotMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Don't exist interface slot with name " + subroutineSlotName + " in shader " + mName,
                        "D3D11HLSLProgram::getInterfaceSlot");
        }

        return it->second;
    }
    //-----------------------------------------------------------------------------
    ID3D11Buffer* D3D11HLSLProgram::getConstantBuffer(GpuProgramParametersSharedPtr params, uint16 variabilityMask)
    {
        // Update the Constant Buffer
        
		if(!mBufferInfoMap.empty())
        {
			BufferInfoIterator it = mBufferInfoMap.begin();
            
            if (it->mUniformBuffer)
            {
                HardwareBufferLockGuard uniformLock(it->mUniformBuffer, HardwareBuffer::HBL_DISCARD);

                // Only iterate through parsed variables (getting size of list)
                void* src = 0;
                ShaderVarWithPosInBuf* iter = &(it->mShaderVars[0]);
                unsigned int lSize = it->mShaderVars.size();
                for (size_t i = 0 ; i < lSize; i++, iter++)
                {
                    const GpuConstantDefinition& def = params->getConstantDefinition(iter->name);
                    // Since we are mapping with write discard, contents of the buffer are undefined.
                    // We must set every variable, even if it has not changed.
                    //if (def.variability & variabilityMask)
                    {
                        if(def.isFloat())
                        {
                            src = (void *)&(*(params->getFloatConstantList().begin() + def.physicalIndex));
                        }
                        else if (def.isInt() || def.isUnsignedInt())
                        {
                            src = (void *)&(*(params->getIntConstantList().begin() + def.physicalIndex));
                        }
                        else
                        {
                            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
                                        "Currently the only supported variables for Direct3D11 hlsl program are: 'float', 'int' and ' unsigned int'", 
                                        "D3D11HLSLProgram::getConstantBuffer");
                        }
                        


                        memcpy( &(((char *)(uniformLock.pData))[iter->startOffset]), src , iter->size);
                    }
                }

                return it->mUniformBuffer->_getImpl<D3D11HardwareBuffer>()->getD3DBuffer();
            }
        }

        return NULL;
    }
    //-----------------------------------------------------------------------------
    void D3D11HLSLProgram::getConstantBuffers(ID3D11Buffer** buffers, unsigned int& numBuffers,
                                              ID3D11ClassInstance** classes, unsigned int& numClasses,
                                              GpuProgramParametersSharedPtr params, uint16 variabilityMask)
    {
        // Update the Constant Buffers
        BufferInfoIterator it = mBufferInfoMap.begin();
        BufferInfoIterator end = mBufferInfoMap.end();
        while (it != end)
        {
            if (it->mUniformBuffer)
            {
                HardwareBufferLockGuard uniformLock(it->mUniformBuffer, HardwareBuffer::HBL_DISCARD);

                // Only iterate through parsed variables (getting size of list)
                void* src = 0;
                ShaderVarWithPosInBuf* iter = &(it->mShaderVars[0]);
                unsigned int lSize = it->mShaderVars.size();
                for (size_t i = 0 ; i < lSize; i++, iter++)
                {
                    const GpuConstantDefinition& def = params->getConstantDefinition(iter->name);
                    // Since we are mapping with write discard, contents of the buffer are undefined.
                    // We must set every variable, even if it has not changed.
                    //if (def.variability & variabilityMask)
                    {
                        if(def.isFloat())
                        {
                            src = (void *)&(*(params->getFloatConstantList().begin() + def.physicalIndex));
                        }
                        else
                        {
                            src = (void *)&(*(params->getIntConstantList().begin() + def.physicalIndex));
                        }

                        memcpy( &(((char *)(uniformLock.pData))[iter->startOffset]), src , iter->size);
                    }
                }

                // Add buffer to list
                buffers[numBuffers] = it->mUniformBuffer->_getImpl<D3D11HardwareBuffer>()->getD3DBuffer();
                // Increment number of buffers
                numBuffers++;
            }
        }

        // Update class instances
        SlotIterator sit = mSlotMap.begin();
        SlotIterator send = mSlotMap.end();
        while (sit != send)
        {
            // Get constant name
            const GpuConstantDefinition& def = params->getConstantDefinition(sit->first);

            // Set to correct slot
            //classes[sit->second] = 
            
            // Increment class count
            numClasses++;
        }
    }
    //-----------------------------------------------------------------------------
    ID3D11VertexShader* D3D11HLSLProgram::getVertexShader(void) const 
    { 
        assert(mType == GPT_VERTEX_PROGRAM);
        assert(mVertexShader);
        return mVertexShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    ID3D11PixelShader* D3D11HLSLProgram::getPixelShader(void) const 
    { 
        assert(mType == GPT_FRAGMENT_PROGRAM);
        assert(mPixelShader);
        return mPixelShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    ID3D11GeometryShader* D3D11HLSLProgram::getGeometryShader(void) const 
    { 
        assert(mType == GPT_GEOMETRY_PROGRAM);
        assert(mGeometryShader);
        return mGeometryShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    ID3D11DomainShader* D3D11HLSLProgram::getDomainShader(void) const 
    { 
        assert(mType == GPT_DOMAIN_PROGRAM);
        assert(mDomainShader);
        return mDomainShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    ID3D11HullShader* D3D11HLSLProgram::getHullShader(void) const 
    { 
        assert(mType == GPT_HULL_PROGRAM);
        assert(mHullShader);
        return mHullShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    ID3D11ComputeShader* D3D11HLSLProgram::getComputeShader(void) const 
    { 
        assert(mType == GPT_COMPUTE_PROGRAM);
        assert(mComputeShader);
        return mComputeShader.Get(); 
    }
    //-----------------------------------------------------------------------------
    const MicroCode & D3D11HLSLProgram::getMicroCode(void) const 
    { 
        assert(mMicroCode.size() > 0);
        return mMicroCode; 
    }
    //-----------------------------------------------------------------------------
    unsigned int D3D11HLSLProgram::getNumInputs( void ) const
    {
        return mD3d11ShaderInputParameters.size();
    }
    //-----------------------------------------------------------------------------
    unsigned int D3D11HLSLProgram::getNumOutputs( void ) const
    {
        return mD3d11ShaderOutputParameters.size();
    }
    //-----------------------------------------------------------------------------
    uint32 D3D11HLSLProgram::getNameForMicrocodeCache()
    {
        uint32 seed = FastHash("D3D11", 5); // shaders are identical to D3D9 & Cg
        return _getHash(seed);
    }


}
