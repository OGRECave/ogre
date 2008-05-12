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

class LightMaterialGeneratorHLSL: public MaterialGenerator::Impl
{
public:
	LightMaterialGeneratorHLSL(const String &baseName):
		mBaseName(baseName)
	{}
	typedef MaterialGenerator::Perm Perm;

	virtual Ogre::GpuProgramPtr generateVertexShader(Perm permutation)
	{
		if(permutation & MLight::MI_QUAD)
		{
			return HighLevelGpuProgramManager::getSingleton().getByName("DeferredShading/post/hlsl/vs");
		}
		else
		{
			return HighLevelGpuProgramManager::getSingleton().getByName("DeferredShading/post/hlsl/LightMaterial_vs");
		}
	}
	virtual Ogre::GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		bool isAttenuated = permutation & MLight::MI_ATTENUATED;
		bool isSpecular = permutation & MLight::MI_SPECULAR;
		// bool isShadowed = perm&4;

		/// Create name
		String name=mBaseName+StringConverter::toString(permutation)+"_ps";
		/// Create shader
		std::stringstream shader;
		shader <<
		"sampler Tex0: register(s0);\n"
		"sampler Tex1: register(s1);\n"
		"float4x4 worldView;\n"
		// Attributes of light
		"float4 lightDiffuseColor;\n"
		"float4 lightSpecularColor;\n"
		"float4 lightFalloff;\n"
		"float4 main(float2 texCoord: TEXCOORD0, float3 projCoord: TEXCOORD1) : COLOR\n"
		"{\n"
		"    float4 a0 = tex2D(Tex0, texCoord); \n"// Attribute 0: Diffuse color+shininess
		"    float4 a1 = tex2D(Tex1, texCoord); \n"// Attribute 1: Normal+depth
		// Attributes
		"    float3 colour = a0.rgb;\n"
		"    float alpha = a0.a;"		// Specularity
		"    float distance = a1.w;"	// Distance from viewer (w)
		"    float3 normal = a1.xyz;\n"
		// Calculate position of texel in view space
		"    float3 position = projCoord*distance;\n"
		// Extract position in view space from worldView matrix
		"	 float3 lightPos = float3(worldView[0][3],worldView[1][3],worldView[2][3]);\n"
		// Calculate light direction and distance
		"    float3 lightVec = lightPos - position;\n"
		"    float len_sq = dot(lightVec, lightVec);\n"
		"    float len = sqrt(len_sq);\n"
		"    float3 lightDir = lightVec/len;\n"
		/// Calculate attenuation
		"    float attenuation = dot(lightFalloff, float3(1, len, len_sq));\n"
		/// Calculate diffuse colour
		"    float3 light_diffuse = max(0,dot(lightDir, normal)) * lightDiffuseColor;\n"
		/// Calculate specular component
		"    float3 viewDir = -normalize(position);\n"
		"    float3 h = normalize(viewDir + lightDir);\n"
		"    float3 light_specular = pow(dot(normal, h),32) * lightSpecularColor;\n"
		// Accumulate total lighting for this fragment
		"    float3 total_light_contrib;\n"
		"    total_light_contrib = light_diffuse;\n";
		if(isSpecular)
		{
			/// Calculate specular contribution
			shader << 
			"	 total_light_contrib += alpha * light_specular;\n";
		}
		if(isAttenuated)
		{
			shader <<
			"    return float4(total_light_contrib*colour/attenuation, 0);\n";
		}
		else
		{
			shader <<
			"    return float4(total_light_contrib*colour, 0);\n";
		}
		shader <<
		"}\n";
		
		/// Create shader object
		HighLevelGpuProgramPtr program = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"hlsl", GPT_FRAGMENT_PROGRAM);
		program->setSource(shader.str());
		program->setParameter("target","ps_2_0");
		program->setParameter("entry_point","main");
		/// Set up default parameters
		GpuProgramParametersSharedPtr params = program->getDefaultParameters();
		params->setNamedAutoConstant("worldView", GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
		params->setNamedAutoConstant("lightDiffuseColor", GpuProgramParameters::ACT_CUSTOM, 1);
		if(isSpecular)
			params->setNamedAutoConstant("lightSpecularColor", GpuProgramParameters::ACT_CUSTOM, 2);
		if(isAttenuated)
			params->setNamedAutoConstant("lightFalloff", GpuProgramParameters::ACT_CUSTOM, 3);

		return HighLevelGpuProgramManager::getSingleton().getByName(program->getName());
	}
	virtual Ogre::MaterialPtr generateTemplateMaterial(Perm permutation)
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
};

