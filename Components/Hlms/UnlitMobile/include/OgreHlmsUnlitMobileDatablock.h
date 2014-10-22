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
#ifndef _OgreHlmsUnlitMobileDatablock_H_
#define _OgreHlmsUnlitMobileDatablock_H_

#include "OgreHlmsUnlitMobilePrerequisites.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsTextureManager.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    /// Contains information needed by the UI (2D) for OpenGL ES 2.0
    class _OgreHlmsUnlitMobileExport HlmsUnlitMobileDatablock : public HlmsDatablock
    {
        friend class HlmsUnlitMobile;

        struct ShaderCreationData
        {
            /// Maps UV coordinate sets to mTextureMatrices starting index.
            uint8           mTextureMatrixMap[8];

            TexturePtr      mDiffuseTextures[16];
            HlmsSamplerblock const  *mSamplerblocks[16];

            /// One per texture unit. Specifies which UV set we will use.
            uint8           mUvSetForTexture[16];
            uint8           mBlendModes[16];
            bool            mTextureIsAtlas[16];

            ShaderCreationData()
            {
                memset( mTextureMatrixMap, 0xffffffff, sizeof(mTextureMatrixMap) );
                memset( mSamplerblocks, 0, sizeof(mSamplerblocks) );
                memset( mUvSetForTexture, 0, sizeof(mUvSetForTexture) );
                memset( mBlendModes, 0, sizeof(mBlendModes) );
                memset( mTextureIsAtlas, 0, sizeof(mTextureIsAtlas) );
            }
        };

    public:
        struct UvAtlasParams
        {
            float uOffset;
            float vOffset;
            float invDivisor;
            UvAtlasParams() : uOffset( 0 ), vOffset( 0 ), invDivisor( 1.0f ) {}
        };

    protected:
        /// Up to 8 matrices; RS APIs don't let us to pass through
        /// more than 8 UVs to the pixel shader anyway
        uint32  mNumTextureMatrices;
        float   mTextureMatrices[16*8];

        bool    mHasColour;         /// When false; mR, mG, mB & mA aren't passed to the pixel shader
        uint8   mNumTextureUnits;
        uint8   mNumUvAtlas;
        float   mR, mG, mB, mA;

        UvAtlasParams mUvAtlasParams[16];

        /// Up to 16 diffuse textures (they can re use UVs); which is the limit for a lot of HW
        /// Must be contiguous (i.e. if mDiffuseTextures[1] isn't used, mDiffuseTextures[2] can't be)
        TexturePtr mBakedDiffuseTextures[16];
        HlmsSamplerblock const  *mBakedSamplerblocks[16];

        /// The data in this structure only affects shader generation (thus modifying it implies
        /// generating a new shader; i.e. a call to flushRenderables()). Because this data
        /// is not needed while iterating (updating constants), it's dynamically allocated
        ShaderCreationData *mShaderCreationData;

    public:
        /** Valid parameters in params:
        @param params
            * diffuse [r g b [a]]
                If absent, the values of mR, mG, mB & mA will be ignored by the pixel shader.
                When present, the rgba values can be specified.
                Default: Absent
                Default (when present): diffuse 1 1 1 1

            * diffuse_map [texture name] [#uv]
                Name of the diffuse texture for the base image (optional, otherwise a dummy is set)
                The #uv parameter is optional, and specifies the texcoord set that will
                be used. Valid range is [0; 8)
                If the Renderable doesn't have enough UV texcoords, HLMS will throw an exception.

                Note: The UV set is evaluated when creating the Renderable cache.

            * diffuse_map1 [texture name] [blendmode] [#uv]
                Name of the diffuse texture that will be layered on top of the base image.
                The #uv parameter is optional. Valid range is [0; 8)
                The blendmode parameter is optional. Valid values are:
                    NormalNonPremul, NormalPremul, Add, Subtract, Multiply,
                    Multiply2x, Screen, Overlay, Lighten, Darken, GrainExtract,
                    GrainMerge, Difference
                which are very similar to Photoshop/GIMP's blend modes.
                See Samples/Media/Hlms/GuiMobile/GLSL/BlendModes_piece_ps.glsl for the exact math.
                Default blendmode:      NormalPremul
                Default uv:             0
                Example: diffuse_map1 myTexture.png Add 3

             * diffuse_map2 through diffuse_map16
                Same as diffuse_map1 but for subsequent layers to be applied on top of the previous
                images. You can't leave gaps (i.e. specify diffuse_map0 & diffuse_map2 but not
                diffuse_map1).
                Note that not all mobile HW supports 16 textures at the same time, thus we will
                just cut/ignore the extra textures that won't fit (we log a warning though).

             * animate <#uv> [<#uv> <#uv> ... <#uv>]
                Enables texture animation through a 4x4 matrix for the specified UV sets.
                Default: All UV set animation/manipulation disabled.
                Example: animate 0 1 2 3 4 5 6 7

             * alpha_test [compare_func] [threshold]
                When present, mAlphaTestThreshold is used.
                compare_func is optional. Valid values are:
                    less, less_equal, equal, greater, greater_equal, not_equal
                Threshold is optional, and a value in the range (0; 1)
                Default: alpha_test less 0.5
                Example: alpha_test equal 0.1
        */
        HlmsUnlitMobileDatablock( IdString name, Hlms *creator,
                                  const HlmsMacroblock *macroblock,
                                  const HlmsBlendblock *blendblock,
                                  const HlmsParamVec &params );
        virtual ~HlmsUnlitMobileDatablock();

        virtual void calculateHash();

        /// Controls whether the value in @see setColour is used.
        /// Calling this function implies calling @see HlmsDatablock::flushRenderables.
        void setUseColour( bool useColour );

        /// If this returns false, the values of mR, mG, mB & mA will be ignored.
        /// @see setUseColour.
        bool hasColour(void) const                      { return mHasColour; }

        /// Sets a new colour value. Asserts if mHasColour is false.
        void setColour( const ColourValue &diffuse );

        /// Gets the current colour. The returned value is meaningless if mHasColour is false
        ColourValue getColour(void) const               { return ColourValue( mR, mG, mB, mA ); }

        /** Sets a new texture for rendering
        @param texUnit
            ID of the texture unit. Must be in range [0; 16) otherwise throws.
        @param newTexture
            Texture to change to. Can't be null, otherwise throws (use a blank texture).
        @param atlasParams
            The atlas offsets in case this texture is an atlas or an array texture
        */
        void setTexture( uint8 texUnit, TexturePtr &newTexture, const UvAtlasParams &atlasParams );

        TexturePtr getTexture( uint8 texUnit ) const;

        /** Sets a new sampler block to be associated with the texture
            (i.e. filtering mode, addressing modes, etc).
        @param texUnit
            ID of the texture unit. Must be in range [0; 16) otherwise throws.
        @param params
            The sampler block to use as reference.
        */
        void setSamplerblock( uint8 texUnit, const HlmsSamplerblock &params );

        const HlmsSamplerblock* getSamplerblock( uint8 texUnit ) const;

        /** Sets the set of UVs that will be used to sample from the texture unit.
        @remarks
            Calling this function implies calling @see HlmsDatablock::flushRenderables. If
            the another shader must be created, it could cause a stall.
        @param texUnit
            ID of the texture unit. Must be in range [0; 16) otherwise throws.
        @param uvSet
            The uv set. Must be in range [0; 8) otherwise throws. If the datablock is assigned
            to a mesh that has less UV sets than required, it will throw during the assignment.
        */
        void setTextureUvSource( uint8 texUnit, uint8 uvSet );

        /// Returns the number of texture units that are actually used.
        uint8 getNumTextureUnitsInUse(void) const           { return mNumTextureUnits; }

        /// Calculates the amount of UV sets used by the datablock
        uint8 getNumUvSets(void) const;

        static UvAtlasParams textureLocationToAtlasParams(
                                            const HlmsTextureManager::TextureLocation &texLocation );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
