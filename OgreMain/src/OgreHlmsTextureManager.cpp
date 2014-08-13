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

#include "OgreStableHeaders.h"

#include "OgreHlmsTextureManager.h"
#include "OgreHlmsTexturePack.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"

namespace Ogre
{
    HlmsTextureManager::HlmsTextureManager() : mRenderSystem( 0 ), mTextureId( 0 )
    {
        mDefaultTextureParameters[TEXTURE_TYPE_DIFFUSE].hwGammaCorrection   = true;
        mDefaultTextureParameters[TEXTURE_TYPE_MONOCHROME].pixelFormat      = PF_L8;
        mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat         = PF_BC5_SNORM;
        mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].isNormalMap         = true;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].hwGammaCorrection    = true;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat=PF_BC5_SNORM;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].isNormalMap = true;
        mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].hwGammaCorrection   = true;
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::~HlmsTextureManager()
    {
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::_changeRenderSystem( RenderSystem *newRs )
    {
        mRenderSystem = newRs;

        if( mRenderSystem )
        {
            const RenderSystemCapabilities *caps = mRenderSystem->getCapabilities();

            if( caps )
            {
                TextureType textureType = TEX_TYPE_2D;

                if( caps->hasCapability(RSC_TEXTURE_2D_ARRAY) && false ) //TODO
                {
                    textureType = TEX_TYPE_2D_ARRAY;

                    for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
                    {
                        mDefaultTextureParameters[i].packingMethod = TextureArrays;
                        mDefaultTextureParameters[i].maxTexturesPerArray = 40;
                    }

                    mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 20;
                    if( !caps->hasCapability(RSC_TEXTURE_CUBE_MAP_ARRAY) )
                        mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 1;
                }
                else
                {
                    for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
                    {
                        mDefaultTextureParameters[i].packingMethod = Atlas;
                        mDefaultTextureParameters[i].maxTexturesPerArray = 1;
                    }
                    mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].maxTexturesPerArray  = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].maxTexturesPerArray = 1;
                }

                bool hwGammaCorrection = caps->hasCapability( RSC_HW_GAMMA );
                mDefaultTextureParameters[TEXTURE_TYPE_DIFFUSE].hwGammaCorrection   = hwGammaCorrection;
                mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].hwGammaCorrection    = hwGammaCorrection;

                // BC5 is the best, native (lossy) compressor for normal maps.
                // DXT5 is like BC5, using the "store only in green and alpha channels" method.
                // The last one is lossless, using UV8 to store uncompressed,
                // and retrieve z = sqrt(x²+y²)
                /*if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat = PF_BC5_SNORM;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_BC5_SNORM;
                }
                else if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = PF_DXT5;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_DXT5;
                }
                else*/
                {
                    PixelFormat pf = caps->hasCapability( RSC_TEXTURE_SIGNED_INT ) ? PF_R8G8_SNORM :
                                                                                     PF_BYTE_LA;
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = pf;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = pf;
                }

                mBlankTexture = TextureManager::getSingleton().createManual( "Hlms_Blanktexture",
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                textureType, 4, 4, 1, 0, PF_R8G8B8A8, TU_DEFAULT, 0,
                                                false, 0, BLANKSTRING, false );

                HardwarePixelBufferSharedPtr pixelBufferBuf = mBlankTexture->getBuffer(0);
                const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0, 4, 4, 1 ),
                                                                  HardwareBuffer::HBL_DISCARD );
                uint8 *data = reinterpret_cast<uint8*>( currImage.data );
                for( size_t y=0; y<currImage.getHeight(); ++y )
                {
                    for( size_t x=0; x<currImage.getWidth(); ++x )
                    {
                        *data++ = 255;
                        *data++ = 255;
                        *data++ = 255;
                        *data++ = 255;
                    }

                    data += ( currImage.rowPitch - currImage.getWidth() ) * 4;
                }
                pixelBufferBuf->unlock();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copyTextureToArray( const Image &srcImage, TexturePtr dst, uint16 entryIdx,
                                                 uint8 srcBaseMip )
    {
        //TODO: Deal with mipmaps (& cubemaps & 3D? does it work?). We could have:
        //  * Original image has mipmaps, we use them all
        //  * Original image has mipmaps, we discard them
        //  * Original image has mipmaps, we use them, but we need to create more
        //  * Original image doesn't have mipmaps, but we need to create them
        // The last problem is a subset of the 3rd problem
        //
        //  See Texture::_loadImages
        HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0);
        const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, entryIdx,
                                                               pixelBufferBuf->getWidth(),
                                                               pixelBufferBuf->getHeight(),
                                                               entryIdx + 1 ),
                                                          HardwareBuffer::HBL_DISCARD );
        PixelUtil::bulkPixelConversion( srcImage.getPixelBox(0, srcBaseMip), currImage );
        pixelBufferBuf->unlock();
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copyTextureToAtlas( const Image &srcImage, TexturePtr dst,
                                                 uint16 entryIdx, uint16 sqrtMaxTextures,
                                                 uint8 srcBaseMip, bool isNormalMap )
    {
        //TODO: Deal with mipmaps (& cubemaps & 3D? does it work?).
        size_t xBlock = entryIdx % sqrtMaxTextures;
        size_t yBlock = entryIdx / sqrtMaxTextures;

        size_t nextX = ( entryIdx % sqrtMaxTextures ) + 1;
        size_t nextY = ( entryIdx / sqrtMaxTextures ) + 1;

        /*if( sqrtMaxTextures > 1 && PixelUtil::isCompressed( dst->getFormat() ) )
        {
            HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0,
                                                                   dst->getWidth(),
                                                                   dst->getHeight(),
                                                                   dst->getDepth() ),
                                                              HardwareBuffer::HBL_DISCARD );
                                                              //HardwareBuffer::HBL_NORMAL );
            PixelUtil::bulkCompressedSubregion( srcImage.getPixelBox(0, srcBaseMip), currImage,
                                                Box( xBlock * srcImage.getWidth(),
                                                     yBlock * srcImage.getHeight(),
                                                     0,
                                                     nextX * srcImage.getWidth(),
                                                     nextY * srcImage.getHeight(),
                                                     dst->getDepth() ) );
            pixelBufferBuf->unlock();
        }
        else*/
        {
            HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( xBlock * srcImage.getWidth(),
                                                                   yBlock * srcImage.getHeight(),
                                                                   0,
                                                                   nextX * srcImage.getWidth(),
                                                                   nextY * srcImage.getHeight(),
                                                                   dst->getDepth() ),
                                                              HardwareBuffer::HBL_DISCARD );
            if( isNormalMap )
                PixelUtil::convertForNormalMapping( srcImage.getPixelBox(0, srcBaseMip), currImage );
            else
                PixelUtil::bulkPixelConversion( srcImage.getPixelBox(0, srcBaseMip), currImage );
            pixelBufferBuf->unlock();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copy3DTexture( const Image &srcImage, TexturePtr dst,
                                            uint16 sliceStart, uint16 sliceEnd, uint8 srcBaseMip )
    {
        for( uint16 i=sliceStart; i<sliceEnd; ++i )
        {
            uint8 minMipmaps = std::min<uint8>( srcImage.getNumMipmaps() - srcBaseMip,
                                                dst->getNumMipmaps() ) + 1;
            for( uint8 j=0; j<minMipmaps; ++j )
            {
                HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer( i, j );
                const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0,
                                                                       pixelBufferBuf->getWidth(),
                                                                       pixelBufferBuf->getHeight(),
                                                                       1 ),
                                                                  HardwareBuffer::HBL_DISCARD );

                PixelUtil::bulkPixelConversion( srcImage.getPixelBox( i - sliceStart,
                                                                      srcBaseMip + j ),
                                                currImage );
                pixelBufferBuf->unlock();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::createOrRetrieveTexture(
                                                                        const String &texName,
                                                                        TextureMapType mapType )
    {
        return createOrRetrieveTexture( texName, texName, mapType );
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::createOrRetrieveTexture(
                                                                        const String &aliasName,
                                                                        const String &texName,
                                                                        TextureMapType mapType )
    {
        TextureEntry searchName( aliasName );
        TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(), searchName );

        TextureLocation retVal;

        try
        {
        if( it == mEntries.end() || it->name != searchName.name )
        {
            Image image;
            image.load( texName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

            PixelFormat imageFormat = image.getFormat();
            if( imageFormat == PF_X8R8G8B8 || imageFormat == PF_R8G8B8 )
                imageFormat = PF_A8R8G8B8;
            else if( imageFormat == PF_X8B8G8R8 || imageFormat == PF_B8G8R8 )
                imageFormat = PF_A8B8G8R8;

            //Find an array where we can put it. If there is none, we'll have have to create a new one
            TextureArrayVec::iterator itor = mTextureArrays[mapType].begin();
            TextureArrayVec::iterator end  = mTextureArrays[mapType].end();

            bool bFound = false;

            while( itor != end && !bFound )
            {
                TextureArray &textureArray = *itor;

                size_t arrayTexWidth = textureArray.texture->getWidth() / textureArray.sqrtMaxTextures;
                size_t arrayTexHeight= textureArray.texture->getHeight() / textureArray.sqrtMaxTextures;
                if( textureArray.automatic &&
                    textureArray.entries.size() < textureArray.maxTextures &&
                    arrayTexWidth  == image.getWidth()  &&
                    arrayTexHeight == image.getHeight() &&
                    textureArray.texture->getDepth()  == image.getDepth()  &&
                    textureArray.texture->getFormat() == imageFormat )
                {
                    //Bingo! Add this.
                    if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                    {
                        copyTextureToArray( image, textureArray.texture,
                                            textureArray.entries.size(), 0 );
                    }
                    else
                    {
                        copyTextureToAtlas( image, textureArray.texture,
                                            textureArray.entries.size(), textureArray.sqrtMaxTextures,
                                            textureArray.isNormalMap, 0 );
                    }

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            itor - mTextureArrays[mapType].begin(),
                                                            textureArray.entries.size() ) );

                    textureArray.entries.push_back( aliasName );

                    bFound = true;
                }

                ++itor;
            }

            if( !bFound )
            {
                PixelFormat defaultPixelFormat = mDefaultTextureParameters[mapType].pixelFormat;
                //Create a new array
                /* Disabled to support the same texture being loaded twice with
                   different parameters, and a different alias.
                if( mDefaultTextureParameters[mapType].maxTexturesPerArray == 1 )
                {
                    TextureType texType = TEX_TYPE_2D;
                    if( image.hasFlag( IF_3D_TEXTURE ) )
                        texType = TEX_TYPE_3D;
                    else if( image.hasFlag( IF_CUBEMAP ) )
                        texType = TEX_TYPE_CUBE_MAP;

                    TextureArray textureArray( 1, 1, true );
                    textureArray.texture = TextureManager::getSingleton().load( texName,
                                                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                texType, mDefaultTextureParameters[mapType].mipmaps ?
                                                                                        MIP_DEFAULT : 0,
                                                1.0f, false,
                                                defaultPixelFormat == PF_UNKNOWN ? image.getFormat() :
                                                                                   defaultPixelFormat,
                                                mDefaultTextureParameters[mapType].hwGammaCorrection );
                    textureArray.entries.push_back( aliasName );

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            mTextureArrays[mapType].size(), 0 ) );

                    mTextureArrays[mapType].push_back( textureArray );
                }
                else*/
                {
                    uint limit = mDefaultTextureParameters[mapType].maxTexturesPerArray;
                    uint limitSquared = mDefaultTextureParameters[mapType].maxTexturesPerArray;

                    if( mDefaultTextureParameters[mapType].packingMethod == Atlas )
                    {
                        limit        = static_cast<uint>( ceilf( sqrtf( (Real)limitSquared ) ) );
                        limitSquared = limit * limit;
                    }

                    TextureArray textureArray( limit, limitSquared, true,
                                               mDefaultTextureParameters[mapType].isNormalMap );

                    TextureType texType = TEX_TYPE_2D;

                    uint width, height, depth;
                    uint8 mipLevel = 0;

                    depth = image.getDepth();

                    if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                    {
                        //Texture Arrays
                        width  = image.getWidth();
                        height = image.getHeight();

                        if( image.hasFlag( IF_3D_TEXTURE ) )
                        {
                            texType = TEX_TYPE_3D;
                            //APIs don't support arrays + 3D textures
                            textureArray.maxTextures = 1;
                        }
                        else
                        {
                            if( image.hasFlag( IF_CUBEMAP ) )
                            {
                                //TODO: Cubemap arrays supported since D3D10.1
                                texType = TEX_TYPE_CUBE_MAP;
                                depth = image.getNumFaces();
                            }
                            else
                            {
                                texType = TEX_TYPE_2D_ARRAY;
                                depth = textureArray.maxTextures;
                            }
                        }
                    }
                    else
                    {
                        //UV Atlas
                        const RenderSystemCapabilities *caps = mRenderSystem->getCapabilities();
                        ushort maxResolution = caps->getMaximumResolution2D();
                        if( image.hasFlag( IF_3D_TEXTURE ) )
                        {
                            maxResolution = caps->getMaximumResolution3D();
                            texType = TEX_TYPE_3D;
                            textureArray.maxTextures = 1; //No UV atlas for 3D
                        }
                        else if( image.hasFlag( IF_CUBEMAP ) )
                        {
                            maxResolution = caps->getMaximumResolutionCubemap();
                            texType = TEX_TYPE_CUBE_MAP;
                            textureArray.maxTextures = 1; //No UV atlas for Cubemaps
                            depth = image.getNumFaces();
                        }

                        width  = image.getWidth() * limit;
                        height = image.getHeight() * limit;

                        if( !maxResolution )
                        {
                            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                         "Maximum resolution for this type of texture is 0.\n"
                                         "Either a driver bug, or this GPU cannot support 2D/"
                                         "Cubemap/3D texture: " + texName,
                                         "HlmsTextureManager::createOrRetrieveTexture" );
                        }

                        if( width > maxResolution || height > maxResolution )
                        {
                            uint imageWidth  = image.getWidth();
                            uint imageHeight = image.getHeight();

                            if( imageWidth > maxResolution || height > maxResolution )
                            {
                                //Ok, not even a single texture can fit the Atlas.
                                //Take a smaller mip. If the texture doesn't have
                                //mipmaps, resize it.
                                bool resize = true;
                                if( image.getNumMipmaps() )
                                {
                                    resize = false;
                                    while( (imageWidth > maxResolution || height > maxResolution)
                                           && (mipLevel <= image.getNumMipmaps()) )
                                    {
                                        imageWidth  >>= 1;
                                        imageHeight >>= 1;
                                        ++mipLevel;
                                    }

                                    if( (imageWidth > maxResolution || height > maxResolution) )
                                        resize = true;
                                }

                                if( resize )
                                {
                                    mipLevel = 0;
                                    Real aspectRatio = (Real)image.getWidth() / (Real)image.getHeight();
                                    if( image.getWidth() >= image.getHeight() )
                                    {
                                        imageWidth  = maxResolution;
                                        imageHeight = static_cast<uint>( floorf( maxResolution /
                                                                                 aspectRatio ) );
                                    }
                                    else
                                    {
                                        imageWidth  = static_cast<uint>( floorf( maxResolution *
                                                                                 aspectRatio ) );
                                        imageHeight = maxResolution;
                                    }

                                    image.resize( maxResolution, maxResolution );
                                }
                            }

                            limit = maxResolution / imageWidth;
                            limit = std::min<uint>( limit, maxResolution / imageHeight );

                            textureArray.maxTextures     = limit * limit;
                            textureArray.sqrtMaxTextures = limit;
                            width  = imageWidth  * limit;
                            height = imageHeight * limit;
                        }
                    }

                    textureArray.texture = TextureManager::getSingleton().createManual(
                                                "HlmsTextureManager/" +
                                                StringConverter::toString( mTextureId++ ),
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                texType, width, height, depth,
                                                mDefaultTextureParameters[mapType].mipmaps ?
                                                                                        MIP_DEFAULT : 0,
                                                defaultPixelFormat == PF_UNKNOWN ? imageFormat :
                                                                                   defaultPixelFormat,
                                                TU_DEFAULT & ~TU_AUTOMIPMAP, 0,
                                                mDefaultTextureParameters[mapType].hwGammaCorrection,
                                                0, BLANKSTRING, false );

                    if( texType != TEX_TYPE_3D && texType != TEX_TYPE_CUBE_MAP )
                    {
                        if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                        {
                            copyTextureToArray( image, textureArray.texture,
                                                textureArray.entries.size(), mipLevel );
                        }
                        else
                        {
                            copyTextureToAtlas( image, textureArray.texture, textureArray.entries.size(),
                                                textureArray.sqrtMaxTextures, mipLevel,
                                                textureArray.isNormalMap );
                        }
                    }
                    else
                    {
                        copy3DTexture( image, textureArray.texture, 0,
                                       std::max<uint32>( image.getNumFaces(), image.getDepth() ),
                                       mipLevel );
                    }

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            mTextureArrays[mapType].size(), 0 ) );

                    textureArray.entries.push_back( aliasName );
                    mTextureArrays[mapType].push_back( textureArray );
                }
            }
        }

        const TextureArray &texArray = mTextureArrays[it->mapType][it->arrayIdx];
        retVal.texture = texArray.texture;

        if( !texArray.texture->isTextureTypeArray() )
        {
            retVal.xIdx = it->entryIdx % texArray.sqrtMaxTextures;
            retVal.yIdx = it->entryIdx / texArray.sqrtMaxTextures;
            retVal.divisor= texArray.sqrtMaxTextures;
        }
        else
        {
            retVal.xIdx = it->entryIdx;
            retVal.yIdx = 0;
            retVal.divisor= 1;
        }
        }
        catch( Exception &e )
        {
            if( e.getNumber() != Exception::ERR_FILE_NOT_FOUND )
                throw e;
            else
            {
                LogManager::getSingleton().logMessage( LML_CRITICAL, e.getFullDescription() );
                retVal.texture  = mBlankTexture;
                retVal.xIdx     = 0;
                retVal.yIdx     = 0;
                retVal.divisor  = 1;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    bool HlmsTextureManager::getTexturePackParameters( const HlmsTexturePack &pack, uint32 &outWidth,
                                                       uint32 &outHeight, uint32 &outDepth,
                                                       PixelFormat &outPixelFormat ) const
    {
        HlmsTexturePack::TextureEntryVec::const_iterator itor = pack.textureEntry.begin();
        HlmsTexturePack::TextureEntryVec::const_iterator end  = pack.textureEntry.end();

        while( itor != end )
        {
            const HlmsTexturePack::TextureEntry &texInfo = *itor;

            StringVector::const_iterator itPath = texInfo.paths.begin();
            StringVector::const_iterator enPath = texInfo.paths.end();

            while( itPath != enPath )
            {
                try
                {
                    Image image;
                    image.load( *itPath, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
                    outWidth    = image.getWidth();
                    outHeight   = image.getHeight();
                    outDepth    = std::max<uint32>( image.getDepth(), image.getNumFaces() );
                    outPixelFormat = image.getFormat();

                    return true;
                }
                catch( ... )
                {
                }

                ++itPath;
            }

            ++itor;
        }

        return false;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::createFromTexturePack( const HlmsTexturePack &pack )
    {
        uint32 width = 0, height = 0, depth = 0;
        PixelFormat pixelFormat;
        uint8 numMipmaps = 0;

        if( !getTexturePackParameters( pack, width, height, depth, pixelFormat ) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Could not derive the texture properties "
                         "for texture pack '" + pack.name + "'",
                         "HlmsTextureManager::createFromTexturePack" );
        }

        if( pack.pixelFormat != PF_UNKNOWN )
            pixelFormat = pack.pixelFormat;

        if( pack.hasMipmaps )
        {
            uint32 heighestRes = std::max( std::max( width, height ), depth );
#if (ANDROID || (OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1700))
            numMipmaps = static_cast<uint8>( floorf( logf( static_cast<float>(heighestRes) ) / logf( 2.0f ) ) );
#else
            numMipmaps = static_cast<uint8>( floorf( log2f( static_cast<float>(heighestRes) ) ) );
#endif
        }

        if( pack.textureType == TEX_TYPE_CUBE_MAP )
        {
            HlmsTexturePack::TextureEntryVec::const_iterator itor = pack.textureEntry.begin();
            HlmsTexturePack::TextureEntryVec::const_iterator end  = pack.textureEntry.end();

            while( itor != end )
            {
                const HlmsTexturePack::TextureEntry &texInfo = *itor;

                TextureEntry searchName( texInfo.name );
                TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(),
                                                                 searchName );

                if( it != mEntries.end() )
                {
                    LogManager::getSingleton().logMessage( "ERROR: A texture by the name '" +
                                                           texInfo.name  + "' already exists!" );
                    ++itor;
                    continue;
                }

                assert( !texInfo.paths.empty() ) ;

                if( texInfo.paths.size() != 1 )
                {
                    //Multiple files
                    assert( !(texInfo.paths.size() % 6) &&
                            "For cubemaps, the number of files must be multiple of 6!" );

                    Image cubeMap;
                    size_t cubeMapSize = PixelUtil::getMemorySize( width, height, 6, pixelFormat );
                    cubeMap.loadDynamicImage( OGRE_ALLOC_T( uchar, cubeMapSize, MEMCATEGORY_GENERAL ),
                                              width, height, 1, pixelFormat, true, 6 );
                    for( size_t i=0; i<6; ++i )
                    {
                        Image image;
                        image.load( texInfo.paths[i],
                                    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

                        if( image.getWidth() != width && image.getHeight() != height )
                        {
                            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, texInfo.paths[i] +
                                         ": All textures in the same pack must have the "
                                         "same resolution!",
                                         "HlmsTextureManager::createFromTexturePack" );
                        }

                        PixelUtil::bulkPixelConversion( image.getPixelBox( 0 ), cubeMap.getPixelBox( i ) );
                    }

                    TextureArray textureArray( 1, 1, false, false );

                    textureArray.texture = TextureManager::getSingleton().createManual(
                                                "HlmsTextureManager/" +
                                                StringConverter::toString( mTextureId++ ),
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                pack.textureType, width, height, depth, numMipmaps,
                                                pixelFormat, TU_DEFAULT & ~TU_AUTOMIPMAP, 0,
                                                pack.hwGammaCorrection, 0, BLANKSTRING, false );

                    if( pack.hasMipmaps )
                    {
                        if( !cubeMap.generateMipmaps( pack.hwGammaCorrection, Image::FILTER_GAUSSIAN ) )
                        {
                            LogManager::getSingleton().logMessage( "Couldn't generate mipmaps for '" +
                                                                    texInfo.name + "'", LML_CRITICAL );
                        }
                    }

                    copy3DTexture( cubeMap, textureArray.texture, 0, 6, 0 );

                    it = mEntries.insert( it, TextureEntry( searchName.name, TEXTURE_TYPE_ENV_MAP,
                                                            mTextureArrays[TEXTURE_TYPE_ENV_MAP].size(), 0 ) );

                    textureArray.entries.push_back( texInfo.name );
                    mTextureArrays[TEXTURE_TYPE_ENV_MAP].push_back( textureArray );
                }
                else
                {
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                 "Oops! Work in Progress, sorry!",
                                 "HlmsTextureManager::createFromTexturePack" );
                    //TODO
                    /*if( image.getNumMipmaps() != numMipmaps )
                    {
                        image.generateMipmaps();
                    }*/
                }

                ++itor;
            }
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Oops! Work in Progress, sorry!",
                         "HlmsTextureManager::createFromTexturePack" );
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::getBlankTexture(void) const
    {
        TextureLocation retVal;
        retVal.texture  = mBlankTexture;
        retVal.xIdx     = 0;
        retVal.yIdx     = 0;
        retVal.divisor  = 1;

        return retVal;
    }
}
