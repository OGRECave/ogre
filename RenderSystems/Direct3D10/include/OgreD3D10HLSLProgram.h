/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __D3D10HLSLProgram_H__
#define __D3D10HLSLProgram_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHighLevelGpuProgram.h"


namespace Ogre {
	/** Specialization of HighLevelGpuProgram to provide support for D3D10 
	High-Level Shader Language (HLSL).
	@remarks
	Note that the syntax of D3D10 HLSL is identical to nVidia's Cg language, therefore
	unless you know you will only ever be deploying on Direct3D, or you have some specific
	reason for not wanting to use the Cg plugin, I suggest you use Cg instead since that
	can produce programs for OpenGL too.
	*/
	class D3D10HLSLProgram : public HighLevelGpuProgram
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

	protected:

		static CmdEntryPoint msCmdEntryPoint;
		static CmdTarget msCmdTarget;
		static CmdPreprocessorDefines msCmdPreprocessorDefines;
		static CmdColumnMajorMatrices msCmdColumnMajorMatrices;

		/** Internal method for creating an appropriate low-level program from this
		high-level program, must be implemented by subclasses. */
		void createLowLevelImpl(void);
		/// Internal unload implementation, must be implemented by subclasses
		void unloadHighLevelImpl(void);
		/// Populate the passed parameters with name->index map, must be overridden
		//void populateParameterNames(GpuProgramParametersSharedPtr params);

		// Recursive utility method for populateParameterNames
		void processParamElement(String prefix, LPCSTR pName, size_t paramIndex, ID3D10ShaderReflectionType* varRefType) const;

		void populateDef(D3D10_SHADER_TYPE_DESC& d3dDesc, GpuConstantDefinition& def) const;

		String mTarget;
		String mEntryPoint;
		String mPreprocessorDefines;
		bool mColumnMajorMatrices;

		bool mErrorsInCompile;
		ID3D10Blob * mpMicroCode;
		ID3D10Buffer* mConstantBuffer;
		
		D3D10Device & mDevice;

		ID3D10ShaderReflectionConstantBuffer* mShaderReflectionConstantBuffer;
		D3D10_SHADER_BUFFER_DESC mConstantBufferDesc ;
		D3D10_SHADER_DESC mShaderDesc;

// this is a temp patch for the nov 08 DX SDK
#ifdef D3DX10ReflectShader 
		ID3D10ShaderReflection1* mpIShaderReflection;
#else
		ID3D10ShaderReflection* mpIShaderReflection;
#endif

		ID3D10VertexShader* mpVertexShader;
		ID3D10PixelShader* mpPixelShader;
		ID3D10GeometryShader* mpGeometryShader;

		struct ShaderVarWithPosInBuf
		{
			bool wasInit;
			bool isFloat;
			size_t physicalIndex;
			void * src;

			D3D10_SHADER_VARIABLE_DESC var;
		};
		typedef vector<ShaderVarWithPosInBuf>::type ShaderVars;
		typedef ShaderVars::iterator ShaderVarsIter; 

		ShaderVars mShaderVars;

		void createConstantBuffer(const UINT ByteWidth);
	public:
		D3D10HLSLProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, D3D10Device & device);
		~D3D10HLSLProgram();

		/** Sets the entry point for this program ie the first method called. */
		void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
		/** Gets the entry point defined for this program. */
		const String& getEntryPoint(void) const { return mEntryPoint; }
		/** Sets the shader target to compile down to, e.g. 'vs_1_1'. */
		void setTarget(const String& target);
		/** Gets the shader target to compile down to, e.g. 'vs_1_1'. */
		const String& getTarget(void) const { return mTarget; }
		/** Sets the preprocessor defines use to compile the program. */
		void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
		/** Sets the preprocessor defines use to compile the program. */
		const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }
		/** Sets whether matrix packing in column-major order. */ 
		void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
		/** Gets whether matrix packed in column-major order. */
		bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }
		/// Overridden from GpuProgram
		bool isSupported(void) const;
		/// Overridden from GpuProgram
		GpuProgramParametersSharedPtr createParameters(void);
		/// Overridden from GpuProgram
		const String& getLanguage(void) const;

		virtual void buildConstantDefinitions() const;
		ID3D10VertexShader* getVertexShader(void) const;
		ID3D10PixelShader* getPixelShader(void) const; 
		ID3D10GeometryShader* getGeometryShader(void) const; 
		ID3D10Blob * getMicroCode(void) const;  

		ID3D10Buffer* getConstantBuffer(GpuProgramParametersSharedPtr params);

		void CreateVertexShader();
		void CreatePixelShader();
		void CreateGeometryShader();

		/** Internal load implementation, must be implemented by subclasses.
		*/
		void loadFromSource(void);

	};
}

#endif
