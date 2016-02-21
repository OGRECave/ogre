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

#include "OgreHlmsComputeJob.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsCompute.h"

#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreUavBufferPacked.h"

#include "OgreTexture.h"
#include "OgreLwString.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------------
    HlmsComputeJob::HlmsComputeJob( IdString name, Hlms *creator,
                                    const String &sourceFilename,
                                    const StringVector &includedPieceFiles ) :
        mCreator( creator ),
        mName( name ),
        mSourceFilename( sourceFilename ),
        mIncludedPieceFiles( includedPieceFiles ),
        mInformHlmsOfTextureData( false ),
        mMaxTexUnitReached( 0 ),
        mMaxUavUnitReached( 0 ),
        mPsoCacheHash( -1 )
    {
        memset( mThreadsPerGroup, 0, sizeof( mThreadsPerGroup ) );
        memset( mNumThreadGroups, 0, sizeof( mNumThreadGroups ) );
    }
    //-----------------------------------------------------------------------------------
    HlmsComputeJob::~HlmsComputeJob()
    {
        //If you get a crash inside this destructor (as part of the callstack, the
        //actual crash can end up somewhere on Resource::unload), then you're leaking
        //a MaterialPtr outside of Ogre.
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::updateAutoProperties( const TextureSlotVec &textureSlots,
                                               uint8 &outMaxTexUnitReached,
                                               const char *propTexture,
                                               const IdString &propNumTextureSlots,
                                               const IdString &propMaxTextureSlot )
    {
        char tmpData[64];
        LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

        propName = propTexture; //ComputeProperty::Texture or ComputeProperty::Uav
        const size_t texturePropSize = propName.size();

        uint8 maxTexUnitReached = outMaxTexUnitReached;

        for( uint8 i=0; i<maxTexUnitReached; ++i )
        {
            propName.resize( texturePropSize );
            propName.a( i );                        //texture0 or uav0

            propName.a( "_width" );                 //texture0_width
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_height" );                //texture0_height
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_depth" );                 //texture0_depth
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_mipmaps" );               //texture0_mipmaps
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_msaa" );                  //texture0_msaa
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_msaa_samples" );          //texture0_msaa_samples
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_1d" );                 //texture0_is_1d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_2d" );                 //texture0_is_2d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_3d" );                 //texture0_is_3d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_cubemap" );            //texture0_is_cubemap
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_2d_array" );           //texture0_is_2d_array
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );

            propName.a( "_is_buffer" );             //texture0_is_buffer
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize + 1u );
        }

        maxTexUnitReached = 0;

        if( mInformHlmsOfTextureData )
        {
            setProperty( propNumTextureSlots, textureSlots.size() );

            TextureSlotVec::const_iterator itor = textureSlots.begin();
            TextureSlotVec::const_iterator end  = textureSlots.end();

            while( itor != end )
            {
                propName.resize( texturePropSize );
                propName.a( itor->slotIdx );            //texture0 or uav0
                setProperty( propName.c_str(), 1 );

                maxTexUnitReached = std::max( itor->slotIdx, maxTexUnitReached );

                if( !itor->texture.isNull() )
                {
                    const TexturePtr &texture = itor->texture;

                    propName.a( "_width" );                 //texture0_width
                    setProperty( propName.c_str(), texture->getWidth() );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_height" );                //texture0_height
                    setProperty( propName.c_str(), texture->getHeight() );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_depth" );                 //texture0_depth
                    setProperty( propName.c_str(), std::max<uint32>( texture->getDepth(),
                                                                     texture->getNumFaces() ) );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_mipmaps" );               //texture0_mipmaps
                    setProperty( propName.c_str(), texture->getNumMipmaps() + 1 );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_msaa" );                  //texture0_msaa
                    setProperty( propName.c_str(), texture->getFSAA() > 1 ? 1 : 0 );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_msaa_samples" );          //texture0_msaa_samples
                    setProperty( propName.c_str(), texture->getFSAA() );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_is_1d" );                 //texture0_is_1d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_1D );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_is_2d" );                 //texture0_is_2d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_2D );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_is_3d" );                 //texture0_is_3d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_3D );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_is_cubemap" );            //texture0_is_cubemap
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_CUBE_MAP );
                    propName.resize( texturePropSize + 1u );

                    propName.a( "_is_2d_array" );           //texture0_is_2d_array
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_2D_ARRAY );
                    propName.resize( texturePropSize + 1u );
                }
                else
                {
                    propName.a( "_is_buffer" );             //texture0_is_buffer
                    setProperty( propName.c_str(), 1 );
                    propName.resize( texturePropSize + 1u );
                }

                ++itor;
            }

            ++maxTexUnitReached;
            setProperty( propMaxTextureSlot, maxTexUnitReached );
        }

        outMaxTexUnitReached = maxTexUnitReached;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::_updateAutoProperties(void)
    {
        setProperty( ComputeProperty::ThreadsPerGroupX, mThreadsPerGroup[0] );
        setProperty( ComputeProperty::ThreadsPerGroupY, mThreadsPerGroup[1] );
        setProperty( ComputeProperty::ThreadsPerGroupZ, mThreadsPerGroup[2] );
        setProperty( ComputeProperty::NumThreadGroupsX, mNumThreadGroups[0] );
        setProperty( ComputeProperty::NumThreadGroupsY, mNumThreadGroups[1] );
        setProperty( ComputeProperty::NumThreadGroupsZ, mNumThreadGroups[2] );

        removeProperty( ComputeProperty::NumTextureSlots );
        removeProperty( ComputeProperty::MaxTextureSlot );
        removeProperty( ComputeProperty::NumUavSlots );
        removeProperty( ComputeProperty::MaxUavSlot );

        updateAutoProperties( mTextureSlots, mMaxTexUnitReached,
                              ComputeProperty::Texture,
                              ComputeProperty::NumTextureSlots,
                              ComputeProperty::MaxTextureSlot );

        updateAutoProperties( mUavSlots, mMaxUavUnitReached,
                              ComputeProperty::Uav,
                              ComputeProperty::NumUavSlots,
                              ComputeProperty::MaxUavSlot );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setInformHlmsOfTextureData( bool bInformHlms )
    {
        mInformHlmsOfTextureData = bInformHlms;
        mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setThreadsPerGroup( uint32 threadsPerGroupX, uint32 threadsPerGroupY, uint32 threadsPerGroupZ )
    {
        if( mThreadsPerGroup[0] != threadsPerGroupX ||
            mThreadsPerGroup[1] != threadsPerGroupY ||
            mThreadsPerGroup[2] != threadsPerGroupZ )
        {
            mThreadsPerGroup[0] = threadsPerGroupX;
            mThreadsPerGroup[1] = threadsPerGroupY;
            mThreadsPerGroup[2] = threadsPerGroupZ;
            mPsoCacheHash = -1;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setNumThreadGroups( uint32 numThreadGroupsX, uint32 numThreadGroupsY, uint32 numThreadGroupsZ )
    {
        if( mNumThreadGroups[0] != numThreadGroupsX ||
            mNumThreadGroups[1] != numThreadGroupsY ||
            mNumThreadGroups[2] != numThreadGroupsZ )
        {
            mNumThreadGroups[0] = numThreadGroupsX;
            mNumThreadGroups[1] = numThreadGroupsY;
            mNumThreadGroups[2] = numThreadGroupsZ;
            mPsoCacheHash = -1;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setProperty( IdString key, int32 value )
    {
        HlmsProperty p( key, value );
        HlmsPropertyVec::iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                         p, OrderPropertyByIdString );
        if( it == mSetProperties.end() || it->keyName != p.keyName )
            mSetProperties.insert( it, p );
        else
            *it = p;

        mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    int32 HlmsComputeJob::getProperty( IdString key, int32 defaultVal ) const
    {
        HlmsProperty p( key, 0 );
        HlmsPropertyVec::const_iterator it = std::lower_bound( mSetProperties.begin(),
                                                               mSetProperties.end(),
                                                               p, OrderPropertyByIdString );
        if( it != mSetProperties.end() && it->keyName == p.keyName )
            defaultVal = it->value;

        return defaultVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::removeProperty( IdString key )
    {
        HlmsProperty p( key, 0 );
        HlmsPropertyVec::iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                         p, OrderPropertyByIdString );
        if( it == mSetProperties.end() || it->keyName != p.keyName )
            mSetProperties.erase( it );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setConstBuffer( uint8 slotIdx, ConstBufferPacked *constBuffer )
    {
        ConstBufferSlotVec::iterator itor = std::lower_bound( mConstBuffers.begin(),
                                                              mConstBuffers.end(), slotIdx,
                                                              ConstBufferSlot() );

        if( !constBuffer )
        {
            if( itor != mConstBuffers.end() && itor->slotIdx == slotIdx )
                mConstBuffers.erase( itor );
        }
        else
        {
            if( itor == mConstBuffers.end() || itor->slotIdx != slotIdx )
                itor = mConstBuffers.insert( itor, ConstBufferSlot() );

            itor->slotIdx = slotIdx;
            itor->buffer = constBuffer;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setBuffer( uint8 slotIdx, BufferPacked *buffer,
                                    size_t offset, size_t sizeBytes,
                                    ResourceAccess::ResourceAccess access,
                                    TextureSlotVec &container )
    {
        TextureSlotVec::iterator itor = std::lower_bound( container.begin(),
                                                          container.end(), slotIdx,
                                                          TextureSlot() );

        if( !buffer )
        {
            if( itor != container.end() && itor->slotIdx == slotIdx )
            {
                container.erase( itor );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }
        }
        else
        {
            if( itor == container.end() || itor->slotIdx != slotIdx )
            {
                itor = container.insert( itor, TextureSlot() );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }

            itor->slotIdx   = slotIdx;
            itor->buffer    = buffer;
            itor->offset    = offset;
            itor->sizeBytes = sizeBytes;
            itor->access    = access;

            if( mInformHlmsOfTextureData && !itor->texture.isNull() )
                mPsoCacheHash = -1;

            itor->texture.setNull();

            HlmsManager *hlmsManager = mCreator->getHlmsManager();
            hlmsManager->destroySamplerblock( itor->samplerblock );
            itor->samplerblock = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setTexBuffer( uint8 slotIdx, TexBufferPacked *texBuffer,
                                       size_t offset, size_t sizeBytes )
    {
        setBuffer( slotIdx, texBuffer, offset, sizeBytes, ResourceAccess::Undefined, mTextureSlots );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setTexture( uint8 slotIdx, TexturePtr &texture,
                                     const HlmsSamplerblock &refParams )
    {
        TextureSlotVec::iterator itor = std::lower_bound( mTextureSlots.begin(),
                                                          mTextureSlots.end(), slotIdx,
                                                          TextureSlot() );

        if( texture.isNull() )
        {
            if( itor != mTextureSlots.end() && itor->slotIdx == slotIdx )
            {
                if( itor->samplerblock )
                {
                    HlmsManager *hlmsManager = mCreator->getHlmsManager();
                    hlmsManager->destroySamplerblock( itor->samplerblock );
                }

                mTextureSlots.erase( itor );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }
        }
        else
        {
            if( itor == mTextureSlots.end() || itor->slotIdx != slotIdx )
            {
                itor = mTextureSlots.insert( itor, TextureSlot() );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }

            itor->slotIdx = slotIdx;
            itor->buffer = 0;

            if( mInformHlmsOfTextureData && itor->texture != texture &&
                (itor->texture->getWidth() != texture->getWidth() ||
                itor->texture->getHeight() != texture->getHeight() ||
                itor->texture->getDepth() != texture->getDepth() ||
                itor->texture->getNumFaces() != texture->getNumFaces() ||
                itor->texture->getNumMipmaps() != texture->getNumMipmaps() ||
                itor->texture->getFSAA() != texture->getFSAA() ||
                itor->texture->getTextureType() != texture->getTextureType()) )
            {
                mPsoCacheHash = -1;
            }

            itor->texture = texture;

            HlmsManager *hlmsManager = mCreator->getHlmsManager();

            const HlmsSamplerblock *oldSamplerblock = itor->samplerblock;
            itor->samplerblock = hlmsManager->getSamplerblock( refParams );

            if( oldSamplerblock )
                hlmsManager->destroySamplerblock( oldSamplerblock );

            itor->mrtIndex      = 0;
            itor->access        = ResourceAccess::Undefined;
            itor->mipmapLevel   = 0;
            itor->pixelFormat   = texture->getFormat();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setUavBuffer( uint8 slotIdx, UavBufferPacked *uavBuffer,
                                       ResourceAccess::ResourceAccess access,
                                       size_t offset, size_t sizeBytes )
    {
        setBuffer( slotIdx, uavBuffer, offset, sizeBytes, access, mUavSlots );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setUavTexture( uint32 slotIdx, TexturePtr &texture, uint32 mrtIndex,
                                        ResourceAccess::ResourceAccess access, int32 mipmapLevel,
                                        PixelFormat pixelFormat )
    {
        TextureSlotVec::iterator itor = std::lower_bound( mUavSlots.begin(),
                                                          mUavSlots.end(), slotIdx,
                                                          TextureSlot() );

        if( texture.isNull() )
        {
            if( itor != mUavSlots.end() && itor->slotIdx == slotIdx )
            {
                mUavSlots.erase( itor );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }
        }
        else
        {
            if( itor == mUavSlots.end() || itor->slotIdx != slotIdx )
            {
                itor = mUavSlots.insert( itor, TextureSlot() );
                if( mInformHlmsOfTextureData )
                    mPsoCacheHash = -1;
            }

            itor->slotIdx = slotIdx;
            itor->buffer = 0;

            if( mInformHlmsOfTextureData && itor->texture != texture &&
                (itor->texture->getWidth() != texture->getWidth() ||
                itor->texture->getHeight() != texture->getHeight() ||
                itor->texture->getDepth() != texture->getDepth() ||
                itor->texture->getNumFaces() != texture->getNumFaces() ||
                itor->texture->getNumMipmaps() != texture->getNumMipmaps() ||
                itor->texture->getFSAA() != texture->getFSAA() ||
                itor->texture->getTextureType() != texture->getTextureType()) )
            {
                mPsoCacheHash = -1;
            }

            itor->texture       = texture;
            itor->mrtIndex      = mrtIndex;
            itor->access        = access;
            itor->mipmapLevel   = mipmapLevel;
            itor->pixelFormat   = pixelFormat;
        }
    }
}
