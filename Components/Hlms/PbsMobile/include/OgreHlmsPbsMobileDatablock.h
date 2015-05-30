/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef _OgreHlmsPbsMobileDatablock_H_
#define _OgreHlmsPbsMobileDatablock_H_

#include "OgreHlmsPbsMobilePrerequisites.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    struct PbsMobileShaderCreationData;
    struct PbsUvAtlasParams;

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL ES 2.0
    */
    class _OgreHlmsPbsMobileExport HlmsPbsMobileDatablock : public HlmsDatablock
    {
        friend class HlmsPbsMobile;
    protected:
        /// [0] = Regular one.
        /// [1] = Used during shadow mapping
        uint16  mFullParametersBytes[2];

    public:
        float   mRoughness;
        float   mkDr, mkDg, mkDb;                   //kD
        float   mkSr, mkSg, mkSb;                   //kS

    protected:
        /// Baked parameters from PbsMobileShaderCreationData.
        /// mNumVariableParameters says how many parameters were
        /// actually baked.
        float   mVariableParameters[60];

        TexturePtr              mTexture[NUM_PBSM_TEXTURE_TYPES];
        HlmsSamplerblock const  *mSamplerblocks[NUM_PBSM_TEXTURE_TYPES];

        /// The data in this structure only affects shader generation (thus modifying it
        /// may imply generating a new shader; i.e. a call to flushRenderables()). Because
        /// this data is not needed while iterating (updating constants), it's dynamically
        /// allocated. It also contains information that rarely changes and is then baked.
        PbsMobileShaderCreationData *mShaderCreationData;


        void bakeVariableParameters(void);
        void setTexture( const String &name, HlmsTextureManager::TextureMapType textureMapType,
                         PbsMobileTextureTypes textureType );

    public:
        /** Valid parameters in params:
        @param params
            * fresnel <value [g, b]>
                The IOR. @See setIndexOfRefraction
                When specifying three values, the fresnel is separate for each
                colour component

            * fresnel_coeff <value [g, b]>
                Directly sets the fresnel values, instead of using IORs
                "F0" in most books about PBS

            * roughness <value>
                Specifies the roughness value. Should be in range (0; inf)
                Note: Values extremely close to zero could cause NaNs and
                INFs in the pixel shader, also depends on the GPU's precision.

            * diffuse <r g b>
                Specifies the RGB diffuse colour. "kD" in most books about PBS
                Default: diffuse 1 1 1 1
                Note: Internally the diffuse colour is divided by PI.

            * diffuse_map <texture name>
                Name of the diffuse texture for the base image (optional)

            * specular <r g b>
                Specifies the RGB specular colour. "kS" in most books about PBS
                Default: specular 1 1 1 1

            * specular_map <texture name>
                Name of the specular texture for the base image (optional).

            * roughness_map <texture name>
                Name of the roughness texture for the base image (optional)
                Note: Only the Red channel will be used, and the texture will be converted to
                an efficient monochrome representation.

            * normal_map <texture name>
                Name of the normal texture for the base image (optional) for normal mapping

            * detail_weight_map <texture name>
                Texture that when present, will be used as mask/weight for the 4 detail maps.
                The R channel is used for detail map #0; the G for detail map #1, B for #2,
                and Alpha for #3.
                This affects both the diffuse and normal-mapped detail maps.

            * detail_map0 <texture name>
              Similar: detail_map1, detail_map2, detail_map3
                Name of the detail map to be used on top of the diffuse colour.
                There can be gaps (i.e. set detail maps 0 and 2 but not 1)

            * detail_blend_mode0 <blend_mode>
              Similar: detail_blend_mode1, detail_blend_mode2, detail_blend_mode3
                Blend mode to use for each detail map. Valid values are:
                    "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
                    "Multiply2x", "Screen", "Overlay", "Lighten", "Darken",
                    "GrainExtract", "GrainMerge", "Difference"

            * detail_offset_scale0 <offset_u> <offset_v> <scale_u> <scale_v>
              Similar: detail_offset_scale1, detail_offset_scale2, detail_offset_scale3
                Sets the UV offset and scale of the detail maps.

            * detail_normal_map0 <texture name>
              Similar: detail_normal_map1, detail_normal_map2, detail_normal_map3
                Name of the detail map's normal map to be used.
                It's not affected by blend mode. May be used even if
                there is no detail_map

            * detail_normal_offset_scale0 <offset_u> <offset_v> <scale_u> <scale_v>
              Similar: detail_normal_offset_scale1, detail_normal_offset_scale2,
                       detail_normal_offset_scale3
                Sets the UV offset and scale of the detail normal maps.

            * reflection_map <texture name>
                Name of the reflection map. Must be a cubemap. Doesn't use an UV set because
                the tex. coords are automatically calculated.

            * uv_diffuse_map <uv>
              Similar: uv_specular_map, uv_normal_map, uv_detail_mapN, uv_detail_normal_mapN,
                       uv_detail_weight_map
              where N is a number between 0 and 3.
                UV set to use for the particular texture map.
                The UV value must be in range [0; 8)
        */
        HlmsPbsMobileDatablock( IdString name, Hlms *creator,
                                const HlmsMacroblock *macroblock,
                                const HlmsBlendblock *blendblock,
                                const HlmsParamVec &params );
        virtual ~HlmsPbsMobileDatablock();

        /// Sets the diffuse colour. The colour will be divided by PI for energy conservation.
        void setDiffuse( const Vector3 &diffuseColour );
        Vector3 getDiffuse(void) const;

        /// Sets the specular colour.
        void setSpecular( const Vector3 &specularColour );
        Vector3 getSpecular(void) const;

        /// Sets the roughness
        void setRoughness( float roughness );
        float getRoughness(void) const;

        /** Calculates fresnel (F0 in most books) based on the IOR.
            The formula used is ( (1 - idx) / 1 + idx )²
        @remarks
            If "separateFresnel" was different from the current setting, it will call
            @see HlmsDatablock::flushRenderables. If the another shader must be created,
            it could cause a stall.
        @param refractionIdx
            The index of refraction of the material for each colour component.
            When separateFresnel = false, the Y and Z components are ignored.
        @param separateFresnel
            Whether to use the same fresnel term for RGB channel, or individual ones.
        */
        void setIndexOfRefraction( const Vector3 &refractionIdx, bool separateFresnel );

        /** Sets the fresnel (F0) directly, instead of using the IOR. @See setIndexOfRefraction
        @remarks
            If "separateFresnel" was different from the current setting, it will call
            @see HlmsDatablock::flushRenderables. If the another shader must be created,
            it could cause a stall.
        @param refractionIdx
            The fresnel of the material for each colour component.
            When separateFresnel = false, the Y and Z components are ignored.
        @param separateFresnel
            Whether to use the same fresnel term for RGB channel, or individual ones.
        */
        void setFresnel( const Vector3 &fresnel, bool separateFresnel );

        /// Returns the current fresnel. Note: when hasSeparateFresnel returns false,
        /// the Y and Z components still correspond to mFresnelG & mFresnelB just
        /// in case you want to preserve this data (i.e. toggling separate fresnel
        /// often (which is not a good idea though, in terms of performance)
        Vector3 getFresnel(void) const;

        /// Whether the same fresnel term is used for RGB, or individual ones for each channel
        bool hasSeparateFresnel(void) const;

        /** Sets a new texture for rendering
        @param UvAtlasParams
            Type of the texture.
        @param newTexture
            Texture to change to. If it is null and previously wasn't (or viceversa), will
            trigger HlmsDatablock::flushRenderables.
        @param atlasParams
            The atlas offsets in case this texture is an atlas or an array texture
            Doesn't apply to certain texture types (i.e. PBSM_REFLECTION)
        */
        void setTexture( PbsMobileTextureTypes texType, TexturePtr &newTexture,
                         const PbsUvAtlasParams &atlasParams );

        /** Sets a new sampler block to be associated with the texture
            (i.e. filtering mode, addressing modes, etc).
        @param texType
            Type of texture.
        @param params
            The sampler block to use as reference.
        */
        void setSamplerblock( PbsMobileTextureTypes texType, const HlmsSamplerblock &params );

        const HlmsSamplerblock* getSamplerblock( PbsMobileTextureTypes texType ) const;

        /** Sets which UV set to use for the given texture.
            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param sourceType
            Source texture to modify. Note that we don't enforce
            PBSM_SOURCE_DETAIL0 = PBSM_SOURCE_DETAIL_NM0, but you probably
            want to have both textures using the same UV source.
            Must be lower than NUM_PBSM_SOURCES.
        @param uvSet
            UV coordinate set. Value must be between in range [0; 8)
        */
        void setTextureUvSource( PbsMobileTextureTypes sourceType, uint8 uvSet );

        /** Changes the blend mode of the detail map. Calling this function triggers a
            HlmsDatablock::flushRenderables even if you never use detail maps (they
            affect the cache's hash)
        @remarks
            This parameter only affects the diffuse detail map. Not the normal map.
        @param detailMapIdx
            Value in the range [0; 4)
        @param blendMode
            Blend mode
        */
        void setDetailMapBlendMode( uint8 detailMapIdx, PbsMobileBlendModes blendMode );

        /** Sets the normal mapping weight. The range doesn't necessarily have to be in [0; 1]
        @remarks
            An exact value of 1 will generate a shader without the weighting math, while any
            other value will generate another shader that uses this weight (i.e. will
            cause a flushRenderables)
        @param detailNormalMapIdx
            Value in the range [0; 4)
        @param weight
            The weight for the normal map.
            A value of 0 means no effect (tangent space normal is 0, 0, 1); and would be
            the same as disabling the normal map texture.
            A value of 1 means full normal map effect.
            A value outside the [0; 1] range extrapolates.
            Default value is 1.
        */
        void setDetailNormalWeight( uint8 detailNormalMapIdx, Real weight );

        /// Returns the detail normal maps' weight
        Real getDetailNormalWeight( uint8 detailNormalMapIdx ) const;

        /// @See setDetailNormalWeight. This is the same, but for the main normal map.
        void setNormalMapWeight( Real weight );

        /// Returns the detail normal maps' weight
        Real getNormalMapWeight(void) const;

        /** Sets the weight of detail map. Affects both diffuse and
            normal at the same time.
        @remarks
            A value of 1 will cause a flushRenderables as we remove the code from the
            shader.
            The weight from @see setNormalMapWeight is multiplied against this value
            when it comes to the detail normal map.
        @param detailMap
            Value in the range [0; 4)
        @param weight
            The weight for the detail map. Usual values are in range [0; 1] but any
            value is accepted and valid.
            Default value is 1
        */
        void setDetailMapWeight( uint8 detailMap, Real weight );
        Real getDetailMapWeight( uint8 detailMap ) const;

        /** Sets the scale and offset of the detail map.
        @remarks
            A value of Vector4( 0, 0, 1, 1 ) will cause a flushRenderables as we
            remove the code from the shader.
        @param detailMap
            Value in the range [0; 8)
            Range [0; 4) affects diffuse maps.
            Range [4; 8) affects normal maps.
        @param offsetScale
            XY = Contains the UV offset.
            ZW = Constains the UV scale.
            Default value is Vector4( 0, 0, 1, 1 )
        */
        void setDetailMapOffsetScale( uint8 detailMap, const Vector4 &offsetScale );
        const Vector4& getDetailMapOffsetScale( uint8 detailMap ) const;

        /** @see HlmsDatablock::setAlphaTest
        @remarks
            Alpha testing works on the alpha channel of the diffuse texture.
            If there is no diffuse texture, the first diffuse detail map after
            applying the blend weights (texture + params) is used.
            If there are no diffuse nor detail-diffuse maps, the alpha test is
            compared against the value 1.0
        */
        virtual void setAlphaTest( CompareFunction compareFunction );
        virtual void setAlphaTestThreshold( float threshold );

        static PbsUvAtlasParams textureLocationToAtlasParams(
                                            const HlmsTextureManager::TextureLocation &texLocation );

        /** Suggests the TextureMapType (aka texture category) for each type of texture
            (i.e. normals should load from TEXTURE_TYPE_NORMALS).
        @remarks
            Remember that if "myTexture" was loaded as TEXTURE_TYPE_DIFFUSE and then you try
            to load it as TEXTURE_TYPE_NORMALS, the first one will prevail until it's removed.
            You could create an alias however, and thus have two copies of the same texture with
            different loading parameters.
        */
        static HlmsTextureManager::TextureMapType suggestMapTypeBasedOnTextureType(
                                                        PbsMobileTextureTypes type );

        int _calculateNumUvAtlas( bool casterPass ) const;

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
