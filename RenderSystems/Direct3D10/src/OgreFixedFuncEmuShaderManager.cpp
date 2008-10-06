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
#include "OgreRoot.h"
#include "OgreFixedFuncEmuShaderManager.h"
#include "OgreFixedFuncEmuShaderGenerator.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	FixedFuncEmuShaderManager::FixedFuncEmuShaderManager()
	{

	}
	//---------------------------------------------------------------------
	FixedFuncEmuShaderManager::~FixedFuncEmuShaderManager()
	{
		for (size_t i = 0 ; i < mProgramsToDeleteAtTheEnd.size() ; i++)
		{
			delete mProgramsToDeleteAtTheEnd[i];
		}
	}
	//---------------------------------------------------------------------
	void FixedFuncEmuShaderManager::registerGenerator( FixedFuncEmuShaderGenerator * generator )
	{
		mFixedFuncEmuShaderGeneratorMap[generator->getName()] = generator;
	}
	//---------------------------------------------------------------------
	void FixedFuncEmuShaderManager::unregisterGenerator( FixedFuncEmuShaderGenerator * generator )
	{
		mFixedFuncEmuShaderGeneratorMap.erase(generator->getName());
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms * FixedFuncEmuShaderManager::_createShaderPrograms( const String & generatorName, 
		const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{

		const String vertexProgramName = "VS";
		const String fragmentProgramName = "FP";

		FixedFuncEmuShaderGenerator * fixedFuncEmuShaderGenerator = mFixedFuncEmuShaderGeneratorMap[generatorName];
		String shaderSource = fixedFuncEmuShaderGenerator->getShaderSource(
			vertexProgramName,
			fragmentProgramName,
			vertexBufferDeclaration,
			fixedFuncState
			);

		// Vertex program details
		GpuProgramUsage * vertexProgramUsage = new GpuProgramUsage(GPT_VERTEX_PROGRAM);
		// Fragment program details
		GpuProgramUsage * fragmentProgramUsage = new GpuProgramUsage(GPT_FRAGMENT_PROGRAM);


		HighLevelGpuProgramPtr vs;
		HighLevelGpuProgramPtr fs;

		class LoadFromSourceGpuProgram : public HighLevelGpuProgram
		{
		public:
			void doLoadFromSource(void)
			{
				loadFromSource();
			};
		};

		static size_t shaderCount = 0;
		shaderCount++;
		vs = HighLevelGpuProgramManager::getSingleton().
			createProgram("VS_" + StringConverter::toString(shaderCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			fixedFuncEmuShaderGenerator->getLanguageName(), GPT_VERTEX_PROGRAM);	
		vs->setSource(shaderSource);
		vs->setParameter("entry_point",vertexProgramName);
		vs->setParameter("target",fixedFuncEmuShaderGenerator->getVpTarget());
		static_cast<LoadFromSourceGpuProgram *>(vs.get())->doLoadFromSource();

		vertexProgramUsage->setProgram(GpuProgramPtr(vs));
		vertexProgramUsage->setParameters(vs->createParameters());

		fs = HighLevelGpuProgramManager::getSingleton().
			createProgram("FS_" + StringConverter::toString(shaderCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			fixedFuncEmuShaderGenerator->getLanguageName(), GPT_FRAGMENT_PROGRAM);	
		fs->setSource(shaderSource);
		fs->setParameter("entry_point",fragmentProgramName);
		fs->setParameter("target",fixedFuncEmuShaderGenerator->getFpTarget());
		static_cast<LoadFromSourceGpuProgram *>(fs.get())->doLoadFromSource();

		fragmentProgramUsage->setProgram(GpuProgramPtr(fs));
		fragmentProgramUsage->setParameters(fs->createParameters());

		FixedFuncPrograms * newPrograms = fixedFuncEmuShaderGenerator->createFixedFuncPrograms();
		mLanguage2State2Declaration2ProgramsMap[generatorName][fixedFuncState][vertexBufferDeclaration] = newPrograms;
		newPrograms->setVertexProgramUsage(vertexProgramUsage);
		newPrograms->setFragmentProgramUsage(fragmentProgramUsage);
		newPrograms->setFixedFuncState(fixedFuncState);
#ifdef OGRE_DEBUG_MODE
		newPrograms->ShaderSource=shaderSource;
#endif

		mProgramsToDeleteAtTheEnd.push_back(newPrograms);
		return newPrograms;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms * FixedFuncEmuShaderManager::getShaderPrograms( const String & generatorName, 
		const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{
		Language2State2Declaration2ProgramsMap::iterator langIter = mLanguage2State2Declaration2ProgramsMap.find(generatorName);
		if (langIter != mLanguage2State2Declaration2ProgramsMap.end())
		{
			State2Declaration2ProgramsMap::iterator fixedFuncStateIter = langIter->second.find(fixedFuncState);
			if (fixedFuncStateIter != langIter->second.end())
			{
				VertexBufferDeclaration2FixedFuncProgramsMap::iterator vertexBufferDeclarationIter = fixedFuncStateIter->second.find(vertexBufferDeclaration);
				if (vertexBufferDeclarationIter != fixedFuncStateIter->second.end())
				{
					return vertexBufferDeclarationIter->second;
				}
			}
		}

		// we didn't find so we will create
		return _createShaderPrograms(generatorName, vertexBufferDeclaration, fixedFuncState);
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	GpuProgramUsage * FixedFuncPrograms::getVertexProgramUsage() const 
	{
		return mVertexProgramUsage;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::setVertexProgramUsage( GpuProgramUsage * val )
	{
		mVertexProgramUsage = val;
		mVertexProgramParameters = mVertexProgramUsage->getParameters();
	}
	//---------------------------------------------------------------------
	GpuProgramUsage * FixedFuncPrograms::getFragmentProgramUsage() const
	{
		return mFragmentProgramUsage;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::setFragmentProgramUsage( GpuProgramUsage * val )
	{
		mFragmentProgramUsage = val;
		mFragmentProgramParameters = mFragmentProgramUsage->getParameters();
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::FixedFuncPrograms() 
		: mVertexProgramUsage(NULL), 
		mFragmentProgramUsage(NULL)
	{

	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::~FixedFuncPrograms()
	{
		if (mVertexProgramUsage)
		{
			delete(mVertexProgramUsage);
		}
		if (mFragmentProgramUsage)
		{
			delete(mFragmentProgramUsage);
		}	
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramParameter( const GpuProgramType type, const String paramName, const void * value, const size_t sizeInBytes )
	{
		switch(type)
		{
		case GPT_VERTEX_PROGRAM: 
			_updateParameter(getVertexProgramUsageParameters(), paramName, value, sizeInBytes);
			break;
		case GPT_FRAGMENT_PROGRAM: 
			_updateParameter(getFragmentProgramUsageParameters(), paramName, value, sizeInBytes);
			break;

		}
		

	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramintParameter( const GpuProgramType type, const String paramName, const int & value )
	{
		_setProgramParameter(type, paramName, &value, sizeof(int));
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramFloatParameter( const GpuProgramType type, const String paramName, const float & value )
	{
		_setProgramParameter(type, paramName, &value, sizeof(float));
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramMatrix4Parameter( const GpuProgramType type, const String paramName, const Matrix4 & value )
	{
		_setProgramParameter(type, paramName, &value, sizeof(Matrix4));
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramColorParameter( const GpuProgramType type, const String paramName, const ColourValue & value )
	{
		float valueAsFloat4[4];
		valueAsFloat4[0] = value[0];
		valueAsFloat4[1] = value[1];
		valueAsFloat4[2] = value[2];
		valueAsFloat4[3] = value[3];
		_setProgramParameter(type, paramName, &valueAsFloat4[0], sizeof(float) * 4);
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::_setProgramVector3Parameter( const GpuProgramType type, const String paramName, const Vector3 & value )
	{
		float valueAsFloat3[3];
		valueAsFloat3[0] = value[0];
		valueAsFloat3[1] = value[1];
		valueAsFloat3[2] = value[2];
		_setProgramParameter(type, paramName, &valueAsFloat3, sizeof(float) * 3);
	}
	//---------------------------------------------------------------------
	const FixedFuncState & FixedFuncPrograms::getFixedFuncState() const
	{
		return mFixedFuncState;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::setFixedFuncState( const FixedFuncState & val )
	{
		mFixedFuncState = val;
	}
	//---------------------------------------------------------------------
	GpuProgramParametersSharedPtr & FixedFuncPrograms::getVertexProgramUsageParameters()
	{
		return mVertexProgramParameters;
	}
	//---------------------------------------------------------------------
	GpuProgramParametersSharedPtr & FixedFuncPrograms::getFragmentProgramUsageParameters()
	{
		return mFragmentProgramParameters;
	}

	void FixedFuncPrograms::_updateParameter( GpuProgramParametersSharedPtr & programParameters, const String paramName, const void * value, const size_t sizeInBytes )
	{
		const GpuConstantDefinition& def = programParameters->getConstantDefinition(paramName);
		if (def.isFloat())
		{
			memcpy((programParameters->getFloatPointer(def.physicalIndex)), value, sizeInBytes);
		}
		else
		{
			memcpy((programParameters->getIntPointer(def.physicalIndex)), value, sizeInBytes);
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	const Matrix4 & FixedFuncPrograms::FixedFuncProgramsParameters::getWorldMat() const
	{
		return mWorldMat;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setWorldMat( const Matrix4 & val )
	{
		mWorldMat = val;
	}
	//---------------------------------------------------------------------
	const Matrix4 & FixedFuncPrograms::FixedFuncProgramsParameters::getProjectionMat() const
	{
		return mProjectionMat;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setProjectionMat( const Matrix4 & val )
	{
		mProjectionMat = val;
	}
	//---------------------------------------------------------------------
	const Matrix4 & FixedFuncPrograms::FixedFuncProgramsParameters::getViewMat() const
	{
		return mViewMat;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setViewMat( const Matrix4 & val )
	{
		mViewMat = val;
	}
	//---------------------------------------------------------------------
	const LightList & FixedFuncPrograms::FixedFuncProgramsParameters::getLights() const
	{
		return mLights;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setLights( const LightList & val )
	{
		mLights = val;
	}
	//---------------------------------------------------------------------
	const FogMode FixedFuncPrograms::FixedFuncProgramsParameters::getFogMode() const
	{
		return mFogMode;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setFogMode( const FogMode val )
	{
		mFogMode = val;
	}
	//---------------------------------------------------------------------
	const ColourValue FixedFuncPrograms::FixedFuncProgramsParameters::getFogColour() const
	{
		return mFogColour;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setFogColour( const ColourValue val )
	{
		mFogColour = val;
	}
	//---------------------------------------------------------------------
	const Real FixedFuncPrograms::FixedFuncProgramsParameters::getFogDensitiy() const
	{
		return mFogDensitiy;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setFogDensitiy( const Real val )
	{
		mFogDensitiy = val;
	}
	//---------------------------------------------------------------------
	const Real FixedFuncPrograms::FixedFuncProgramsParameters::getFogStart() const
	{
		return mFogStart;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setFogStart( const Real val )
	{
		mFogStart = val;
	}
	//---------------------------------------------------------------------
	const Real FixedFuncPrograms::FixedFuncProgramsParameters::getFogEnd() const
	{
		return mFogEnd;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setFogEnd( const Real val )
	{
		mFogEnd = val;
	}
	//---------------------------------------------------------------------
	const bool FixedFuncPrograms::FixedFuncProgramsParameters::getLightingEnabled() const
	{
		return mLightingEnabled;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setLightingEnabled( const bool val )
	{
		mLightingEnabled = val;
	}
	//---------------------------------------------------------------------
	const ColourValue & FixedFuncPrograms::FixedFuncProgramsParameters::getLightAmbient() const
	{
		return mLightAmbient;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setLightAmbient( const ColourValue val )
	{
		mLightAmbient = val;
	}
	//---------------------------------------------------------------------
	const FixedFuncPrograms::FixedFuncProgramsParameters::TextureMatrixVector & FixedFuncPrograms::FixedFuncProgramsParameters::getTextureMatrices() const
	{
		return mTextureMatrices;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setTextureMatrix( const size_t index, const Matrix4 & val )
	{
		// resize
		while (index + 1> mTextureMatrices.size())
		{
			mTextureMatrices.push_back(Matrix4::IDENTITY);
		}
		mTextureMatrices[index] = val;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::FixedFuncProgramsParameters::FixedFuncProgramsParameters()
	{
		mFogMode = FOG_NONE;
		mFogColour = ColourValue::Black;
		mFogDensitiy = 0.0f;
		mFogStart = 0.0f;
		mFogEnd = 0.0f;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::FixedFuncProgramsParameters::~FixedFuncProgramsParameters()
	{

	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::FixedFuncProgramsParameters::setTextureEnabled( const size_t index, const bool val )
	{
		// resize
		while (index + 1 > mTextureEnabledVector.size())
		{
			mTextureEnabledVector.push_back(false);
		}
		mTextureEnabledVector[index] = val;

	}
	//---------------------------------------------------------------------
	bool FixedFuncPrograms::FixedFuncProgramsParameters::isTextureStageEnabled( const size_t index ) const
	{
		if(index < mTextureEnabledVector.size())
		{
			return mTextureEnabledVector[index];
		}
		else
		{
			return false;
		}
			
	}
	//---------------------------------------------------------------------
}
