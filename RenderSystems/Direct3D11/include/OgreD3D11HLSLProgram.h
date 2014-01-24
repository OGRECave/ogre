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
#include "OgreHighLevelGpuProgram.h"
#include "OgreHardwareUniformBuffer.h"
#include "OgreD3D11VertexDeclaration.h"


namespace Ogre {
	typedef vector<byte>::type MicroCode;

	/** Specialization of HighLevelGpuProgram to provide support for D3D11 
	High-Level Shader Language (HLSL).
	@remarks
	Note that the syntax of D3D11 HLSL is identical to nVidia's Cg language, therefore
	unless you know you will only ever be deploying on Direct3D, or you have some specific
	reason for not wanting to use the Cg plugin, I suggest you use Cg instead since that
	can produce programs for OpenGL too.
	*/
	class D3D11HLSLProgram : public HighLevelGpuProgram
	{
	public:
		/// Command object for setting entry point
		class CmdEntryPoint : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		/// Command object for setting target assembler
		class CmdTarget : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		/// Command object for setting macro defines
		class CmdPreprocessorDefines : public ParamCommand
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

	protected:

		static CmdEntryPoint msCmdEntryPoint;
		static CmdTarget msCmdTarget;
		static CmdPreprocessorDefines msCmdPreprocessorDefines;
		static CmdColumnMajorMatrices msCmdColumnMajorMatrices;
		static CmdEnableBackwardsCompatibility msCmdEnableBackwardsCompatibility;
		
		/** Internal method for creating an appropriate low-level program from this
		high-level program, must be implemented by subclasses. */
		void createLowLevelImpl(void);
		/// Internal unload implementation, must be implemented by subclasses
		void unloadHighLevelImpl(void);
		/// Populate the passed parameters with name->index map, must be overridden
		void populateParameterNames(GpuProgramParametersSharedPtr params);

		// Recursive utility method for populateParameterNames
		void processParamElement(String prefix, LPCSTR pName, ID3D11ShaderReflectionType* varRefType);

		void populateDef(D3D11_SHADER_TYPE_DESC& d3dDesc, GpuConstantDefinition& def) const;

		String mTarget;
		String mEntryPoint;
		String mPreprocessorDefines;
		bool mColumnMajorMatrices;
		bool mEnableBackwardsCompatibility;

		bool mErrorsInCompile;
		MicroCode mMicroCode;
		ID3D11Buffer* mConstantBuffer;
		
		D3D_SHADER_MACRO* mShaderMacros;
		bool shaderMacroSet;

		D3D11Device & mDevice;

		D3D11VertexDeclaration mInputVertexDeclaration;

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11GeometryShader* mGeometryShader;
		ID3D11DomainShader* mDomainShader;
		ID3D11HullShader* mHullShader;
		ID3D11ComputeShader* mComputeShader;

		struct ShaderVarWithPosInBuf
		{
			mutable String name;
			size_t size;
			size_t startOffset;
			
			ShaderVarWithPosInBuf& operator=(const ShaderVarWithPosInBuf& var)
			{
				name = var.name;
				size = var.size;
				startOffset = var.startOffset;
				return *this;
			}
		};
		typedef vector<ShaderVarWithPosInBuf>::type ShaderVars;
		typedef ShaderVars::iterator ShaderVarsIter;
		typedef ShaderVars::const_iterator ShaderVarsConstIter; 

		// A hack for cg to get the "original name" of the var in the "auto comments"
		// that cg adds to the hlsl 4 output. This is to solve the issue that
		// in some cases cg changes the name of the var to a new name.
		void fixVariableNameFromCg(const ShaderVarWithPosInBuf& newVar);
		//ShaderVars mShaderVars;
		
		// HACK: Multi-index emulation container to store constant buffer information by index and name at same time
		// using tips from http://www.boost.org/doc/libs/1_35_0/libs/multi_index/doc/performance.html
		// and http://cnx.org/content/m35767/1.2/
#define INVALID_IDX (unsigned int)-1
		struct BufferInfo
		{
			static _StringHash mHash;
			unsigned int mIdx;
			String mName;
			mutable HardwareUniformBufferSharedPtr mUniformBuffer;
			mutable ShaderVars mShaderVars;
				
