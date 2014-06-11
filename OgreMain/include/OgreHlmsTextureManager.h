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
#ifndef _OgreHlmsTextureManager_H_
#define _OgreHlmsTextureManager_H_

#include "OgreTexture.h"
#include "OgreIdString.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** HLMS Texture Manager manages textures in the way HLMS expects.
    @par
        Most HLMS implementations, when creating a datablock/material with a texture,
        you can't remove the texture or use a null ptr texture; either you create a new
        datablock, or use an empty dummy texture. This manager ensures you always
        get at least a dummy texture.
    @par
        Its other main job is to provide UV atlas on the fly and/or texture arrays
        on the fly. To do that, we group textures into categories:
            * @TEXTURE_TYPE_DIFFUSE
            * @TEXTURE_TYPE_SPECULAR
            * @TEXTURE_TYPE_NORMALS
            * @TEXTURE_TYPE_ENV_MAP
            * @TEXTURE_TYPE_DETAIL
            * TEXTURE_TYPE_DETAIL_NORMAL_MAP
        Different categories have different default parameters. By default
        all types try to use array textures unless the RenderSystem API doesn't
        support it (in which case fallbacks to UV atlas).
    @par
        For example, Diffuse & detail textures enable hardware gamma correction.
        Normal maps attempt to use BC5 compression, DXT5, or uncompress UV8,
        in that order (depends on HW support).
        Detail maps default to not using UV atlas when texture arrays aren't
        supported (because detail maps are often meant to be tileable), etc
    */
    class _OgreExport HlmsTextureManager : public PassAlloc
    {
    public:
        enum PackingMethod
        {
            TextureArrays,  //DX11, GL3+
            Atlas           //Mobile
        };

        struct DefaultTextureParameters
        {
            PixelFormat pixelFormat;    /// Unknown means assign based on the individual texture
            uint16      maxTexturesPerArray;
            bool        mipmaps;
            bool        hwGammaCorrection;
            PackingMethod packingMethod;

            DefaultTextureParameters() :
                pixelFormat( PF_UNKNOWN ), maxTexturesPerArray( 16 ),
                mipmaps( true ), hwGammaCorrection( false ),
                packingMethod( TextureArrays ) {}
        };

        enum TextureMapType
        {
            TEXTURE_TYPE_DIFFUSE,
            TEXTURE_TYPE_SPECULAR,
            TEXTURE_TYPE_NORMALS,
            TEXTURE_TYPE_ENV_MAP,
            TEXTURE_TYPE_DETAIL,
            TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            NUM_TEXTURE_TYPES
        };

    protected:
        struct TextureArray
        {
            TexturePtr  texture;
            uint16      sqrtMaxTextures;
            uint16      maxTextures;
            bool        automatic;

            StringVector entries;

            TextureArray( uint16 _sqrtMaxTextures, uint16 _maxTextures, bool _automatic ) :
                sqrtMaxTextures( _sqrtMaxTextures ), maxTextures( _maxTextures ), automatic( _automatic )
            {
                entries.reserve( maxTextures );
            }
        };

        struct TextureEntry
        {
            IdString        name;
            TextureMapType  mapType;
            uint16          arrayIdx;
            uint16          entryIdx;

            TextureEntry( IdString _name ) :
                name( _name ), mapType( NUM_TEXTURE_TYPES ), arrayIdx( ~0 ), entryIdx( ~0 ) {}

            TextureEntry( IdString _name, TextureMapType _mapType, uint16 arrayIdx, uint16 entryIdx ) :
                name( _name ), mapType( _mapType ), arrayIdx( arrayIdx), entryIdx( entryIdx ) {}

            inline bool operator < ( const TextureEntry &_right ) const
            {
                return name < _right.name;
            }
        };

        RenderSystem        *mRenderSystem;

        typedef vector<TextureEntry>::type TextureEntryVec;
        typedef vector<TextureArray>::type TextureArrayVec;

        TextureEntryVec     mEntries;
        TextureArrayVec     mTextureArrays[NUM_TEXTURE_TYPES];

        DefaultTextureParameters    mDefaultTextureParameters[NUM_TEXTURE_TYPES];
        size_t              mTextureId;

        TexturePtr mBlankTexture;

        static void copyTextureToArray( const Image &srcImage, TexturePtr dst, uint16 entryIdx );
        static void copyTextureToAtlas( const Image &srcImage, TexturePtr dst,
                                        uint16 entryIdx, uint16 sqrtMaxTextures );

    public:
        HlmsTextureManager();
        virtual ~HlmsTextureManager();

        void initialize(void);

        /** Called when the RenderSystem changes.
        @remarks
            Some settings in mDefaultTextureParameters will be reset
        */
        void _changeRenderSystem( RenderSystem *newRs );

        struct TextureLocation
        {
            TexturePtr  texture;
            uint16      xIdx;
            uint16      yIdx;
            uint16      divisor;
        };

        /** Create a texture based on its name. If a texture with such name has already been
            created, retrieves the existing one.
        @param texName
            Name of the texture (e.g. "myTextureFile.dds" "penguin.jpg")
        @param mapType
            The type of texture map this texture is going to be. See the class overview
            documentation for an explanation of the differences.
            If the texture has already been created, this parameter is ignored.
        @return
            retVal.texture The texture that should be bound

            if packingMethond == TextureArrays
                retVal.xIdx: The array index in the texture array.
            if packingMethond == Atlas
                retVal.xIdx: The U offset to apply to UVs
                retVal.yIdx: The V offset to apply to UVs
                retVal.divisor: The value the original UVs have to be divided for
        */
        TextureLocation createOrRetrieveTexture( const String &texName, TextureMapType mapType );

        /// Returns the precreated blank texture
        TextureLocation getBlankTexture(void) const;

        DefaultTextureParameters* getDefaultTextureParameters(void) { return mDefaultTextureParameters; }
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
