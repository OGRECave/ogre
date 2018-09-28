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
#include "OgreStringVector.h"
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
            * @TEXTURE_TYPE_MONOCHROME
            * @TEXTURE_TYPE_NORMALS
            * @TEXTURE_TYPE_ENV_MAP
            * @TEXTURE_TYPE_DETAIL
            * @TEXTURE_TYPE_DETAIL_NORMAL_MAP
            * @TEXTURE_TYPE_NON_COLOR_DATA
        Different categories have different default parameters. By default
        all types try to use array textures unless the RenderSystem API doesn't
        support it (in which case fallbacks to UV atlas).
    @par
        For example, Diffuse & detail textures enable hardware gamma correction.
        Normal maps attempt to use BC5 compression, or uncompress UV8,
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

        /// Textures whose size are less or equal to minTextureSize
        /// (without considering mipmaps) will have their maxTexturesPerArray
        /// clamped to the value given in this threshold structure
        struct Threshold
        {
            uint32      minTextureSize;
            uint16      maxTexturesPerArray;

            Threshold( uint32 _minTextureSize, uint16 _maxTexturesPerArray ) :
                minTextureSize( _minTextureSize ),
                maxTexturesPerArray( _maxTexturesPerArray )
            {}
        };

        typedef vector<Threshold>::type ThresholdVec;

        struct DefaultTextureParameters
        {
            PixelFormat pixelFormat;    /// Unknown means assign based on the individual texture
            uint16      maxTexturesPerArray;
            bool        mipmaps;
            bool        hwGammaCorrection;
            PackingMethod packingMethod;
            bool        isNormalMap;

            /// Whether non-power-of-2 textures should be packed together.
            bool        packNonPow2;

            /// Textures with a higher AR (whether width / height or height / width) than
            /// this value won't be packed.
            /// Example: packMaxRatio = 2; textures with a resolution of 1024x512 or 512x2048
            /// won't be packed because 1024 / 512 >= 2 and 2048 / 512 >= 2
            float       packMaxRatio;

            /// Only used when packingMethod == TextureArrays
            ThresholdVec    textureArraysTresholds;

            DefaultTextureParameters() :
                pixelFormat( PF_UNKNOWN ), maxTexturesPerArray( 16 ),
                mipmaps( true ), hwGammaCorrection( false ),
                packingMethod( TextureArrays ), isNormalMap( false ),
                packNonPow2( false ), packMaxRatio( 3.0f )
            {
                //This vector must be sorted!
                textureArraysTresholds.push_back( Threshold( 1024 * 1024 / 2, 40 ) );
                textureArraysTresholds.push_back( Threshold( 1024 * 1024 * 1, 20 ) );
                textureArraysTresholds.push_back( Threshold( 1024 * 1024 * 4, 10 ) );
                textureArraysTresholds.push_back( Threshold( 2048 * 2048 * 4, 2 ) );
                textureArraysTresholds.push_back( Threshold( 4096 * 4096 * 4, 1 ) );
            }
        };

        enum TextureMapType
        {
            TEXTURE_TYPE_DIFFUSE,
            TEXTURE_TYPE_MONOCHROME,
            TEXTURE_TYPE_NORMALS,
            TEXTURE_TYPE_ENV_MAP,
            TEXTURE_TYPE_DETAIL,
            TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            TEXTURE_TYPE_NON_COLOR_DATA,
            NUM_TEXTURE_TYPES
        };

        struct MetadataCacheEntry
        {
            TextureMapType mapType;
            uint32 poolId;
            MetadataCacheEntry();
        };

        typedef map<IdString, MetadataCacheEntry>::type MetadataCacheMap;

    protected:
        struct TextureArray
        {
            struct NamePair
            {
                String aliasName;
                String resourceName;
                NamePair() {}
                NamePair( const String &_aliasName, const String &_resourceName ) :
                    aliasName( _aliasName ), resourceName( _resourceName ) {}
            };
            typedef vector<NamePair>::type NamePairVec;

            TexturePtr  texture;
            /// When using UV atlas maxTextures = sqrtMaxTextures * sqrtMaxTextures;
            /// However when using texture arrays, sqrtMaxTextures' value is ignored
            uint16      sqrtMaxTextures;
            uint16      maxTextures;
            bool        automatic;
            bool        isNormalMap;
            bool        manuallyReserved;

            uint16      activeEntries;
            NamePairVec entries;

            uint32      uniqueSpecialId;

            TextureArray( uint16 _sqrtMaxTextures, uint16 _maxTextures, bool _automatic, bool _isNormalMap,
                          bool _manuallyReserved, uint32 _uniqueSpecialId ) :
                sqrtMaxTextures( _sqrtMaxTextures ), maxTextures( _maxTextures ),
                automatic( _automatic ), isNormalMap( _isNormalMap ),
                manuallyReserved( _manuallyReserved ), activeEntries( 0 ),
                uniqueSpecialId( _uniqueSpecialId )
            {
                entries.resize( maxTextures );
            }

            uint16 createEntry(void);
            void destroyEntry( uint16 entry );
        };

        struct TextureEntry
        {
            IdString        name;
            TextureMapType  mapType;
            uint16          arrayIdx;
            uint16          entryIdx;

            TextureEntry( IdString _name ) :
                name( _name ), mapType( NUM_TEXTURE_TYPES ), arrayIdx( uint16(~0) ), entryIdx( uint16(~0) ) {}

            TextureEntry( IdString _name, TextureMapType _mapType, uint16 _arrayIdx, uint16 _entryIdx ) :
                name( _name ), mapType( _mapType ), arrayIdx( _arrayIdx ), entryIdx( _entryIdx ) {}

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
        MetadataCacheMap    mMetadataCache;

        DefaultTextureParameters    mDefaultTextureParameters[NUM_TEXTURE_TYPES];
        size_t              mTextureId;

        TexturePtr mBlankTexture;

        static void copyTextureToArray( const Image &srcImage, TexturePtr dst, uint16 entryIdx,
                                        uint8 srcBaseMip, bool isNormalMap );
        static void copyTextureToAtlas( const Image &srcImage, TexturePtr dst,
                                        uint16 entryIdx, uint16 sqrtMaxTextures,
                                        uint8 srcBaseMip, bool isNormalMap );
        static void copy3DTexture( const Image &srcImage, TexturePtr dst,
                                   uint16 sliceStart, uint16 sliceEnd, uint8 srcBaseMip );

        /// The parameter textureName is for logging purposes
        TextureArrayVec::iterator findSuitableArray( TextureMapType mapType, uint32 width, uint32 height,
                                                     uint32 depth, uint32 faces, PixelFormat format,
                                                     uint8 numMipmaps, uint32 uniqueSpecialId,
                                                     const String &textureName );

        /// Looks for the first image it can successfully load from the pack, and extracts its parameters.
        /// Returns false if failed to retrieve parameters.
        bool getTexturePackParameters( const HlmsTexturePack &pack, uint32 &outWidth, uint32 &outHeight,
                                       uint32 &outDepth, PixelFormat &outPixelFormat ) const;

    public:
        HlmsTextureManager();
        virtual ~HlmsTextureManager();

        void initialize(void);

        /** Called when the RenderSystem changes.
        @remarks
            Some settings in mDefaultTextureParameters will be reset
        */
        void _changeRenderSystem( RenderSystem *newRs );

        /** Reserves a specific pool ID with the given parameters and immediately creates
            that texture.
        @param uniqueSpecialId
        @param mapType
        @param width
        @param height
        @param numSlices
        @param numMipmaps
        @param pixelFormat
        @param isNormalMap
        @param hwGammaCorrection
        @return
            Created texture for the pool.
        */
        TexturePtr reservePoolId( uint32 uniqueSpecialId, TextureMapType mapType,
                                  uint32 width, uint32 height,
                                  uint16 numSlices, uint8 numMipmaps, PixelFormat pixelFormat,
                                  bool isNormalMap, bool hwGammaCorrection );
        bool hasPoolId( uint32 uniqueSpecialId, TextureMapType mapType ) const;

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

        /** See other overload. This one allows aliasing a texture. If you have
            "VERY_TECHNICAL_NAME_HASH_1234.png" as texName, you can make your first
            call with aliasName as "Tree Wood", and the next calls to
            createOrRetrieveTexture( "Tree Wood", mapType ) will refer to this texture
            NOTE: aliasName cannot be blank/empty.
        @param uniqueSpecialId
            Textures loaded with a value uniqueSpecialId != 0 will tried to be pooled together
            inside the same pool that has the same uniqueSpecialId.
            If we have to create more than one pool, you will get log warnings.
            This pool ID is intended for Decals and Area Lights as Ogre implementations only allow
            one global texture per pass; thus all decal/area light texture must have the same
            resolution, pixel format and live in the same array texture.
            See HlmsTextureManager::reservePoolId
        @param imgSource
            When null, texture is loaded from texName as a file. When not null, texture is
            loaded from imgSource and texName is ignored (still used in logging messages though).
            Note imgSource may be modified (e.g. to generate mipmaps).
            Note this pointer is ignored if the texture already exists and is just being retrieved.
        */
        TextureLocation createOrRetrieveTexture( const String &aliasName,
                                                 const String &texName,
                                                 TextureMapType mapType,
                                                 uint32 uniqueSpecialId = 0,
                                                 Image *imgSource = 0 );

        /// Destroys a texture. If the array has multiple entries, the entry for this texture is
        /// sent back to a waiting list for a future new entry. Trying to read from this texture
        /// after this call may result in garbage.
        void destroyTexture( IdString aliasName );

        /// Finds the alias name of a texture given its TextureLocation. Useful for retrieving
        /// back the name of a texture as it was called via createOrRetrieveTexture.
        /// Returns null if not found.
        /// Note the returned pointer may be invalidated if new calls are made to
        /// createOrRetrieveTexture or destroyTexture
        const String* findAliasName( const TextureLocation &textureLocation ) const;
        const String* findResourceNameFromAlias( IdString aliasName ) const;
        /// Output outPoolId is left untouched if returned pointer is null
        const String* findResourceNameFromAlias( IdString aliasName, uint32 &outPoolId ) const;

        void createFromTexturePack( const HlmsTexturePack &pack );

        /** Saves a texture to the given folder. Even if the texture was not created
            by HlmsTextureManager.

            We will not save RenderTargets.

            If the texture is managed by HlmsTextureManager, further information to
            obtain the original filename (even if it's aliased) will be used.
        @param texLocation
        @param folderPath
            Folder where to dump the textures.
        @param savedTextures [in/out]
            Set of texture names. Textures whose name is already in the set won't be saved again.
            Textures that were saved will be inserted into the set.
        @param saveOitd
            When true, we will download the texture from GPU and save it in OITD format.
            OITD is faster to load as it's stored in Ogre's native format it understands,
            but it cannot be opened by traditional image editors; also OITD is not backwards
            compatible with older versions of Ogre.
        @param saveOriginal
            When true, we will attempt to read the raw filestream of the original texture
            and save it (i.e. copy the original png/dds/etc file).
        @param listener
        */
        void saveTexture( const HlmsTextureManager::TextureLocation &texLocation,
                          const String &folderPath, set<String>::type &savedTextures,
                          bool saveOitd, bool saveOriginal,
                          uint32 slice, uint32 numSlices,
                          HlmsTextureExportListener *listener );

        /** Retrieves an entry in the metadata cache that was loaded via
            HlmsTextureManager::importTextureMetadataCache.

            Note that this texture may not even have been ever loaded yet.
        @param aliasName
            Alias of the resource.
        @return
            Null if not found. The cache entry otherwise.
        */
        const HlmsTextureManager::MetadataCacheEntry* getMetadataCacheEntry( IdString aliasName ) const;
        void importTextureMetadataCache( const String &filename, const char *jsonString );
        void exportTextureMetadataCache( String &outJson );
        void clearTextureMetadataCache(void);
        HlmsTextureManager::MetadataCacheMap& getTextureMetadataCache(void) { return mMetadataCache; }

        /// Returns the precreated blank texture
        TextureLocation getBlankTexture(void) const;

        /// Dumps to the Ogre log passed as parameter (if NULL, uses the default one)
        /// in csv format (separator is '|') the usage statistics of all textures currently loaded by the texture manager.
        /// Useful for profiling or determining sources of waste GPU RAM.
        void dumpMemoryUsage( Log* log = NULL ) const;

        DefaultTextureParameters* getDefaultTextureParameters(void) { return mDefaultTextureParameters; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
