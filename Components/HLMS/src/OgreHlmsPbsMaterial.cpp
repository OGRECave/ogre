/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#include "OgreHlmsPbsMaterial.h"

namespace Ogre
{
	const uint32 PbsMaterial::maxLightCount = PBS_MAX_LIGHT_COUNT;
	//-----------------------------------------------------------------------------------
	PbsMaterial::PbsMaterial() : mAlbedo(1, 1, 1, 0), mF0(0.1f, 0.1f, 0.1f, 1.0f), mRoughness(0.1f), mLightRoughnessOffset(0.0f),
	/*header initial values*/	mMainUvSetIndex(0), mD1UvSetIndex(0), mD2UvSetIndex(0), _hasSamplerListChanged(false), 
								_hasSamplerChanged(false)
	{
		//Header initial values
		mMainOffset = Vector2::ZERO;
		mMainScale = Vector2::UNIT_SCALE;	
		mD1Offset = Vector2::ZERO;
		mD1Scale = Vector2::UNIT_SCALE;
		mD2Offset = Vector2::ZERO;
		mD2Scale = Vector2::UNIT_SCALE;

		_samplers[ST_ENV_MAP].init("environment", false, false, false, false, true, true, TEX_TYPE_CUBE_MAP);

		_samplers[ST_MAIN_ALBEDO].init("main_albedo", true, true, false, true);
		_samplers[ST_MAIN_NORMALR].init("main_normalr", false, true, true);
		_samplers[ST_MAIN_F0].init("main_f0", true, true);

		_samplers[ST_D1_ALBEDO].init("d1_albedo", true, true, false, true);
		_samplers[ST_D1_NORMALR].init("d1_normalr", false, true, true);
		_samplers[ST_D1_F0].init("d1_f0", true, true);

		_samplers[ST_D2_ALBEDO].init("d2_albedo", true, true, false, true);
		_samplers[ST_D2_NORMALR].init("d2_normalr", false, true, true);
		_samplers[ST_D2_F0].init("d2_f0", true, true);


		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();

		String language = "";
		if (caps->isShaderProfileSupported("hlsl"))
		{
			language = "hlsl";
			mVertexDatablock.addProfile("vs_3_0");
			mFragmentDatablock.addProfile("ps_3_0");
		}
		else if (caps->isShaderProfileSupported("glsl"))
		{
			language = "glsl";
		}
		else 
		{
			language = "glsles";
		}

		mVertexDatablock.setLanguage(language);
		mFragmentDatablock.setLanguage(language);

		mVertexDatablock.setTemplateName("PBS");
		mFragmentDatablock.setTemplateName("PBS");

		mCanHardwareGamma = caps->hasCapability(RSC_HW_GAMMA);
	}
	//-----------------------------------------------------------------------------------
	PbsMaterial::PbsMaterial(const PbsMaterial &obj)
	{
		OgreAssert(false, "this should not be copied");
	}
	//-----------------------------------------------------------------------------------
	PbsMaterial::~PbsMaterial()
	{

	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setEnvironmentMap(TexturePtr tex, float intensityFactor)
	{
		setTexture(ST_ENV_MAP, tex, TextureAddressing(), 0, 0, BF_ADD, intensityFactor);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setAlbedoTexture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing, BlendFunction blendFunc, float blendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_ALBEDO + mapSlot * 3), tex, textureAddressing, blendFactor, 0, blendFunc);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setNormalRTexture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing, float normalBlendFactor, float rBlendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_NORMALR + mapSlot * 3), tex, textureAddressing, normalBlendFactor, rBlendFactor);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setF0Texture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing, BlendFunction blendFunc, float blendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_F0 + mapSlot * 3), tex, textureAddressing, blendFactor, 0, blendFunc);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setOffsetAndScale(MapSlot mapSlot, Vector2 offset, Vector2 scale)
	{
		switch (mapSlot)
		{
		case PbsMaterial::MS_MAIN:
			mMainOffset = offset;
			mMainScale = scale;
			break;
		case PbsMaterial::MS_D1:
			mD1Offset = offset;
			mD1Scale = scale;
			break;
		case PbsMaterial::MS_D2:
			mD2Offset = offset;
			mD2Scale = scale;
			break;
		default:
			break;
		}
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setUvSetIndex(MapSlot mapSlot, uint index)
	{
		switch (mapSlot)
		{
		case PbsMaterial::MS_MAIN:
			mMainUvSetIndex = index;
			break;
		case PbsMaterial::MS_D1:
			mD1UvSetIndex = index;
			break;
		case PbsMaterial::MS_D2:
			mD2UvSetIndex = index;
			break;
		default:
			break;
		}
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::updatePropertyMap(Camera* camera, const LightList* pLightList)
	{
		// Update the light properties
		mDirectionalLightCount = 0;
		mPointLightCount = 0;
		mSpotLightCount = 0;
		
		for (unsigned int i = 0; i < pLightList->size(); i++)
		{
			Light* light = pLightList->at(i);
			switch (light->getType())
			{
			case Light::LT_DIRECTIONAL:
				mDirectionalLightCount++;
				break;

			case Light::LT_POINT:
				mPointLightCount++;
				break;

			case Light::LT_SPOTLIGHT:
				mSpotLightCount++;
				break;

			default:
				break;
			}
		}

		mPropertyMap.setProperty("lights_directional_start", 0);
		mPropertyMap.setProperty("lights_directional_count", mDirectionalLightCount);

		mPropertyMap.setProperty("lights_point_start", mDirectionalLightCount);
		mPropertyMap.setProperty("lights_point_count", mPointLightCount);

		mPropertyMap.setProperty("lights_spot_start", mDirectionalLightCount + mPointLightCount);
		mPropertyMap.setProperty("lights_spot_count", mSpotLightCount);

		mPropertyMap.setProperty("lights_count", mDirectionalLightCount + mPointLightCount + mSpotLightCount);

		// tell the shader if the hardware supports hardware gamma correction or not 
		mPropertyMap.setProperty("hw_gamma_read", mCanHardwareGamma);

		// if HardwareGammaWrite is enable we don't need to bring the result from the shader to gamma space
		bool isHardwareGammaWriteEnabled = camera->getViewport()->getTarget()->isHardwareGammaEnabled();
		mPropertyMap.setProperty("hw_gamma_write", isHardwareGammaWriteEnabled);

		// UV Sets
		mPropertyMap.setProperty("uvset_main_index", mMainUvSetIndex);
		mPropertyMap.setProperty("uvset_d1_index", mD1UvSetIndex);
		mPropertyMap.setProperty("uvset_d2_index", mD2UvSetIndex);

		// check for duplicate attributes (e.g. do not define attribut vec4 uv0 more than once)
		mPropertyMap.setProperty("uvset_d1_setattribute", mD1UvSetIndex != mMainUvSetIndex);
		mPropertyMap.setProperty("uvset_d2_setattribute", mD2UvSetIndex != mMainUvSetIndex && mD2UvSetIndex != mD1UvSetIndex);

		// Add or remove the texture units
		if (_hasSamplerListChanged)
		{
			int registerIndex = 0;
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer& s = _samplers[i];

				if (s.status == SS_ACTIVE || s.status == SS_ADDED || s.status == SS_UPDATED)
				{
					mPropertyMap.setProperty("map_" + s.name, 1);
					mPropertyMap.setProperty("map_" + s.name + "_register", registerIndex++);
				}
				else if (s.status == SS_REMOVED)
				{
					mPropertyMap.removeProperty("map_" + s.name);
					mPropertyMap.removeProperty("map_" + s.name + "_register");
				}
			}

			_hasSamplerListChanged = false;
		}

		// Update the texture properties
		if (_hasSamplerChanged)
		{
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer& s = _samplers[i];

				if (s.hasBlendFunc)
				{
					if (s.status == SS_ACTIVE || s.status == SS_ADDED || s.status == SS_UPDATED)
					{
						mPropertyMap.setProperty("blendFunc_" + s.name, s.blendFunc);
					}
					else if (s.status == SS_REMOVED)
					{
						mPropertyMap.removeProperty("blendFunc_" + s.name);
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::createTextureUnits(Pass* pass)
	{
		GpuProgramParametersSharedPtr fragmentParams = pass->getFragmentProgramParameters();
		fragmentParams->setIgnoreMissingParams(true);

		// Create the texture unit states
		for (int i = 0; i < ST_COUNT; i++)
		{
			SamplerContainer& s = _samplers[i];
			if (s.status == SS_ACTIVE || s.status == SS_ADDED || s.status == SS_UPDATED)
			{
				s.textureUnitState = pass->createTextureUnitState();
				s.textureUnitState->setName("in_map_" + s.name);
				s.status = SS_UPDATED;
			}
			else
			{
				s.status = SS_NOT_ACTIVE;
			}
		}

		// set the sampler name for the texture unit state
		int size = pass->getNumTextureUnitStates();
		for (int i = 0; i < size; i++)
		{
			TextureUnitState* tus = pass->getTextureUnitState(i);
			fragmentParams->setNamedConstant(tus->getName(), i);
		}

		_hasSamplerChanged = true;
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::updateUniforms(const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList)
	{
		// Vertex program
		GpuProgramParametersSharedPtr vertexParams = pass->getVertexProgramParameters();
		vertexParams->setIgnoreMissingParams(true);

		vertexParams->setNamedAutoConstant("mvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		vertexParams->setNamedAutoConstant("mvMat", GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

		// Fragment program
		GpuProgramParametersSharedPtr fragmentParams = pass->getFragmentProgramParameters();
		
		fragmentParams->setNamedAutoConstant("ivMat", GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);
		
		fragmentParams->setNamedConstant("in_albedo", mAlbedo);
		fragmentParams->setNamedConstant("in_f0", mF0);
		fragmentParams->setNamedConstant("in_roughness", mRoughness);
		fragmentParams->setNamedConstant("in_light_roughness_offset", mLightRoughnessOffset);

		fragmentParams->setNamedConstant("in_offset_main", mMainOffset);
		fragmentParams->setNamedConstant("in_scale_main", mMainScale);

		fragmentParams->setNamedConstant("in_offset_d1", mD1Offset);
		fragmentParams->setNamedConstant("in_scale_d1", mD1Scale);

		fragmentParams->setNamedConstant("in_offset_d2", mD2Offset);
		fragmentParams->setNamedConstant("in_scale_d2", mD2Scale);

		// Set light uniforms
		unsigned int count = std::min(mDirectionalLightCount + mPointLightCount + mSpotLightCount, maxLightCount);
		if (count)
		{
		    Affine3 viewMatrix = source->getViewMatrix();
			Quaternion viewMatrixQuat = Quaternion(viewMatrix.linear());

			int directionalLightIndex = 0;
			int pointLightLightIndex = 0;
			int spotLightIndex = 0;

			for (unsigned int i = 0; i < count; i++)
			{
				Light* light = (*pLightList)[i];

				int index;
				if (light->getType() == Light::LT_DIRECTIONAL)
				{
					index = directionalLightIndex;
					directionalLightIndex++;
				}
				else if (light->getType() == Light::LT_POINT)
				{
					index = mDirectionalLightCount + pointLightLightIndex;
					pointLightLightIndex++;
				}
				else
				{
					index = mDirectionalLightCount + mPointLightCount + spotLightIndex;
					spotLightIndex++;
				}

				Vector3 pos = viewMatrix * light->getDerivedPosition();
				mLightPositions_es[index * 4 + 0] = pos.x;
				mLightPositions_es[index * 4 + 1] = pos.y;
				mLightPositions_es[index * 4 + 2] = pos.z;

				Vector3 dir = -(viewMatrixQuat * light->getDerivedDirection()).normalisedCopy();
				mLightDirections_es[index * 4 + 0] = dir.x;
				mLightDirections_es[index * 4 + 1] = dir.y;
				mLightDirections_es[index * 4 + 2] = dir.z;

				ColourValue color = light->getDiffuseColour();
				mLightColors[index * 4 + 0] = color.r;
				mLightColors[index* 4 + 1] = color.g;
				mLightColors[index* 4 + 2] = color.b;

				mLightParameters[index * 4 + 0] = light->getAttenuationRange();
				mLightParameters[index * 4 + 1] = Math::Cos(light->getSpotlightOuterAngle() / 2.0);
				mLightParameters[index * 4 + 2] = light->getSpotlightFalloff();
			}

			fragmentParams->setNamedConstant("lightPositions_es", &(mLightPositions_es[0]), count);
			fragmentParams->setNamedConstant("lightDirections_es", &(mLightDirections_es[0]), count);
			fragmentParams->setNamedConstant("lightColors", &(mLightColors[0]), count);
			fragmentParams->setNamedConstant("lightParameters", &(mLightParameters[0]), count);
		}

		// update the textures
		if (_hasSamplerChanged)
		{
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer& s = _samplers[i];
				if (s.status == SS_UPDATED)
				{
					updateTextureUnits(s.textureUnitState, fragmentParams, s, i);
					s.status = SS_ACTIVE;
				}
			}

			_hasSamplerChanged = false;
		}
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::updateTextureUnits(TextureUnitState* textureUnitState, GpuProgramParametersSharedPtr fragmentParams, SamplerContainer& s, int index)
	{
		if (s.textureType == TEX_TYPE_2D)
		{
			s.textureUnitState->setTexture(s.tex);
			s.textureUnitState->setTextureAddressingMode(s.textureAddressing.u, s.textureAddressing.v, TextureUnitState::TAM_WRAP);
		}
		else if (s.textureType == TEX_TYPE_CUBE_MAP)
		{
			s.textureUnitState->setTexture(s.tex);
			if (mFragmentDatablock.getLanguage() != "hlsl")
			{
				s.textureUnitState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
			}
		}
		s.textureUnitState->setTextureFiltering(TFO_TRILINEAR);
	
		if (s.hasIntensity)
		{
			fragmentParams->setNamedConstant("in_map_" + s.name + "_intensity", s.intensity);
		}

		if (s.hasMipmapCount)
		{
			fragmentParams->setNamedConstant("in_map_" + s.name + "_mipmapcount", s.mipmapCount);
		}

		if (s.hasBlendFactor1)
		{
			fragmentParams->setNamedConstant("in_blendfactor1_" + s.name, s.blendFactor1);
		}

		if (s.hasBlendFactor2)
		{
			fragmentParams->setNamedConstant("in_blendfactor2_" + s.name, s.blendFactor2);
		}
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setTexture(SamplerType samplerType, const TexturePtr& tex, const TextureAddressing& textureAddr,
		float blendFactor1, float blendFactor2, BlendFunction blendFunc, float intensityFactor)
	{
		SamplerContainer& s = _samplers[samplerType];
		if (s.status == SS_ACTIVE && tex == s.tex && s.blendFunc == blendFunc && s.blendFactor1 == blendFactor1 && s.blendFactor2 == blendFactor2 &&
			s.intensity == intensityFactor && s.textureAddressing == textureAddr)
			return;
		if (s.status == SS_NOT_ACTIVE && !tex)
			return;

		if (tex)
		{
			// Ensure that the texture in the shader is in linear space
			tex->setHardwareGammaEnabled(mCanHardwareGamma && s.needsGammaCorrection);

			if (s.status == SS_NOT_ACTIVE) s.status = SS_ADDED;
			else if (s.status == SS_ACTIVE) s.status = SS_UPDATED;
			else if (s.status == SS_UPDATED) s.status = SS_UPDATED;
			else if (s.status == SS_ADDED) s.status = SS_ADDED;
			else if (s.status == SS_REMOVED) s.status = SS_UPDATED;
		}
		else
		{
			if (s.status == SS_NOT_ACTIVE) s.status = SS_NOT_ACTIVE;
			else if (s.status == SS_ACTIVE) s.status = SS_REMOVED;
			else if (s.status == SS_UPDATED) s.status = SS_REMOVED;
			else if (s.status == SS_ADDED) s.status = SS_NOT_ACTIVE;
			else if (s.status == SS_REMOVED) s.status = SS_REMOVED;
		}

		s.tex = tex;
		s.textureAddressing = textureAddr;

		s.blendFunc = blendFunc;
		s.blendFactor1 = blendFactor1;
		s.blendFactor2 = blendFactor2;
		s.hasBlendFactor1 = s.hasBlendFactor2 = true;

		s.intensity = intensityFactor;
		s.mipmapCount = !tex ? 0.0f : tex->getNumMipmaps();

		_hasSamplerChanged = true;
		_hasSamplerListChanged = s.status == SS_ADDED || s.status == SS_REMOVED;
	}
	//-----------------------------------------------------------------------------------
}
