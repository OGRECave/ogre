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
	/** \addtogroup Component
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

		class TextureAddressing
		{
		public:
			Ogre::TextureUnitState::TextureAddressingMode u = Ogre::TextureUnitState::TAM_WRAP;
			Ogre::TextureUnitState::TextureAddressingMode v = Ogre::TextureUnitState::TAM_WRAP;
			bool operator ==(TextureAddressing& b){ return u == b.u && v == b.v; }
		};

	private:
		enum SamplerType
		{
			ST_ENV_MAP = 0,
			ST_MAIN_ALBEDO,
			ST_MAIN_NORMAL,
			ST_MAIN_F0R,
			ST_D1_ALBEDO,
			ST_D1_NORMAL,
			ST_D1_F0R,
			ST_D2_ALBEDO,
			ST_D2_NORMAL,
			ST_D2_F0R,
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
			Ogre::String name;
			Ogre::TextureType textureType;
			Ogre::TexturePtr tex;
			Ogre::TextureUnitState* textureUnitState = NULL;

			SamplerStatus status = SS_NOT_ACTIVE;

			bool hasIntensity = false;
			float intensity = 0;

			bool hasMipmapCount = false;
			float mipmapCount = 0;

			bool hasBlendFunc = false;
			BlendFunction blendFunc = BF_ALPHA;
			bool hasBlendFactor1 = false;
			float blendFactor1 = 0;
			bool hasBlendFactor2 = false;
			float blendFactor2 = 0;

			TextureAddressing textureAddressing;

			bool needsGammaCorrection = false;

			void init(Ogre::String n, bool hasBlendFu = false, bool hasBlendFc1 = false, bool hasBlendFc2 = false, bool needsGammaCorrect = false, bool hasIntens = false, bool hasMipmapC = false, Ogre::TextureType texType = Ogre::TEX_TYPE_2D)
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

		void setEnvironmapTexture(Ogre::TexturePtr tex, float intensityFactor = 1.0f);

		Ogre::ColourValue getAlbedo(){ return mAlbedo; }
		void setAlbedo(Ogre::ColourValue val){ mAlbedo = val; }

		Ogre::ColourValue getF0(){ return mF0; }
		void setF0(Ogre::ColourValue val){ mF0 = val; }

		Ogre::Real getRothness(){ return mRothness; }
		void setRothness(Ogre::Real val){ mRothness = val; }

		void setAlbedoTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), BlendFunction blendFunc = BF_ALPHA, float blendFactor = 0);
		void setNormalTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), float blendFactor = 0);
		void setF0RTexture(MapSlot mapSlot, Ogre::TexturePtr tex, TextureAddressing textureAddressing = TextureAddressing(), BlendFunction blendFunc = BF_ALPHA, float f0BlendFactor = 0, float rBlendFactor = 0);
		void setOffsetAndScale(MapSlot mapSlot, Ogre::Vector2 offset, Ogre::Vector2 scale);
		void setUvSetIndex(MapSlot mapSlot, Ogre::uint index);

		void updatePropertyMap(Ogre::Camera* camera, const Ogre::LightList* pLightList);
		void updateUniforms(Ogre::Camera* camera, Ogre::Pass* pass, const Ogre::AutoParamDataSource* source, const Ogre::LightList* pLightList, bool shaderHasChanged);
		void updateTexturUnits(Ogre::TextureUnitState* textureUnitState, Ogre::GpuProgramParametersSharedPtr fragmentParams, SamplerContainer& s);

	protected:

		Ogre::ColourValue mAlbedo;
		Ogre::ColourValue mF0;
		Ogre::Real mRothness;

		Ogre::Vector2 mMainOffset = Ogre::Vector2::ZERO;
		Ogre::Vector2 mMainScale = Ogre::Vector2::UNIT_SCALE;
		Ogre::uint mMainUvSetIndex = 0;

		Ogre::Vector2 mD1Offset = Ogre::Vector2::ZERO;
		Ogre::Vector2 mD1Scale = Ogre::Vector2::UNIT_SCALE;
		Ogre::uint mD1UvSetIndex = 0;

		Ogre::Vector2 mD2Offset = Ogre::Vector2::ZERO;
		Ogre::Vector2 mD2Scale = Ogre::Vector2::UNIT_SCALE;
		Ogre::uint mD2UvSetIndex = 0;

		Ogre::uint32 mDirectionalLightCount;
		Ogre::uint32 mPointLightCount;
		Ogre::uint32 mSpotLightCount;

		SamplerContainer _samplers[ST_COUNT];
		bool _hasSamplerListChanged = false;
		bool _hasSamplerChanged = false;

		static const Ogre::uint32 maxLightCount = 10;

		float mLightPositions_es[maxLightCount * 3];
		float mLightDirections_es[maxLightCount * 3];
		float mLightColors[maxLightCount * 3];
		float mLightParameters[maxLightCount * 3];

		void setTexture(SamplerType samplerType, Ogre::TexturePtr tex, TextureAddressing uTextureAddr,
			float blendFactor1 = 0, float blendFactor2 = 0, BlendFunction blendFunc = BF_ALPHA, float intensityFactor = 1.0);
    };
}

