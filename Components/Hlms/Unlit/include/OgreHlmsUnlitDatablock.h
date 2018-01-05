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
#ifndef _OgreHlmsUnlitDatablock_H_
#define _OgreHlmsUnlitDatablock_H_

#include "OgreHlmsUnlitPrerequisites.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsTextureManager.h"
#include "OgreConstBufferPool.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    struct UnlitBakedTexture
    {
        TexturePtr              texture;
        HlmsSamplerblock const *samplerBlock;

        UnlitBakedTexture() : samplerBlock( 0 ) {}
        UnlitBakedTexture( const TexturePtr tex, const HlmsSamplerblock *_samplerBlock ) :
            texture( tex ), samplerBlock( _samplerBlock ) {}

        bool operator == ( const UnlitBakedTexture &_r ) const
        {
            return texture == _r.texture && samplerBlock == _r.samplerBlock;
        }
    };

    typedef FastArray<UnlitBakedTexture> UnlitBakedTextureArray;

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL 3+ & D3D11+
    */
    class _OgreHlmsUnlitExport HlmsUnlitDatablock : public HlmsDatablock, public ConstBufferPoolUser
    {
        friend class HlmsUnlit;
    public:
        static const uint8 R_MASK;
        static const uint8 G_MASK;
        static const uint8 B_MASK;
        static const uint8 A_MASK;

        Matrix4 mTextureMatrices[NUM_UNLIT_TEXTURE_TYPES];

    protected:
        uint8   mNumEnabledAnimationMatrices;
        bool    mHasColour;         /// When false; mR, mG, mB & mA aren't passed to the pixel shader
        float   mR, mG, mB, mA;
        uint16  mTexIndices[NUM_UNLIT_TEXTURE_TYPES];

        uint8   mUvSource[NUM_UNLIT_TEXTURE_TYPES];
        uint8   mBlendModes[NUM_UNLIT_TEXTURE_TYPES];
        bool    mEnabledAnimationMatrices[NUM_UNLIT_TEXTURE_TYPES];
        bool    mEnablePlanarReflection[NUM_UNLIT_TEXTURE_TYPES];

        UnlitBakedTextureArray mBakedTextures;
        /// The way to read this variable is i.e. get diffuse texture 0,
        /// mBakedTextures[mTexToBakedTextureIdx[0]]
        /// Then read mTexIndices[0] to know which slice of the texture array.
        uint8   mTexToBakedTextureIdx[NUM_UNLIT_TEXTURE_TYPES];
        uint8   mTextureSwizzles[NUM_UNLIT_TEXTURE_TYPES];

        HlmsSamplerblock const  *mSamplerblocks[NUM_UNLIT_TEXTURE_TYPES];

        virtual void cloneImpl( HlmsDatablock *datablock ) const;

        void scheduleConstBufferUpdate(void);
        virtual void uploadToConstBuffer( char *dstPtr );
        virtual void uploadToExtraBuffer( char *dstPtr );

        /// Sets the appropiate mTexIndices[texUnit], and returns the texture pointer
        TexturePtr setTexture( const String &name, uint8 texUnit );

        void decompileBakedTextures( UnlitBakedTexture outTextures[NUM_UNLIT_TEXTURE_TYPES] );
        void bakeTextures( const UnlitBakedTexture textures[NUM_UNLIT_TEXTURE_TYPES] );

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

             * animate <#tex_unit> [<#tex_unit> <#tex_unit> ... <#tex_unit>]
                Enables texture animation through a 4x4 matrix for the specified textures.
                Default: All texture animation/manipulation disabled.
                Example: animate 0 1 2 3 4 14 15

             * alpha_test [compare_func] [threshold]
                When present, mAlphaTestThreshold is used.
                compare_func is optional. Valid values are:
                    less, less_equal, equal, greater, greater_equal, not_equal
                Threshold is optional, and a value in the range (0; 1)
                Default: alpha_test less 0.5
                Example: alpha_test equal 0.1
        */
        HlmsUnlitDatablock( IdString name, HlmsUnlit *creator,
                            const HlmsMacroblock *macroblock,
                            const HlmsBlendblock *blendblock,
                            const HlmsParamVec &params );
        virtual ~HlmsUnlitDatablock();

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

        /** Sets a new texture for rendering. Calling this function may trigger an
            HlmsDatablock::flushRenderables if the texture or the samplerblock changes.
            Won't be called if only the arrayIndex changes
        @param texType
            Texture unit. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param arrayIndex
            The index in the array texture.
        @param newTexture
            Texture to change to. If it is null and previously wasn't (or viceversa), will
            trigger HlmsDatablock::flushRenderables.
        @param params
            Optional. We'll create (or retrieve an existing) samplerblock based on the input parameters.
            When null, we leave the previously set samplerblock (if a texture is being set, and if no
            samplerblock was set, we'll create a default one)
        */
        void setTexture( uint8 texType, uint16 arrayIndex, const TexturePtr &newTexture,
                         const HlmsSamplerblock *refParams=0 );

        TexturePtr getTexture( uint8 texType ) const;

        /// Returns the internal index to the array in a texture array.
        /// Note: If there is no texture assigned to the given texType, returned value is undefined
        uint16 _getTextureIdx( uint8 texType ) const                    { return mTexIndices[texType]; }

        /** Sets the final swizzle when sampling the given texture. e.g.
            calling setTextureSwizzle( 0, R_MASK, G_MASK, R_MASK, G_MASK );
            will generated the following pixel shader:
                vec4 texColour = texture( tex, uv ).xyxy;

            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param texType
            Texture unit. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param r
            Where to source the red channel from. Must be R_MASK, G_MASK, B_MASK or A_MASK.
            Default: R_MASK
        @param g
            Where to source the green channel from.
            Default: G_MASK
        @param b
            Where to source the blue channel from.
            Default: B_MASK
        @param a
            Where to source the alpha channel from.
            Default: A_MASK
        */
        void setTextureSwizzle( uint8 texType, uint8 r, uint8 g, uint8 b, uint8 a );

        /** Sets a new sampler block to be associated with the texture
            (i.e. filtering mode, addressing modes, etc). If the samplerblock changes,
            this function will always trigger a HlmsDatablock::flushRenderables
        @param texType
            Texture unit. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param params
            The sampler block to use as reference.
        */
        void setSamplerblock( uint8 texType, const HlmsSamplerblock &params );

        const HlmsSamplerblock* getSamplerblock( uint8 texType ) const;

        /** Sets which UV set to use for the given texture.
            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param sourceType
            Source texture to modify. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param uvSet
            UV coordinate set. Value must be between in range [0; 8)
        */
        void setTextureUvSource( uint8 sourceType, uint8 uvSet );
        uint8 getTextureUvSource( uint8 sourceType ) const;

        /** Sets the blending mode (how the texture unit gets layered
            on top of the previous texture units).
            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param texType
            The texture unit. Must be in range [1; NUM_UNLIT_TEXTURE_TYPES)
            The value 0 is ignored.
        @param blendMode
            The blending mode to use.
        */
        void setBlendMode( uint8 texType, UnlitBlendModes blendMode );
        UnlitBlendModes getBlendMode( uint8 texType ) const;

        /** Enables the animation of the given texture unit.
            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param textureUnit
            Texture unit. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param bEnable
            Whether to enable or disable. Default is disabled.
        */
        void setEnableAnimationMatrix( uint8 textureUnit, bool bEnable );
        bool getEnableAnimationMatrix( uint8 textureUnit ) const;

        void setAnimationMatrix( uint8 textureUnit, const Matrix4 &matrix );
        const Matrix4 & getAnimationMatrix( uint8 textureUnit ) const;

        /** Set to true if the texture at the given texture unit is a planar
            reflection texture. UVs will be ignored for that texture unit.
            Calling this function triggers a HlmsDatablock::flushRenderables.
        @param textureUnit
            Texture unit. Must be in range [0; NUM_UNLIT_TEXTURE_TYPES)
        @param bEnable
            Whether to enable or disable. Default is disabled.
        */
        void setEnablePlanarReflection( uint8 textureUnit, bool bEnable );
        bool getEnablePlanarReflection( uint8 textureUnit ) const;

        /// Returns the index to mBakedTextures. Returns NUM_PBSM_TEXTURE_TYPES if
        /// there is no texture assigned to texType
        uint8 getBakedTextureIdx( uint8 texType ) const;

        virtual void calculateHash();

        static const size_t MaterialSizeInGpu;
        static const size_t MaterialSizeInGpuAligned;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
