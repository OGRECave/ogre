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
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
    HlmsTextureManager::HlmsTextureManager() : mRenderSystem( 0 )
    {
        mDefaultTextureParameters[TEXTURE_TYPE_DIFFUSE].hwGammaCorrection   = true;
        mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat         = PF_BC5_SNORM;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].hwGammaCorrection    = true;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat=PF_BC5_SNORM;
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
                        mDefaultTextureParameters[i].maxTexturesPerArray = 16;
                    }
                    mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].maxTexturesPerArray  = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].maxTexturesPerArray = 1;
                }

                // BC5 is the best, native (lossy) compressor for normal maps.
                // DXT5 is like BC5, using the "store only in green and alpha channels" method.
                // The last one is lossless, using UV8 to store uncompressed,
                // and retrieve z = sqrt(x²+y²)
                if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat = PF_BC5_SNORM;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_BC5_SNORM;
                }
                else if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = PF_DXT5;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_DXT5;
                }
                else
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = PF_R8G8_SNORM;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_R8G8_SNORM;
                }

                mBlankTexture = TextureManager::getSingleton().createManual( "Hlms_Blanktexture",
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                textureType, 4, 4, 1, 0, PF_R8G8B8A8, TU_DEFAULT, 0,
                                                true, 0, BLANKSTRING, false );

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
    void HlmsTextureManager::copyTextureToArray( const Image &srcImage, TexturePtr dst, uint16 entryIdx )
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
                                                               dst->getWidth(),
                                                               dst->getHeight(),
                                                               entryIdx + 1 ),
                                                          HardwareBuffer::HBL_DISCARD );
        PixelUtil::bulkPixelConversion( srcImage.getPixelBox(), currImage );
        pixelBufferBuf->unlock();
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copyTextureToAtlas(const Image &srcImage, TexturePtr dst,
                                          uint16 entryIdx, uint16 sqrtMaxTextures )
    {
        //TODO: Deal with mipmaps (& cubemaps & 3D? does it work?).
        size_t xBlock = entryIdx % sqrtMaxTextures;
        size_t yBlock = entryIdx / sqrtMaxTextures;

        size_t nextX = ( entryIdx % sqrtMaxTextures ) + 1;
        size_t nextY = ( entryIdx / sqrtMaxTextures ) + 1;

        HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0);
        const PixelBox &currImage = pixelBufferBuf->lock( Box( xBlock * srcImage.getWidth(),
                                                               yBlock * srcImage.getHeight(),
                                                               0,
                                                               nextX * srcImage.getWidth(),
                                                               nextY * srcImage.getHeight(),
                                                               dst->getDepth() ),
                                                          HardwareBuffer::HBL_DISCARD );
        PixelUtil::bulkPixelConversion( srcImage.getPixelBox(), currImage );
        pixelBufferBuf->unlock();
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::createOrRetrieveTexture(
                                                                        const String &texName,
                                                                        TextureMapType mapType )
    {
        TextureEntry searchName( texName );
        TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(), searchName );

        TextureLocation retVal;

        try
        {
        if( it == mEntries.end() || it->name != searchName.name )
        {
            Image image;
            image.load( texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

            //Find an array where we can put it. If there is none, we'll have have to create a new one
            TextureArrayVec::iterator itor = mTextureArrays[mapType].begin();
            TextureArrayVec::iterator end  = mTextureArrays[mapType].end();

            bool bFound = false;

            while( itor != end && !bFound )
            {
                TextureArray &textureArray = *itor;
                if( textureArray.automatic &&
                    textureArray.entries.size() < textureArray.maxTextures &&
                    textureArray.texture->getWidth()  == image.getWidth()  &&
                    textureArray.texture->getHeight() == image.getHeight() &&
                    textureArray.texture->getDepth()  == image.getDepth()  &&
                    textureArray.texture->getFormat() == image.getFormat() )
                {
                    //Bingo! Add this.
                    if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                    {
                        copyTextureToArray( image, textureArray.texture, textureArray.entries.size() );
                    }
                    else
                    {
                        copyTextureToAtlas( image, textureArray.texture,
                                            textureArray.entries.size(), textureArray.maxTextures );
                    }

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            itor - mTextureArrays[mapType].begin(),
                                                            textureArray.entries.size() ) );

                    textureArray.entries.push_back( texName );

                    bFound = true;
                }

                ++itor;
            }

            if( !bFound )
            {
                PixelFormat defaultPixelFormat = mDefaultTextureParameters[mapType].pixelFormat;
                //Create a new array
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
                    textureArray.entries.push_back( texName );
                    mTextureArrays[mapType].push_back( textureArray );

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            mTextureArrays[mapType].size(), 0 ) );
                }
                else
                {
                    uint limit = mDefaultTextureParameters[mapType].maxTexturesPerArray;
                    uint limitSquared = mDefaultTextureParameters[mapType].maxTexturesPerArray;

                    if( mDefaultTextureParameters[mapType].packingMethod == Atlas )
                    {
                        limit        = static_cast<uint>( ceilf( sqrtf( limitSquared ) ) );
                        limitSquared = limit * limit;
                    }

                    TextureArray textureArray( limit, limitSquared, true );

                    TextureType texType = TEX_TYPE_2D;

                    uint width, height, depth;

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

                        if( width  > maxResolution || height > maxResolution )
                        {
                            limit = maxResolution / width;
                            limit = std::min<uint>( limit, maxResolution / height );

                            textureArray.maxTextures     = limit * limit;
                            textureArray.sqrtMaxTextures = limit;
                            width  = image.getWidth() * limit;
                            height = image.getHeight() * limit;
                        }
                    }

                    textureArray.texture = TextureManager::getSingleton().createManual( "TODO",
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                texType, width, height, depth,
                                                mDefaultTextureParameters[mapType].mipmaps ?
                                                                                        MIP_DEFAULT : 0,
                                                defaultPixelFormat == PF_UNKNOWN ? image.getFormat() :
                                                                                   defaultPixelFormat,
                                                TU_DEFAULT, 0,
                                                mDefaultTextureParameters[mapType].hwGammaCorrection,
                                                0, BLANKSTRING, false );

                    if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                    {
                        copyTextureToArray( image, textureArray.texture, textureArray.entries.size() );
                    }
                    else
                    {
                        copyTextureToAtlas( image, textureArray.texture, textureArray.entries.size(),
                                            textureArray.sqrtMaxTextures );
                    }

                    it = mEntries.insert( it, TextureEntry( searchName.name, mapType,
                                                            mTextureArrays[mapType].size(), 0 ) );

                    textureArray.entries.push_back( texName );
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
                retVal.texture  = mBlankTexture;
                retVal.xIdx     = 0;
                retVal.yIdx     = 0;
                retVal.divisor  = 1;
            }
        }

        return retVal;
    }
}
