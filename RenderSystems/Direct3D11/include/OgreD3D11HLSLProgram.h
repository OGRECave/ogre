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
#ifndef __D3D11HLSLProgram_H__
#define __D3D11HLSLProgram_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreD3D11DeviceResource.h"
#include "OgreHighLevelGpuProgram.h"


namespace Ogre {
    typedef std::vector<byte> MicroCode;

    /** Specialization of HighLevelGpuProgram to provide support for D3D11 
    High-Level Shader Language (HLSL).

    Note that the syntax of D3D11 HLSL is identical to nVidia's Cg language, therefore
    unless you know you will only ever be deploying on Direct3D, or you have some specific
    reason for not wanting to use the Cg plugin, I suggest you use Cg instead since that
    can produce programs for OpenGL too.
    */
    class _OgreD3D11Export D3D11HLSLProgram
        : public HighLevelGpuProgram
        , protected D3D11DeviceResource
    {
    public:
        /// Command object for setting target assembler
        class CmdTarget : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting matrix packing in column-major order
        class CmdColumnMajorMatrices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting backwards compatibility
        class CmdEnableBackwardsCompatibility : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        typedef std::vector<D3D11_SIGNATURE_PARAMETER_DESC> D3d11ShaderParameters;
        typedef std::map<String, int> BufferInfoMap;
    protected:

        static CmdTarget msCmdTarget;
        static CmdColumnMajorMatrices msCmdColumnMajorMatrices;
        static CmdEnableBackwardsCompatibility msCmdEnableBackwardsCompatibility;
        
        void notifyDeviceLost(D3D11Device* device);
        void notifyDeviceRestored(D3D11Device* device);

        /// noop
        void createLowLevelImpl(void) override {}
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void) override;
        void unprepareImpl() override;

        static void populateDef(D3D11_SHADER_TYPE_DESC& d3dDesc, GpuConstantDefinition& def);

        bool mColumnMajorMatrices;
        bool mEnableBackwardsCompatibility;

        MicroCode mMicroCode;

        D3D11Device & mDevice;

        ComPtr<ID3D11VertexShader> mVertexShader;
        ComPtr<ID3D11PixelShader> mPixelShader;
        ComPtr<ID3D11GeometryShader> mGeometryShader;
        ComPtr<ID3D11DomainShader> mDomainShader;
        ComPtr<ID3D11HullShader> mHullShader;
        ComPtr<ID3D11ComputeShader> mComputeShader;

        // Make sure that objects have index and name, or some search will fail
        BufferInfoMap mBufferInfoMap;

        // Map to store interface slot position. 
        // Number of interface slots is size of this map.
        typedef std::map<String, unsigned int> SlotMap;
        typedef std::map<String, unsigned int>::const_iterator SlotIterator;
        SlotMap mSlotMap;

        typedef D3d11ShaderParameters::iterator D3d11ShaderParametersIter; 


        typedef std::vector<D3D11_SHADER_VARIABLE_DESC> D3d11ShaderVariables;
        typedef D3d11ShaderVariables::iterator D3d11ShaderVariablesIter; 

        struct MemberTypeName
        {
            String                  Name;
        };

        typedef std::vector<D3D11_SHADER_BUFFER_DESC> D3d11ShaderBufferDescs;
        typedef std::vector<D3D11_SHADER_TYPE_DESC> D3d11ShaderTypeDescs;
        typedef std::vector<UINT> InterfaceSlots;
        typedef std::vector<MemberTypeName> MemberTypeNames;

        UINT mConstantBufferSize;
        UINT mConstantBufferNr;
        UINT mNumSlots;
        D3d11ShaderParameters mD3d11ShaderInputParameters;
        D3d11ShaderParameters mD3d11ShaderOutputParameters;
        D3d11ShaderVariables mD3d11ShaderVariables;
        D3d11ShaderBufferDescs mD3d11ShaderBufferDescs;
        D3d11ShaderVariables mVarDescBuffer;
        D3d11ShaderVariables mVarDescPointer;
        D3d11ShaderTypeDescs mD3d11ShaderTypeDescs;
        D3d11ShaderTypeDescs mMemberTypeDesc;
        MemberTypeNames mMemberTypeName;
        InterfaceSlots mInterfaceSlots;

        void analizeMicrocode();
        void getMicrocodeFromCache(uint32 id);
        void compileMicrocode(void);
    public:
        D3D11HLSLProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, D3D11Device & device);
        ~D3D11HLSLProgram();

        /** Sets the shader target to compile down to, e.g. 'vs_1_1'. */
        void setTarget(const String& target);
        /** Gets the shader target to compile down to, e.g. 'vs_1_1'. */
        const String& getTarget(void) const { return mSyntaxCode; }
        /** Gets the shader target promoted to the first compatible, e.g. 'vs_4_0' or 'ps_4_0' if backward compatibility is enabled. */
        const char* getCompatibleTarget(void) const;

        /** Sets whether matrix packing in column-major order. */ 
        void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
        /** Gets whether matrix packed in column-major order. */
        bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }
        /** Sets whether backwards compatibility is enabled. */ 
        void setEnableBackwardsCompatibility(bool enableBackwardsCompatibility) { mEnableBackwardsCompatibility = enableBackwardsCompatibility; }
        /** Gets whether backwards compatibility is enabled. */
        bool getEnableBackwardsCompatibility(void) const { return mEnableBackwardsCompatibility; }
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);
        /// Overridden from GpuProgram
        const String& getLanguage(void) const;

        virtual void buildConstantDefinitions() override;
        ID3D11VertexShader* getVertexShader(void) const;
        ID3D11PixelShader* getPixelShader(void) const; 
        ID3D11GeometryShader* getGeometryShader(void) const; 
        ID3D11DomainShader* getDomainShader(void) const;
        ID3D11HullShader* getHullShader(void) const;
        ID3D11ComputeShader* getComputeShader(void) const;
        const MicroCode &  getMicroCode(void) const;  

        const BufferInfoMap& getBufferInfoMap() const { return mBufferInfoMap; }

        // Get slot for a specific interface
        unsigned int getSubroutineSlot(const String& subroutineSlotName) const;

        void CreateVertexShader();
        void CreatePixelShader();
        void CreateGeometryShader();
        void CreateDomainShader();
        void CreateHullShader();
        void CreateComputeShader();

        /// shortcut as we there is no low-level separation here
        GpuProgram* _getBindingDelegate(void) { return this; }

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);

        void prepareImpl();

        void reinterpretGSForStreamOut(void);
        bool mReinterpretingGS;
        
        uint32 getNameForMicrocodeCache();

        const D3d11ShaderParameters& getInputParams() const { return mD3d11ShaderInputParameters; }
    };
}

#endif
