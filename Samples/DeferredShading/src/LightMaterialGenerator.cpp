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

#include "DLight.h"

using namespace Ogre;

//CG
class LightMaterialGeneratorCG : public MaterialGenerator::Impl
{
public:
	typedef MaterialGenerator::Perm Perm;
	LightMaterialGeneratorCG(const String &baseName):
	    mBaseName(baseName) 
	{

	}
	virtual ~LightMaterialGeneratorCG()
	{

	}

	virtual GpuProgramPtr generateVertexShader(Perm permutation)
	{
        String programName = "DeferredShading/post/";

		if (permutation & LightMaterialGenerator::MI_DIRECTIONAL)
		{
			programName += "vs";
		}
		else
		{
			programName += "LightMaterial_vs";
		}

		GpuProgramPtr ptr = HighLevelGpuProgramManager::getSingleton().getByName(programName);
		assert(!ptr.isNull());
		return ptr;
	}

	virtual GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		/// Create shader
		if (mMasterSource.empty())
		{
			DataStreamPtr ptrMasterSource = ResourceGroupManager::getSingleton().openResource(
				 "DeferredShading/post/LightMaterial_ps.cg"
				, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			assert(ptrMasterSource.isNull()==false);
			mMasterSource = ptrMasterSource->getAsString();
		}

		assert(mMasterSource.empty()==false);

		// Create name
		String name = mBaseName+StringConverter::toString(permutation)+"_ps";		

		// Create shader object
		HighLevelGpuProgramPtr ptrProgram = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"cg", GPT_FRAGMENT_PROGRAM);
		ptrProgram->setSource(mMasterSource);
		ptrProgram->setParameter("entry_point","main");
	    ptrProgram->setParameter("profiles","ps_2_x arbfp1");
		// set up the preprocessor defines
		// Important to do this before any call to get parameters, i.e. before the program gets loaded
		ptrProgram->setParameter("compile_arguments", getPPDefines(permutation));

		setUpBaseParameters(ptrProgram->getDefaultParameters());

		return GpuProgramPtr(ptrProgram);
	}

	virtual MaterialPtr generateTemplateMaterial(Perm permutation)
	{
		String materialName = mBaseName;
	
        if(permutation & LightMaterialGenerator::MI_DIRECTIONAL)
		{   
			materialName += "Quad";
		}
		else
		{
			materialName += "Geometry";
		}

		if(permutation & LightMaterialGenerator::MI_SHADOW_CASTER)
		{
			materialName += "Shadow";
		}
		return MaterialManager::getSingleton().getByName(materialName);
	}

	protected:
		String mBaseName;
        String mMasterSource;
		// Utility method
		String getPPDefines(Perm permutation)
		{
			String strPPD;

			//Get the type of light
			String lightType;
			if (permutation & LightMaterialGenerator::MI_POINT)
			{
				lightType = "POINT";
			}
			else if (permutation & LightMaterialGenerator::MI_SPOTLIGHT)
			{
				lightType = "SPOT";
			}
			else if (permutation & LightMaterialGenerator::MI_DIRECTIONAL)
			{
				lightType = "DIRECTIONAL";
			}
			else
			{
				assert(false && "Permutation must have a light type");
			}
			strPPD += "-DLIGHT_TYPE=LIGHT_" + lightType + " ";

			//Optional parameters
            if (permutation & LightMaterialGenerator::MI_SPECULAR)
			{
				strPPD += "-DIS_SPECULAR ";
			}
			if (permutation & LightMaterialGenerator::MI_ATTENUATED)
			{
				strPPD += "-DIS_ATTENUATED ";
			}
			if (permutation & LightMaterialGenerator::MI_SHADOW_CASTER)
			{
				strPPD += "-DIS_SHADOW_CASTER ";
			}
			return strPPD;
		}

		void setUpBaseParameters(const GpuProgramParametersSharedPtr& params)
		{
			assert(params.isNull()==false);

			struct AutoParamPair { String name; GpuProgramParameters::AutoConstantType type; };	

			//A list of auto params that might be present in the shaders generated
			static const AutoParamPair AUTO_PARAMS[] = {
				{ "vpWidth",			GpuProgramParameters::ACT_VIEWPORT_WIDTH },
				{ "vpHeight",			GpuProgramParameters::ACT_VIEWPORT_HEIGHT },
				{ "worldView",			GpuProgramParameters::ACT_WORLDVIEW_MATRIX },
				{ "invProj",			GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX },
				{ "invView",			GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX },
				{ "flip",				GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING },
				{ "lightDiffuseColor",	GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR },
				{ "lightSpecularColor", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR },
				{ "lightFalloff",		GpuProgramParameters::ACT_LIGHT_ATTENUATION },
				{ "lightPos",			GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE },
				{ "lightDir",			GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE },
				{ "spotParams",			GpuProgramParameters::ACT_SPOTLIGHT_PARAMS },
				{ "farClipDistance",	GpuProgramParameters::ACT_FAR_CLIP_DISTANCE },
				{ "shadowViewProjMat",	GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX }
			};
			int numParams = sizeof(AUTO_PARAMS) / sizeof(AutoParamPair);

			for (int i=0; i<numParams; i++)
			{
				if (params->_findNamedConstantDefinition(AUTO_PARAMS[i].name))
				{
					params->setNamedAutoConstant(AUTO_PARAMS[i].name, AUTO_PARAMS[i].type);
				}
			}
		}
};

LightMaterialGenerator::LightMaterialGenerator()
{
	vsMask = 0x00000004;
	fsMask = 0x0000003F;
	matMask =	LightMaterialGenerator::MI_DIRECTIONAL | 
				LightMaterialGenerator::MI_SHADOW_CASTER;
	
	materialBaseName = "DeferredShading/LightMaterial/";
    mImpl = new LightMaterialGeneratorCG("DeferredShading/LightMaterial/");
}

LightMaterialGenerator::~LightMaterialGenerator()
{

}