			// Default constructor
			BufferInfo() : mIdx(0), mName("") { mUniformBuffer.setNull(); }
			BufferInfo(unsigned int index, const String& name)
				: mIdx(index), mName(name)
			{
				mUniformBuffer.setNull();
			}
			
			// Copy constructor
			BufferInfo(const BufferInfo& info) 
				: mIdx(info.mIdx)
				, mName(info.mName)
				, mUniformBuffer(info.mUniformBuffer)
				, mShaderVars(info.mShaderVars)
			{

			}

			// Copy operator
			BufferInfo& operator=(const BufferInfo& info)
			{
				this->mIdx = info.mIdx;
				this->mName = info.mName;
				mUniformBuffer = info.mUniformBuffer;
				mShaderVars = info.mShaderVars;
				return *this;
			}
			
			// Constructors and operators used for search
			BufferInfo(unsigned int index) : mIdx(index), mName("") { }
			BufferInfo(const String& name) : mIdx(INVALID_IDX), mName(name) { }
			BufferInfo& operator=(unsigned int index) { this->mIdx = index; return *this; }
			BufferInfo& operator=(const String& name) { this->mName = name; return *this; }	
			
			bool operator==(const BufferInfo& other) const
			{
				return mName == other.mName && mIdx == other.mIdx;
			}
			bool operator<(const BufferInfo& other) const
			{
				if (mIdx == INVALID_IDX || other.mIdx == INVALID_IDX) 
				{
					return mName < other.mName;
				}
				else if (mName == "" || other.mName == "")
				{
					return mIdx < other.mIdx;
				}
				else 
				{
					if (mName == other.mName)
					{
						return mIdx < other.mIdx;
					}
					else
					{
						return mName < other.mName;
					}
				}
			}
		};

		// Make sure that objects have index and name, or some search will fail
		typedef std::set<BufferInfo> BufferInfoMap;
		typedef std::set<BufferInfo>::iterator BufferInfoIterator;
		BufferInfoMap mBufferInfoMap;

		// Map to store interface slot position. 
		// Number of interface slots is size of this map.
		typedef std::map<String, unsigned int> SlotMap;
		typedef std::map<String, unsigned int>::const_iterator SlotIterator;
		SlotMap mSlotMap;

        typedef vector<D3D11_SIGNATURE_PARAMETER_DESC>::type D3d11ShaderParameters;
        typedef D3d11ShaderParameters::iterator D3d11ShaderParametersIter; 


        typedef vector<D3D11_SHADER_VARIABLE_DESC>::type D3d11ShaderVariables;
        typedef D3d11ShaderVariables::iterator D3d11ShaderVariablesIter; 

        struct GpuConstantDefinitionWithName : GpuConstantDefinition
        {
            LPCSTR                  Name;          
        };
        typedef vector<GpuConstantDefinitionWithName>::type D3d11ShaderVariableSubparts;
        typedef D3d11ShaderVariableSubparts::iterator D3d11ShaderVariableSubpartsIter; 

        typedef struct MemberTypeName
        {
            LPCSTR                  Name;           
        };

        vector<String *>::type mSerStrings;

		typedef vector<D3D11_SHADER_BUFFER_DESC>::type D3d11ShaderBufferDescs;
		typedef vector<D3D11_SHADER_TYPE_DESC>::type D3d11ShaderTypeDescs;
		typedef vector<UINT>::type InterfaceSlots;
		typedef vector<MemberTypeName>::type MemberTypeNames;

        UINT mConstantBufferSize;
        UINT mConstantBufferNr;
        UINT mNumSlots;
        ShaderVars mShaderVars;
        D3d11ShaderParameters mD3d11ShaderInputParameters;
        D3d11ShaderParameters mD3d11ShaderOutputParameters;
        D3d11ShaderVariables mD3d11ShaderVariables;
        D3d11ShaderVariableSubparts mD3d11ShaderVariableSubparts;
		D3d11ShaderBufferDescs mD3d11ShaderBufferDescs;
		D3d11ShaderVariables mVarDescBuffer;
		D3d11ShaderVariables mVarDescPointer;
		D3d11ShaderTypeDescs mD3d11ShaderTypeDescs;
		D3d11ShaderTypeDescs mMemberTypeDesc;
		MemberTypeNames mMemberTypeName;
		InterfaceSlots mInterfaceSlots;

