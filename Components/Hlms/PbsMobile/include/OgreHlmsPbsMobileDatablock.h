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

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL ES 2.0
    */
    class _OgreHlmsPbsMobileExport HlmsPbsMobileDatablock : public HlmsDatablock
    {
        friend class HlmsPbsMobile;
    public:
        struct UvAtlasParams
        {
            float uOffset;
            float vOffset;
            float invDivisor;
            UvAtlasParams() : uOffset( 0 ), vOffset( 0 ), invDivisor( 1.0f ) {}
        };

    protected:
        uint8   mFresnelTypeSizeBytes;              //4 if mFresnel is float, 12 if it is vec3
        uint8   mNumUvAtlas;
        /// Used during shadow mapping and alpha testing
        uint8   mNumUvAtlasCaster;
    public:
        float   mRoughness;
        float   mkDr, mkDg, mkDb;                   //kD
        float   mkSr, mkSg, mkSb;                   //kS

    protected:
        float   mFresnelR, mFresnelG, mFresnelB;    //F0

    protected:
        UvAtlasParams mUvAtlasParams[4];

        TexturePtr              mTexture[NUM_PBSM_TEXTURE_TYPES];
        HlmsSamplerblock const  *mSamplerblocks[NUM_PBSM_TEXTURE_TYPES];

        /// The data in this structure only affects shader generation (thus modifying it implies
        /// generating a new shader; i.e. a call to flushRenderables()). Because this data
        /// is not needed while iterating (updating constants), it's dynamically allocated
        PbsMobileShaderCreationData *mShaderCreationData;


        void setTexture( const String &name, HlmsTextureManager::TextureMapType textureMapType,
                         TexturePtr &outTexture, UvAtlasParams *outAtlasParams );

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
            * Similar: detail_map1, detail_map2, detail_map3
                Name of the detail map to be used on top of the diffuse colour.
                There can be gaps (i.e. set detail maps 0 and 2 but not 1)

            * detail_blend_mode0 <blend_mode>
            * Similar: detail_blend_mode1, detail_blend_mode2, detail_blend_mode3
                Blend mode to use for each detail map. Valid values are:
                    "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
                    "Multiply2x", "Screen", "Overlay", "Lighten", "Darken",
                    "GrainExtract", "GrainMerge", "Difference"

            * detail_normal_map0 <texture name>
            * Similar: detail_normal_map1, detail_normal_map2, detail_normal_map3
                Name of the detail map's normal map to be used.
                It's not affected by blend mode. May be used even if
                there is no detail_map

            * reflection_map <texture name>
                Name of the reflection map. Must be a cubemap. Doesn't use an UV set because
                the tex. coords are automatically calculated.

            * uv_diffuse_map <uv>
            * Similar: uv_specular_map, uv_normal_map, uv_detail_mapN, uv_detail_normal_mapN,
            *          uv_detail_weight_map
            * where N is a number between 0 and 3.
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
        bool hasSeparateFresnel(void) const         { return mFresnelTypeSizeBytes != 4; }

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
                         const UvAtlasParams &atlasParams );

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
        @param detailMap
            Value in the range [0; 4)
        @param blendMode
            Blend mode
        */
        void setDetailMapBlendMode( uint8 detailMap, PbsMobileBlendModes blendMode );

        static UvAtlasParams textureLocationToAtlasParams(
                                            const HlmsTextureManager::TextureLocation &texLocation );

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
