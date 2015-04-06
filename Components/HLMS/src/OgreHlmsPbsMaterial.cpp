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
	//-----------------------------------------------------------------------------------
	PbsMaterial::PbsMaterial() : mAlbedo(1, 1, 1, 0), mF0(0.1f, 0.1f, 0.1f, 1.0f), mRothness(0.1f), 
	/*header initial values*/	mMainUvSetIndex(0), mD1UvSetIndex(0), mD2UvSetIndex(0), _hasSamplerListChanged(false), 
								_hasSamplerChanged(false)
	{
		//Header initial values
		mMainOffset = Ogre::Vector2::ZERO;
		mMainScale = Ogre::Vector2::UNIT_SCALE;	
		mD1Offset = Ogre::Vector2::ZERO;
		mD1Scale = Ogre::Vector2::UNIT_SCALE;
		mD2Offset = Ogre::Vector2::ZERO;
		mD2Scale = Ogre::Vector2::UNIT_SCALE;

		_samplers[ST_ENV_MAP].init("environment", false, false, false, false, true, true, Ogre::TEX_TYPE_CUBE_MAP);

		_samplers[ST_MAIN_ALBEDO].init("main_albedo", true, true, false, true);
		_samplers[ST_MAIN_NORMAL].init("main_normal", false, true);
		_samplers[ST_MAIN_F0R].init("main_f0r", true, true, true);

		_samplers[ST_D1_ALBEDO].init("d1_albedo", true, true, false, true);
		_samplers[ST_D1_NORMAL].init("d1_normal", false, true);
		_samplers[ST_D1_F0R].init("d1_f0r", true, true, true);

		_samplers[ST_D2_ALBEDO].init("d2_albedo", true, true, false, true);
		_samplers[ST_D2_NORMAL].init("d2_normal", false, true);
		_samplers[ST_D2_F0R].init("d2_f0r", true, true, true);

		// TODO select languarge
		Ogre::String languarge = "hlsl";
		mVertexDatablock.setLanguarge(languarge);
		mFragmentDatablock.setLanguarge(languarge);

		if (!languarge.empty())
		{
			mVertexDatablock.addProfile("vs_3_0");
			mFragmentDatablock.addProfile("ps_3_0");
		}

		mVertexDatablock.setTemplateName("PBS");
		mFragmentDatablock.setTemplateName("PBS");
	}
	//-----------------------------------------------------------------------------------
	PbsMaterial::PbsMaterial(const PbsMaterial &obj)
	{
		assert(true); // this should not be copied
	}
	//-----------------------------------------------------------------------------------
	PbsMaterial::~PbsMaterial()
	{

	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setEnvironmapTexture(Ogre::TexturePtr tex, float intensityFactor)
	{
		setTexture(ST_ENV_MAP, tex, TextureAddressing(), 0, 0, BF_ADD, intensityFactor);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setAlbedoTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing, BlendFunction blendFunc, float blendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_ALBEDO + mapSlot * 3), tex, textureAddressing, blendFactor, 0, blendFunc);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setNormalTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing, float blendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_NORMAL + mapSlot * 3), tex, textureAddressing, blendFactor);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setF0RTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing, BlendFunction f0BlendFunc, float f0BlendFactor, float rBlendFactor)
	{
		setTexture((SamplerType)(ST_MAIN_F0R + mapSlot * 3), tex, textureAddressing, f0BlendFactor, rBlendFactor, f0BlendFunc);
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::setOffsetAndScale(MapSlot mapSlot, Ogre::Vector2 offset, Ogre::Vector2 scale)
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
	void PbsMaterial::setUvSetIndex(MapSlot mapSlot, Ogre::uint index)
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
	void PbsMaterial::updatePropertyMap(Ogre::Camera* camera, const Ogre::LightList* pLightList)
	{
		// Update the light properties
		mDirectionalLightCount = 0;
		mPointLightCount = 0;
		mSpotLightCount = 0;
		
		for (unsigned int i = 0; i < pLightList->size(); i++)
		{
			Ogre::Light* light = pLightList->at(i);
			switch (light->getType())
			{
			case Ogre::Light::LT_DIRECTIONAL:
				mDirectionalLightCount++;
				break;

			case Ogre::Light::LT_POINT:
				mPointLightCount++;
				break;

			case Ogre::Light::LT_SPOTLIGHT:
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
		// TODO check if the hardware supports gamma correction "Ogre::Root::getSingleton().getRenderSystem()->getCapabilities()" doesen't support this check
		bool canHardwareGamma = true; 
		mPropertyMap.setProperty("hw_gamma_read", canHardwareGamma);

		// if HardwareGammaWrite is enable we don't need to bring the result from the shader to gamma space
		bool isHardwareGammaWriteEnabled = camera->getViewport()->getTarget()->isHardwareGammaEnabled();
		mPropertyMap.setProperty("hw_gamma_write", canHardwareGamma && isHardwareGammaWriteEnabled);

		// UV Sets
		mPropertyMap.setProperty("uvset_main_index", mMainUvSetIndex);
		mPropertyMap.setProperty("uvset_d1_index", mD1UvSetIndex);
		mPropertyMap.setProperty("uvset_d2_index", mD2UvSetIndex);

		// Add or remove the texture units
		if (_hasSamplerListChanged)
		{
			int registerIndex = 0;
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer s = _samplers[i];

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
		}

		// Update the texture properties
		if (_hasSamplerChanged)
		{
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer s = _samplers[i];

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
	void PbsMaterial::updateUniforms(Ogre::Camera* camera, Ogre::Pass* pass, const Ogre::AutoParamDataSource* source, const Ogre::LightList* pLightList, bool shaderHasChanged)
	{
		// Vertex program
		GpuProgramParametersSharedPtr vertexParams = pass->getVertexProgramParameters();
		vertexParams->setIgnoreMissingParams(true);

		vertexParams->setNamedAutoConstant("mvpMat", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		vertexParams->setNamedAutoConstant("mvMat", Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

		// Fragment program
		GpuProgramParametersSharedPtr fragmentParams = pass->getFragmentProgramParameters();
		fragmentParams->setIgnoreMissingParams(true);
		
		fragmentParams->setNamedAutoConstant("ivMat", Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);
		
		fragmentParams->setNamedConstant("in_albedo", mAlbedo);
		fragmentParams->setNamedConstant("in_f0", mF0);
		fragmentParams->setNamedConstant("in_roughness", mRothness);

		fragmentParams->setNamedConstant("in_offset_main", mMainOffset);
		fragmentParams->setNamedConstant("in_scale_main", mMainScale);

		fragmentParams->setNamedConstant("in_offset_d1", mD1Offset);
		fragmentParams->setNamedConstant("in_scale_d1", mD1Scale);

		fragmentParams->setNamedConstant("in_offset_d2", mD2Offset);
		fragmentParams->setNamedConstant("in_scale_d2", mD2Scale);

		// Set light uniforms
		int count = std::min(mDirectionalLightCount + mPointLightCount + mSpotLightCount, maxLightCount);
		if (count)
		{
			Ogre::Matrix4 viewMatrix = camera->getViewMatrix();
			Ogre::Quaternion viewMatrixQuat = viewMatrix.extractQuaternion();

			for (unsigned int i = 0; i < count; i++)
			{
				Ogre::Light* light = (*pLightList)[i];

				Ogre::Vector3 pos = viewMatrix * light->getPosition();
				mLightPositions_es[i * 3 + 0] = pos.x;
				mLightPositions_es[i * 3 + 1] = pos.y;
				mLightPositions_es[i * 3 + 2] = pos.z;

				Ogre::Vector3 dir = -(viewMatrixQuat * light->getDirection()).normalisedCopy();
				mLightDirections_es[i * 3 + 0] = dir.x;
				mLightDirections_es[i * 3 + 1] = dir.y;
				mLightDirections_es[i * 3 + 2] = dir.z;

				Ogre::ColourValue color = light->getDiffuseColour();
				mLightColors[i * 3 + 0] = color.r;
				mLightColors[i * 3 + 1] = color.g;
				mLightColors[i * 3 + 2] = color.b;

				mLightParameters[i * 3 + 0] = light->getAttenuationRange();
				mLightParameters[i * 3 + 1] = Ogre::Math::Cos(light->getSpotlightOuterAngle() / 2.0);
				mLightParameters[i * 3 + 2] = light->getSpotlightFalloff();
			}

			fragmentParams->setNamedConstant("lightPositions_es", &(mLightPositions_es[0]), count, 3);
			fragmentParams->setNamedConstant("lightDirections_es", &(mLightDirections_es[0]), count, 3);
			fragmentParams->setNamedConstant("lightColors", &(mLightColors[0]), count, 3);
			fragmentParams->setNamedConstant("lightParameters", &(mLightParameters[0]), count, 3);
		}

		// update the textures
		if (shaderHasChanged)
		{
			pass->removeAllTextureUnitStates();

			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer s = _samplers[i];
				if (s.status == SS_ACTIVE || s.status == SS_ADDED || s.status == SS_UPDATED)
				{
					s.textureUnitState = pass->createTextureUnitState("map_" + s.name);
					updateTexturUnits(s.textureUnitState, fragmentParams, s);
					s.status = SS_ACTIVE;
				}
				else
				{
					s.status = SS_NOT_ACTIVE;
				}
			}
		}
		else if (_hasSamplerChanged)
		{
			for (int i = 0; i < ST_COUNT; i++)
			{
				SamplerContainer s = _samplers[i];
				if (s.status == SS_UPDATED)
				{
					updateTexturUnits(s.textureUnitState, fragmentParams, s);
					s.status = SS_ACTIVE;
				}
			}
		}

		_hasSamplerListChanged = false;
		_hasSamplerChanged = false;
	}
	//-----------------------------------------------------------------------------------
	void PbsMaterial::updateTexturUnits(Ogre::TextureUnitState* textureUnitState, Ogre::GpuProgramParametersSharedPtr fragmentParams, SamplerContainer& s)
	{
		fragmentParams->setNamedConstant("in_map_" + s.name, 0);

		if (s.textureType == Ogre::TEX_TYPE_2D)
		{
			s.textureUnitState->setTexture(s.tex);
			s.textureUnitState->setTextureAddressingMode(s.textureAddressing.u, s.textureAddressing.v, Ogre::TextureUnitState::TAM_WRAP);
		}
		else if (s.textureType == Ogre::TEX_TYPE_CUBE_MAP)
		{
			s.textureUnitState->setCubicTexture(&s.tex, true);
		}

		s.textureUnitState->setTextureFiltering(Ogre::TFO_TRILINEAR);

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
	void PbsMaterial::setTexture(SamplerType samplerType, Ogre::TexturePtr tex, TextureAddressing textureAddr,
		float blendFactor1, float blendFactor2, BlendFunction blendFunc, float intensityFactor)
	{
		SamplerContainer s = _samplers[samplerType];
		if (s.status == SS_ACTIVE && tex == s.tex && s.blendFunc == blendFunc && s.blendFactor1 == blendFactor1 && s.blendFactor2 == blendFactor2 &&
			s.intensity == intensityFactor && s.textureAddressing == textureAddr)
			return;
		if (s.status == SS_NOT_ACTIVE && tex.isNull())
			return;

		if (!tex.isNull())
		{
			// Ensure that the texture in the shader is in linear space
			tex->setHardwareGammaEnabled(s.needsGammaCorrection);

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

		s.intensity = intensityFactor;
		s.mipmapCount = tex.isNull() ? 0 : tex->getNumMipmaps();

		_hasSamplerChanged = true;
		_hasSamplerListChanged = s.status == SS_ADDED || s.status == SS_REMOVED;
	}
	//-----------------------------------------------------------------------------------
}