		void createConstantBuffer(const UINT ByteWidth);
		void analizeMicrocode();
		void getMicrocodeFromCache(void);
		void compileMicrocode(void);
	public:
		D3D11HLSLProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, D3D11Device & device);
		~D3D11HLSLProgram();

		/** Sets the entry point for this program ie the first method called. */
		void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
		/** Gets the entry point defined for this program. */
		const String& getEntryPoint(void) const { return mEntryPoint; }
		/** Sets the shader target to compile down to, e.g. 'vs_1_1'. */
		void setTarget(const String& target);
		/** Gets the shader target to compile down to, e.g. 'vs_1_1'. */
		const String& getTarget(void) const { return mTarget; }
		/** Gets the shader target promoted to the first compatible, e.g. 'vs_4_0' or 'ps_4_0' if backward compatibility is enabled. */
		const String& getCompatibleTarget(void) const;
		/** Sets shader macros created manually*/
		void setShaderMacros(D3D_SHADER_MACRO* shaderMacros);

		/** Sets the preprocessor defines use to compile the program. */
		void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
		/** Sets the preprocessor defines use to compile the program. */
		const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }
		/** Sets whether matrix packing in column-major order. */ 
		void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
		/** Gets whether matrix packed in column-major order. */
		bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }
		/** Sets whether backwards compatibility is enabled. */ 
		void setEnableBackwardsCompatibility(bool enableBackwardsCompatibility) { mEnableBackwardsCompatibility = enableBackwardsCompatibility; }
		/** Gets whether backwards compatibility is enabled. */
		bool getEnableBackwardsCompatibility(void) const { return mEnableBackwardsCompatibility; }
		/// Overridden from GpuProgram
		bool isSupported(void) const;
		/// Overridden from GpuProgram
		GpuProgramParametersSharedPtr createParameters(void);
		/// Overridden from GpuProgram
		const String& getLanguage(void) const;

		virtual void buildConstantDefinitions() const;
		ID3D11VertexShader* getVertexShader(void) const;
		ID3D11PixelShader* getPixelShader(void) const; 
		ID3D11GeometryShader* getGeometryShader(void) const; 
		ID3D11DomainShader* getDomainShader(void) const;
		ID3D11HullShader* getHullShader(void) const;
		ID3D11ComputeShader* getComputeShader(void) const;
		const MicroCode &  getMicroCode(void) const;  

		ID3D11Buffer* getConstantBuffer(GpuProgramParametersSharedPtr params, uint16 variabilityMask);

		void getConstantBuffers(ID3D11Buffer** buffers, unsigned int& numBuffers,
								ID3D11ClassInstance** classes, unsigned int& numInstances,
								GpuProgramParametersSharedPtr params, uint16 variabilityMask);

		// Get slot for a specific interface
		unsigned int getSubroutineSlot(const String& subroutineSlotName) const;

		void CreateVertexShader();
		void CreatePixelShader();
		void CreateGeometryShader();
		void CreateDomainShader();
		void CreateHullShader();
		void CreateComputeShader();

		/** Internal load implementation, must be implemented by subclasses.
		*/
		void loadFromSource(void);

		D3D11VertexDeclaration & getInputVertexDeclaration() { return mInputVertexDeclaration; }

		void reinterpretGSForStreamOut(void);
		bool mReinterpretingGS;
        
		unsigned int getNumInputs(void)const;
		unsigned int getNumOutputs(void)const;

		String getNameForMicrocodeCache();

		const D3D11_SIGNATURE_PARAMETER_DESC & getInputParamDesc(unsigned int index) const;
		const D3D11_SIGNATURE_PARAMETER_DESC & getOutputParamDesc(unsigned int index) const;	
	};
}

#endif
