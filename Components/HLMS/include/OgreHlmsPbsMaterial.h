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

#pragma once

#include "Ogre.h"  
#include "OgreHlmsPrerequisites.h"
#include "OgreHlmsMaterialBase.h"

namespace Ogre
{

#	define PBS_MAX_LIGHT_COUNT 10

	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport PbsMaterial : public HlmsMaterialBase
	{
	public:
		enum BlendFunction
		{
			BF_ALPHA = 0,
			BF_ALPHA_PREMUL,
			BF_ADD,
			BF_SUBTRACT,
			BF_MULTIPLY,
			BF_MULTIPLY_2X,
			BF_SCREEN,
			BF_OVERLAY,
			BF_LIGHTEN,
			BF_DARKEN,
			BF_GRAIN_EXTRACT,
			BF_GRAIN_MERGE,
			BF_DIFFERENCE
		};

		enum MapSlot
		{
			MS_MAIN,
			MS_D1,
			MS_D2
		};

		struct TextureAddressing
		{
			TextureAddressing() : u(TextureUnitState::TAM_WRAP), v(TextureUnitState::TAM_WRAP)
			{}

			TextureAddressing(TextureAddressingMode inU, TextureAddressingMode  inV) : u(inU), v(inV)
			{}

			TextureAddressingMode u;
			TextureAddressingMode v;
			bool operator ==(const TextureAddressing& b){ return u == b.u && v == b.v; }
		};

	private:
		enum SamplerType
		{
			ST_ENV_MAP = 0,
			ST_MAIN_ALBEDO,
			ST_MAIN_NORMALR,
			ST_MAIN_F0,
			ST_D1_ALBEDO,
			ST_D1_NORMALR,
			ST_D1_F0,
			ST_D2_ALBEDO,
			ST_D2_NORMALR,
			ST_D2_F0,
			ST_COUNT
		};

		enum SamplerStatus
		{
			SS_NOT_ACTIVE,
			SS_ACTIVE,
			SS_UPDATED,
			SS_ADDED,
			SS_REMOVED
		};

		class SamplerContainer
		{
		public:
			SamplerContainer() :textureUnitState(NULL), status(SS_NOT_ACTIVE), hasIntensity(false), intensity(0),
				hasMipmapCount(false), mipmapCount(0), hasBlendFunc(false), blendFunc(BF_ALPHA),
				hasBlendFactor1(false), blendFactor1(0), hasBlendFactor2(false), blendFactor2(0),
				needsGammaCorrection(false)
			{}

			String name;
			TextureType textureType;
			TexturePtr tex;
			TextureUnitState* textureUnitState;

			SamplerStatus status;

			bool hasIntensity;
			float intensity;

			bool hasMipmapCount;
			float mipmapCount;

			bool hasBlendFunc;
			BlendFunction blendFunc;
			bool hasBlendFactor1;
			float blendFactor1;
			bool hasBlendFactor2;
			float blendFactor2;

			TextureAddressing textureAddressing;

			bool needsGammaCorrection;

			void init(String n, bool hasBlendFu = false, bool hasBlendFc1 = false, bool hasBlendFc2 = false, bool needsGammaCorrect = false, bool hasIntens = false, bool hasMipmapC = false, TextureType texType = TEX_TYPE_2D)
			{
				name = n;
				hasBlendFunc = hasBlendFu;
				hasBlendFactor1 = hasBlendFc1;
				hasBlendFactor2 = hasBlendFc2;
				needsGammaCorrection = needsGammaCorrect;
				hasIntensity = hasIntens;
				hasMipmapCount = hasMipmapC;
				textureType = texType;
			}
		};

	public:
		PbsMaterial();

		PbsMaterial(const PbsMaterial &obj);
		virtual ~PbsMaterial();

		void setEnvironmentMap(TexturePtr tex, float intensityFactor = 1.0f);

		ColourValue getAlbedo(){ return mAlbedo; }
		void setAlbedo(ColourValue val){ mAlbedo = val; }

		ColourValue getF0(){ return mF0; }
		void setF0(ColourValue val){ mF0 = val; }

		Real getRoughness(){ return mRoughness; }
		void setRoughness(Real val){ mRoughness = val; }

		Real getLightRoughnessOffset(){ return mLightRoughnessOffset; }
		void setLightRoughnessOffset(Real val){ mLightRoughnessOffset = val; }

		void setAlbedoTexture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), BlendFunction blendFunc = BF_ALPHA, float blendFactor = 1);
		/// set texture containing normals and roughness
		void setNormalRTexture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), float normalBlendFactor = 1, float rBlendFactor = 0);
		void setF0Texture(MapSlot mapSlot, TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), BlendFunction blendFunc = BF_ALPHA, float blendFactor = 1);
		void setOffsetAndScale(MapSlot mapSlot, Vector2 offset, Vector2 scale);
		void setUvSetIndex(MapSlot mapSlot, uint index);

		void updatePropertyMap(Camera* camera, const LightList* pLightList);

		void createTextureUnits(Pass* pass);

		void updateUniforms(const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

		void updateTextureUnits(TextureUnitState* textureUnitState, GpuProgramParametersSharedPtr fragmentParams, SamplerContainer& s, int index);

	protected:

		bool mCanHardwareGamma;

		ColourValue mAlbedo;
		ColourValue mF0;
		Real mRoughness;
		Real mLightRoughnessOffset;

		Vector2 mMainOffset;
		Vector2 mMainScale;
		uint mMainUvSetIndex;

		Vector2 mD1Offset;
		Vector2 mD1Scale;
		uint mD1UvSetIndex;

		Vector2 mD2Offset;
		Vector2 mD2Scale;
		uint mD2UvSetIndex;

		uint32 mDirectionalLightCount;
		uint32 mPointLightCount;
		uint32 mSpotLightCount;

		SamplerContainer _samplers[ST_COUNT];
		bool _hasSamplerListChanged;
		bool _hasSamplerChanged;

		static const uint32 maxLightCount;

		float mLightPositions_es[PBS_MAX_LIGHT_COUNT * 3];
		float mLightDirections_es[PBS_MAX_LIGHT_COUNT * 3];
		float mLightColors[PBS_MAX_LIGHT_COUNT * 3];
		float mLightParameters[PBS_MAX_LIGHT_COUNT * 3];

		void setTexture(SamplerType samplerType, const TexturePtr& tex, const TextureAddressing& uTextureAddr,
			float blendFactor1 = 0, float blendFactor2 = 0, BlendFunction blendFunc = BF_ALPHA, float intensityFactor = 1.0);
	};
}

