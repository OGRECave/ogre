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

    enum PbsMobileTextureTypes
    {
        PBSM_DIFFUSE,
        PBSM_NORMAL,
        PBSM_SPECULAR,
        PBSM_REFLECTION,
        PBSM_MAX_TEXTURE_TYPES
    };

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL ES 2.0
    */
    class _OgreHlmsPbsMobileExport HlmsPbsMobileDatablock : public HlmsDatablock
    {
        friend class HlmsPbsMobile;

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

    public:
        struct UvAtlasParams
        {
            float uOffset;
            float vOffset;
            float invDivisor;
            UvAtlasParams() : uOffset( 0 ), vOffset( 0 ), invDivisor( 1.0f ) {}
        };

    protected:
        UvAtlasParams mUvAtlasParams[3];

        TexturePtr  mTexture[PBSM_MAX_TEXTURE_TYPES];

        void setTexture( const String &name, HlmsTextureManager::TextureMapType textureMapType,
                         TexturePtr &outTexture, UvAtlasParams &outAtlasParams );

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
                You can make it present but leave blank. A dummy texture will be assigned
                When not present, the diffuse texture sampling is left out from the
                pixel shader.

            * specular <r g b>
                Specifies the RGB specular colour. "kS" in most books about PBS
                Default: specular 1 1 1 1

            * specular_map <texture name>
                Name of the specular texture for the base image (optional)
                When not present, the specular texture is left out from the pixel shader
                Note: The alpha channel will be multiplied against the roughness value.

            * normal_map <texture name>
                Name of the normal texture for the base image (optional) for normal mapping
                When not present, the normal mapping is left out from the pixel & vertex shaders
        */
        HlmsPbsMobileDatablock( IdString name, Hlms *creator,
                             const HlmsMacroblock *macroblock,
                             const HlmsBlendblock *blendblock,
                             const HlmsParamVec &params );

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

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
