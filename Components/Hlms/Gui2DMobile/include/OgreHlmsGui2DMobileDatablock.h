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
#ifndef _OgreHlmsGui2DMobileDatablock_H_
#define _OgreHlmsGui2DMobileDatablock_H_

#include "OgreHlmsDatablock.h"
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
    class _OgreExport HlmsGui2DMobileDatablock : public HlmsDatablock
    {
        friend class HlmsGui2DMobile;
    protected:

        struct UvAtlasParams
        {
            float uOffset;
            float vOffset;
            float invDivisor;
        };

        /// Up to 8 matrices; RS APIs don't let us to pass through
        /// more than 8 UVs to the pixel shader anyway
        uint32  mNumTextureMatrices;
        float   mTextureMatrices[16*8];

        bool    mHasColour;         /// When false; mR, mG, mB & mA aren't passed to the pixel shader
        bool    mIsAlphaTested;
        uint8   mNumTextureUnits;
        float   mR, mG, mB, mA;
        float   mAlphaTestThreshold;

        UvAtlasParams mUvAtlasParams[16];

        /// Up to 16 diffuse textures (they can re use UVs); which is the limit for a lot of HW
        /// Must be contiguous (i.e. if mDiffuseTextures[1] isn't used, mDiffuseTextures[2] can't be)
        TexturePtr mDiffuseTextures[16];

        /// Maps UV coordinate sets to mTextureMatrices starting index.
        uint8   mTextureMatrixMap[8];

    public:
        /** Valid parameters in params:
        @param params
            * diffuse [r g b [a]]
                If absent, the values of mR, mG, mB & mA will be ignored by the pixel shader.
                When present, the rgba values can be specified.
                Default: Absent
                Default (when present): diffuse 1 1 1 1

            * diffuse_map <texture name> [#uv]
                Name of the diffuse texture for the base image.
                The #uv parameter is optional, and specifies the texcoord set that will
                be used. Valid range is [0; 8)
                If the Renderable doesn't have enough UV texcoords, HLMS will throw an exception.

                Note: The UV set is evaluated when creating the Renderable cache.

            * diffuse_map0
                Alias for diffuse_map

            * diffuse_map1 <texture name> [blendmode] [#uv]
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

                Note:   Blend modes and UV sets can't be changed afterwards.
                        You'll need to create a new Datablock.

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

                Note:   The cmp function is evaluated when creating the renderable cache
        */
        HlmsGui2DMobileDatablock( IdString name, Hlms *creator,
                                  const HlmsMacroblock *macroblock,
                                  const HlmsBlendblock *blendblock,
                                  const HlmsParamVec &params );

        virtual void calculateHash();

        /// If this returns false, the values of mR, mG, mB & mA will be ignored.
        bool hasColour(void) const                      { return mHasColour; }

        /// If this returns false, the value of mIsAlphaTested will be ignored.
        bool isAlphaTested(void) const                  { return mIsAlphaTested; }

        /// Sets a new colour value. Asserts if mHasColour is false.
        void setColour( const ColourValue &diffuse );

        /// Gets the current colour. The returned value is meaningless if mHasColour is false
        ColourValue getColour(void) const               { return ColourValue( mR, mG, mB, mA ); }

        /// Sets a new colour value. Asserts if mIsAlphaTested is false.
        void setAlphaTestThreshold( float alphaThreshold );

        /// Gets the current alpha test threshold. The returned
        /// value is meaningless if mIsAlphaTested is false
        float getAlphaTestThreshold(void) const         { return mAlphaTestThreshold; }

        /** Sets a new texture for rendering
        @param texUnit
            ID of the texture unit. Must be in range [0; mNumTextureUnits) otherwise throws.
        @param newTexture
            Texture to change to. Can't be null, otherwise throws (use a blank texture).
        */
        void setTexture( uint8 texUnit, TexturePtr &newTexture );

        /// Returns the number of texture units.
        uint8 getNumTextureUnits(void) const            { return mNumTextureUnits; }
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