class LightMaterialGeneratorGLSL: public MaterialGenerator::Impl
{
public:
	LightMaterialGeneratorGLSL(const String &baseName):
		mBaseName(baseName)
	{}
	typedef MaterialGenerator::Perm Perm;

	virtual Ogre::GpuProgramPtr generateVertexShader(Perm permutation)
	{
		if(permutation & MLight::MI_QUAD)
		{
			return HighLevelGpuProgramManager::getSingleton().getByName("DeferredShading/post/glsl/vs");
		}
		else
		{
			return HighLevelGpuProgramManager::getSingleton().getByName("DeferredShading/post/glsl/LightMaterial_vs");
		}
	}
	virtual Ogre::GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		bool isAttenuated = permutation & MLight::MI_ATTENUATED;
		bool isSpecular = permutation & MLight::MI_SPECULAR;
		// bool isShadowed = perm&4;

		/// Create name
		String name=mBaseName+StringConverter::toString(permutation)+"_ps";
		/// Create shader
		std::stringstream shader;
		shader <<
		"uniform sampler2D tex0;\n"
		"uniform sampler2D tex1;\n"
		"varying vec2 texCoord;\n"
		"varying vec3 projCoord;\n"
		/// World view matrix to get object position in view space
		"uniform mat4 worldView;\n"
		/// Attributes of light
		"uniform vec3 lightDiffuseColor;\n"
		"uniform vec3 lightSpecularColor;\n"
		"uniform vec3 lightFalloff;\n"
		"void main()\n"
		"{\n"
		"	 vec4 a0 = texture2D(tex0, texCoord);\n" // Attribute 0: Diffuse color+shininess
		"    vec4 a1 = texture2D(tex1, texCoord);\n" // Attribute 1: Normal+depth
		/// Attributes
		"    vec3 colour = a0.rgb;\n"
		"    float alpha = a0.a;\n"		// Specularity
		"    float distance = a1.w;\n"  // Distance from viewer (w)
		"    vec3 normal = a1.xyz;\n"
		/// Calculate position of texel in view space
		"    vec3 position = projCoord*distance;\n"
		/// Extract position in view space from worldView matrix
		"	 vec3 lightPos = vec3(worldView[3][0],worldView[3][1],worldView[3][2]);\n"
		/// Calculate light direction and distance
		"    vec3 lightVec = lightPos - position;\n"
		"    float len_sq = dot(lightVec, lightVec);\n"
		"    float len = sqrt(len_sq);\n"
		"    vec3 lightDir = lightVec/len;\n"
		/// Calculate attenuation
		"    float attenuation = dot(lightFalloff, vec3(1, len, len_sq));\n"
		/// Calculate diffuse colour
		"    vec3 light_diffuse = max(0.0,dot(lightDir, normal)) * lightDiffuseColor;\n"
		/// Calculate specular component
		"    vec3 viewDir = -normalize(position);\n"
		"    vec3 h = normalize(viewDir + lightDir);\n"
		"    vec3 light_specular = pow(dot(normal, h),32.0) * lightSpecularColor;\n"
		/// Calcalate total lighting for this fragment
		"    vec3 total_light_contrib;\n"
		"    total_light_contrib = light_diffuse;\n";
		if(isSpecular)
		{
			shader<<
			"	 total_light_contrib += alpha * light_specular;\n";
		}
		if(isAttenuated)
		{
			shader<<
			"    gl_FragColor = vec4(total_light_contrib*colour/attenuation, 0);\n";
		}
		else
		{
			shader<<
			"    gl_FragColor = vec4(total_light_contrib*colour, 0);\n";
		}
		shader<<
		"}\n";
		
		/// Create shader object
		HighLevelGpuProgramPtr program = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"glsl", GPT_FRAGMENT_PROGRAM);
		program->setSource(shader.str());
		/// Set up default parameters
		GpuProgramParametersSharedPtr params = program->getDefaultParameters();

		params->setNamedAutoConstant("worldView", GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
		params->setNamedAutoConstant("lightDiffuseColor", GpuProgramParameters::ACT_CUSTOM, 1);
		if(isSpecular)
			params->setNamedAutoConstant("lightSpecularColor", GpuProgramParameters::ACT_CUSTOM, 2);
		if(isAttenuated)
			params->setNamedAutoConstant("lightFalloff", GpuProgramParameters::ACT_CUSTOM, 3);

		params->setNamedConstant("tex0", 0);
		params->setNamedConstant("tex1", 1);

		return HighLevelGpuProgramManager::getSingleton().getByName(program->getName());
	}
	virtual Ogre::MaterialPtr generateTemplateMaterial(Perm permutation)
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
};


LightMaterialGenerator::LightMaterialGenerator(const Ogre::String &language)
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
