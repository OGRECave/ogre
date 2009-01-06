/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __FixedFuncEmuShaderManager_H__
#define __FixedFuncEmuShaderManager_H__

#include "OgreTextureManager.h"
#include "OgreGpuProgramUsage.h"
#include "OgreFixedFuncState.h"
#include "OgreGpuProgram.h"

namespace Ogre 
{
	class FixedFuncEmuShaderGenerator;
	
	class FixedFuncPrograms
	{
	#ifdef OGRE_DEBUG_MODE
	public:
			String ShaderSource;
	#endif

	protected:
		// Vertex program details
		GpuProgramUsage * mVertexProgramUsage;
		GpuProgramParametersSharedPtr mVertexProgramParameters;
		// Fragment program details
		GpuProgramUsage * mFragmentProgramUsage;
		GpuProgramParametersSharedPtr mFragmentProgramParameters;

		FixedFuncState mFixedFuncState;
		void _setProgramParameter(const GpuProgramType type, const String paramName, const void * value, const size_t sizeInBytes);

		void _updateParameter( GpuProgramParametersSharedPtr & programParameters, const String paramName, const void * value, const size_t sizeInBytes );
		void _setProgramintParameter(const GpuProgramType type, const String paramName, const int & value);
		void _setProgramFloatParameter(const GpuProgramType type, const String paramName, const float & value);
		void _setProgramMatrix4Parameter(const GpuProgramType type, const String paramName, const Matrix4 & value);
		void _setProgramColorParameter(const GpuProgramType type, const String paramName, const ColourValue & value);
		void _setProgramVector3Parameter(const GpuProgramType type, const String paramName, const Vector3 & value);
	public:

		class FixedFuncProgramsParameters
		{
		public:
			typedef vector<Matrix4>::type  TextureMatrixVector;
			typedef vector<bool>::type  TextureEnabledVector;
		protected:
			Matrix4 mWorldMat;
			Matrix4 mProjectionMat;
			Matrix4 mViewMat;
			
			bool mLightingEnabled;
			LightList mLights;
			ColourValue mLightAmbient;

			FogMode mFogMode;
			ColourValue mFogColour;
			Real mFogDensitiy;
			Real mFogStart;
			Real mFogEnd;

			TextureMatrixVector mTextureMatrices;
			TextureEnabledVector mTextureEnabledVector;

		public:
			FixedFuncProgramsParameters();
			~FixedFuncProgramsParameters();
			

			const Matrix4 & getWorldMat() const;
			void setWorldMat(const Matrix4 & val);
			const Matrix4 & getProjectionMat() const;
			void setProjectionMat(const Matrix4 & val);
			const Matrix4 & getViewMat() const;
			void setViewMat(const Matrix4 & val);
			const LightList & getLights() const;
			void setLights(const LightList & val);
			const FogMode getFogMode() const;
			void setFogMode(const FogMode val);
			const ColourValue getFogColour() const;
			void setFogColour(const ColourValue val);
			const Real getFogDensitiy() const;
			void setFogDensitiy(const Real val);
			const Real getFogStart() const;
			void setFogStart(const Real val);
			const Real getFogEnd() const;
			void setFogEnd(const Real val);
			const bool getLightingEnabled() const;
			void setLightingEnabled(const bool val);
			const ColourValue & getLightAmbient() const;
			void setLightAmbient(const ColourValue val);

			const TextureMatrixVector & getTextureMatrices() const;
			void setTextureMatrix(const size_t index, const Matrix4 & val);

			void setTextureEnabled(const size_t index, const bool val);
			bool isTextureStageEnabled(const size_t index) const;

		};

		FixedFuncPrograms();
		virtual ~FixedFuncPrograms();
		GpuProgramUsage * getVertexProgramUsage() const;
		GpuProgramParametersSharedPtr & getVertexProgramUsageParameters();
		void setVertexProgramUsage(GpuProgramUsage * val);
		GpuProgramUsage * getFragmentProgramUsage() const;
		GpuProgramParametersSharedPtr & getFragmentProgramUsageParameters();
		void setFragmentProgramUsage(GpuProgramUsage * val);
		const FixedFuncState & getFixedFuncState() const;
		void setFixedFuncState(const FixedFuncState & val);

		virtual void setFixedFuncProgramsParameters(const FixedFuncProgramsParameters & val) = 0;
	};

	class FixedFuncEmuShaderManager
	{
	protected:
		typedef map<String, FixedFuncEmuShaderGenerator *>::type FixedFuncEmuShaderGeneratorMap;
		FixedFuncEmuShaderGeneratorMap mFixedFuncEmuShaderGeneratorMap;

		typedef map<VertexBufferDeclaration, FixedFuncPrograms *>::type VertexBufferDeclaration2FixedFuncProgramsMap;
		typedef map<FixedFuncState, VertexBufferDeclaration2FixedFuncProgramsMap>::type State2Declaration2ProgramsMap;
		typedef map<String, State2Declaration2ProgramsMap>::type Language2State2Declaration2ProgramsMap;
		Language2State2Declaration2ProgramsMap mLanguage2State2Declaration2ProgramsMap;

		vector<FixedFuncPrograms *>::type mProgramsToDeleteAtTheEnd;

		FixedFuncPrograms * _createShaderPrograms(const String & generatorName,
			const VertexBufferDeclaration & vertexBufferDeclaration, 
			FixedFuncState &  fixedFuncState);
	public:
		FixedFuncEmuShaderManager();
		virtual ~FixedFuncEmuShaderManager();

		void registerGenerator(FixedFuncEmuShaderGenerator * generator);
		void unregisterGenerator(FixedFuncEmuShaderGenerator * generator);

		FixedFuncPrograms * getShaderPrograms(const String & generatorName,
			const VertexBufferDeclaration & vertexBufferDeclaration, 
			FixedFuncState &  fixedFuncState);

	};
}
#endif
