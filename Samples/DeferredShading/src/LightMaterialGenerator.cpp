/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "LightMaterialGenerator.h"

#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"

#include "OgrePass.h"
#include "OgreTechnique.h"

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "MLight.h"

using namespace Ogre;

class LightMaterialGeneratorImpl : public MaterialGenerator::Impl
{
public:
	typedef MaterialGenerator::Perm Perm;
	LightMaterialGeneratorImpl(const String &baseName):
	mBaseName(baseName)
	{

	}
	virtual ~LightMaterialGeneratorImpl()
	{

	}
	virtual GpuProgramPtr generateVertexShader(Perm permutation)
	{
		String strUnifiedProgram;
		if (permutation & MLight::MI_QUAD)
		{
			strUnifiedProgram = "DeferredShading/post/vs";
		}
		else
		{
			strUnifiedProgram = "DeferredShading/post/LightMaterial_vs";
		}
		GpuProgramPtr ptr = HighLevelGpuProgramManager::getSingleton().getByName(strUnifiedProgram);
		assert(ptr->getLanguage()=="unified");
		return ptr;
	}
	virtual GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		assert(false && "Concrete Class must Implement!");
		GpuProgramPtr ptr;
		return ptr;
	}
	virtual MaterialPtr generateTemplateMaterial(Perm permutation)
	{
		if(permutation & MLight::MI_QUAD)
		{
			return MaterialManager::getSingleton().getByName("DeferredShading/LightMaterialQuad");
		}
		else
		{
			return MaterialManager::getSingleton().getByName("DeferredShading/LightMaterial");
		}
	}
	protected:
		String mBaseName;
		// Utility method
		String getPPDefines(Perm permutation)
		{
			String strPPD;
			if (permutation & MLight::MI_SPECULAR)
			{
				strPPD += "IS_SPECULAR=1,";
			}
			else
			{
				strPPD += "IS_SPECULAR=0,";
			}
			if (permutation & MLight::MI_ATTENUATED)
			{
				strPPD += "IS_ATTENUATED=1";
			}
			else
			{
				strPPD += "IS_ATTENUATED=0";
			}
			return strPPD;
		}
		void setUpBaseParameters(const GpuProgramParametersSharedPtr& params)
		{
			assert(params.isNull()==false);

			if (params->_findNamedConstantDefinition("worldView"))
			{
				params->setNamedAutoConstant("worldView", GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
			}
			if (params->_findNamedConstantDefinition("invProj"))
			{
				params->setNamedAutoConstant("invProj", GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX);
			}
			if (params->_findNamedConstantDefinition("lightDiffuseColor"))
			{
				params->setNamedAutoConstant("lightDiffuseColor", GpuProgramParameters::ACT_CUSTOM, 1);
			}
			if (params->_findNamedConstantDefinition("lightSpecularColor"))
			{
				params->setNamedAutoConstant("lightSpecularColor", GpuProgramParameters::ACT_CUSTOM, 2);
			}			
			if(params->_findNamedConstantDefinition("lightFalloff"))
			{
				params->setNamedAutoConstant("lightFalloff", GpuProgramParameters::ACT_CUSTOM, 3);
			}
		}
};

class LightMaterialGeneratorHLSL: public LightMaterialGeneratorImpl
{
public:
	LightMaterialGeneratorHLSL(const String &baseName):
	  LightMaterialGeneratorImpl(baseName)
	  {

	  }
	  virtual ~LightMaterialGeneratorHLSL()
	  {

	  }

	virtual GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		
		/// Create shader
		if (mMasterSource.empty())
		{
			DataStreamPtr ptrMasterSource = ResourceGroupManager::getSingleton().openResource(
				 "DeferredShading/post/hlsl/LightMaterial_ps.hlsl"
				, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			assert(ptrMasterSource.isNull()==false);
			mMasterSource = ptrMasterSource->getAsString();
		}

		assert(mMasterSource.empty()==false);

		// Create name
		String name=mBaseName+StringConverter::toString(permutation)+"_ps";		

		// Create shader object
		HighLevelGpuProgramPtr ptrProgram = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"hlsl", GPT_FRAGMENT_PROGRAM);
		ptrProgram->setSource(mMasterSource);
		ptrProgram->setParameter("target","ps_2_0");
		ptrProgram->setParameter("entry_point","main");
		
		// set up the preprocessor defines
		// Important to do this before any call to get parameters, i.e. before the program gets loaded
		ptrProgram->setParameter("preprocessor_defines", getPPDefines(permutation));

		setUpBaseParameters(ptrProgram->getDefaultParameters());

		return GpuProgramPtr(ptrProgram);
	}
protected:

	static String mMasterSource;
};

String LightMaterialGeneratorHLSL::mMasterSource = "";

class LightMaterialGeneratorGLSL: public LightMaterialGeneratorImpl
{
public:
	LightMaterialGeneratorGLSL(const String &baseName):
	  LightMaterialGeneratorImpl(baseName)
	  {

	  }
	  virtual ~LightMaterialGeneratorGLSL()
	  {

	  }

	virtual GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		/// Create shader
		if (mMasterSource.empty())
		{
			DataStreamPtr ptrMasterSource = ResourceGroupManager::getSingleton().openResource(
				 "DeferredShading/post/glsl/LightMaterial_ps.glsl"
				, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			assert(ptrMasterSource.isNull()==false);
			mMasterSource = ptrMasterSource->getAsString();
		}

		assert(mMasterSource.empty()==false);

		// Create name
		String name=mBaseName+StringConverter::toString(permutation)+"_ps";		

		// Create shader object
		HighLevelGpuProgramPtr ptrProgram = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"glsl", GPT_FRAGMENT_PROGRAM);
		ptrProgram->setSource(mMasterSource);

		// set up the preprocessor defines
		// Important to do this before any call to get parameters, i.e. before the program gets loaded
		ptrProgram->setParameter("preprocessor_defines", getPPDefines(permutation));

		// set up common parameters
		setUpBaseParameters(ptrProgram->getDefaultParameters());

		// set up parameters specific to this implementation		
		ptrProgram->getDefaultParameters()->setNamedConstant("tex0", 0);
		ptrProgram->getDefaultParameters()->setNamedConstant("tex1", 1);

		return GpuProgramPtr(ptrProgram);
	}
protected:
	static String mMasterSource;
};

String LightMaterialGeneratorGLSL::mMasterSource = "";

LightMaterialGenerator::LightMaterialGenerator(const String &language)
{
	bitNames.push_back("Quad");		  // MI_QUAD
	bitNames.push_back("Attenuated"); // MI_ATTENUATED
	bitNames.push_back("Specular");   // MI_SPECULAR

	vsMask = 0x00000001;
	fsMask = 0x00000006;
	matMask = 0x00000001;
	
	materialBaseName = "DeferredShading/LightMaterial/";
	if(language=="hlsl")
		mImpl = new LightMaterialGeneratorHLSL("DeferredShading/LightMaterial/hlsl/");
	else
		mImpl = new LightMaterialGeneratorGLSL("DeferredShading/LightMaterial/glsl/");
}
LightMaterialGenerator::~LightMaterialGenerator()
{

}
