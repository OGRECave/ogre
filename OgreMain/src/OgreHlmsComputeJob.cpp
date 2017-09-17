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

#include "OgreRenderSystem.h"

#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreUavBufferPacked.h"

#include "OgreTexture.h"
#include "OgreLwString.h"

#include "OgreLogManager.h"

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
        mThreadGroupsBasedOnTexture( ThreadGroupsBasedOnNothing ),
        mThreadGroupsBasedOnTexSlot( 0 ),
        mInformHlmsOfTextureData( false ),
        mMaxTexUnitReached( 0 ),
        mMaxUavUnitReached( 0 ),
        mPsoCacheHash( -1 )
    {
        memset( mThreadsPerGroup, 0, sizeof( mThreadsPerGroup ) );
        memset( mNumThreadGroups, 0, sizeof( mNumThreadGroups ) );

        mThreadGroupsBasedDivisor[0] = 1;
        mThreadGroupsBasedDivisor[1] = 1;
        mThreadGroupsBasedDivisor[2] = 1;
    }
    //-----------------------------------------------------------------------------------
    HlmsComputeJob::~HlmsComputeJob()
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        TextureSlotVec::iterator itor = mTextureSlots.begin();
        TextureSlotVec::iterator end  = mTextureSlots.end();

        while( itor != end )
        {
            if( itor->samplerblock )
            {
                hlmsManager->destroySamplerblock( itor->samplerblock );
                itor->samplerblock = 0;
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::updateAutoProperties( const TextureSlotVec &textureSlots,
                                               uint8 &outMaxTexUnitReached,
                                               const char *propTexture,
                                               const IdString &propNumTextureSlots,
                                               const IdString &propMaxTextureSlot )
    {
        assert( propTexture == ComputeProperty::Texture || propTexture == ComputeProperty::Uav );

        char tmpData[64];
        LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

        propName = propTexture; //It's either ComputeProperty::Texture or ComputeProperty::Uav
        const size_t texturePropNameSize = propName.size();

        uint8 maxTexUnitReached = outMaxTexUnitReached;

        //Remove everything from any previous run.
        for( uint8 i=0; i<maxTexUnitReached; ++i )
        {
            propName.resize( texturePropNameSize );
            propName.a( i );                        //texture0 or uav0

            const size_t texturePropSize = propName.size();

            propName.a( "_width" );                 //texture0_width
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_height" );                //texture0_height
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_depth" );                 //texture0_depth
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_mipmaps" );               //texture0_mipmaps
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_msaa" );                  //texture0_msaa
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_msaa_samples" );          //texture0_msaa_samples
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_1d" );                 //texture0_is_1d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_2d" );                 //texture0_is_2d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_3d" );                 //texture0_is_3d
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_cubemap" );            //texture0_is_cubemap
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_2d_array" );           //texture0_is_2d_array
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_is_buffer" );             //texture0_is_buffer
            removeProperty( propName.c_str() );
            propName.resize( texturePropSize );

            propName.a( "_pf_type" );               //uav0_pf_type
            removePiece( propName.c_str() );
            propName.resize( texturePropSize );

            //Note we're comparing pointers, not string comparison!
            if( propTexture == ComputeProperty::Uav )
            {
                propName.a( "_width_with_lod" );    //uav0_width_with_lod
                removePiece( propName.c_str() );
                propName.resize( texturePropSize );

                propName.a( "_height_with_lod" );   //uav0_height_with_lod
                removePiece( propName.c_str() );
                propName.resize( texturePropSize );
            }
        }

        //Set the new value.
        maxTexUnitReached = static_cast<uint8>( textureSlots.size() );

        if( mInformHlmsOfTextureData )
        {
            setProperty( propNumTextureSlots, static_cast<int32>( textureSlots.size() ) );

            RenderSystem *renderSystem = mCreator->getRenderSystem();
            const PixelFormatToShaderType *toShaderType = renderSystem->getPixelFormatToShaderType();

            TextureSlotVec::const_iterator begin= textureSlots.begin();
            TextureSlotVec::const_iterator itor = textureSlots.begin();
            TextureSlotVec::const_iterator end  = textureSlots.end();

            while( itor != end )
            {
                const size_t slotIdx = itor - begin;
                propName.resize( texturePropNameSize );
                propName.a( static_cast<uint32>(slotIdx) ); //texture0 or uav0
                const size_t texturePropSize = propName.size();
                setProperty( propName.c_str(), 1 );

                if( !itor->texture.isNull() )
                {
                    const TexturePtr &texture = itor->texture;

                    propName.a( "_width" );                 //texture0_width
                    setProperty( propName.c_str(), texture->getWidth() );
                    propName.resize( texturePropSize );

                    propName.a( "_height" );                //texture0_height
                    setProperty( propName.c_str(), texture->getHeight() );
                    propName.resize( texturePropSize );

                    propName.a( "_depth" );                 //texture0_depth
                    setProperty( propName.c_str(), std::max<uint32>( texture->getDepth(),
                                                                     texture->getNumFaces() ) );
                    propName.resize( texturePropSize );

                    propName.a( "_mipmaps" );               //texture0_mipmaps
                    setProperty( propName.c_str(), texture->getNumMipmaps() + 1 );
                    propName.resize( texturePropSize );

                    propName.a( "_msaa" );                  //texture0_msaa
                    setProperty( propName.c_str(), texture->getFSAA() > 1 ? 1 : 0 );
                    propName.resize( texturePropSize );

                    propName.a( "_msaa_samples" );          //texture0_msaa_samples
                    setProperty( propName.c_str(), texture->getFSAA() );
                    propName.resize( texturePropSize );

                    propName.a( "_is_1d" );                 //texture0_is_1d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_1D );
                    propName.resize( texturePropSize );

                    propName.a( "_is_2d" );                 //texture0_is_2d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_2D );
                    propName.resize( texturePropSize );

                    propName.a( "_is_3d" );                 //texture0_is_3d
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_3D );
                    propName.resize( texturePropSize );

                    propName.a( "_is_cubemap" );            //texture0_is_cubemap
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_CUBE_MAP );
                    propName.resize( texturePropSize );

                    propName.a( "_is_2d_array" );           //texture0_is_2d_array
                    setProperty( propName.c_str(), texture->getTextureType() == TEX_TYPE_2D_ARRAY );
                    propName.resize( texturePropSize );

                    propName.a( "_pf_type" );           //uav0_pf_type
                    const char *typeName = toShaderType->getPixelFormatType( texture->getFormat() );
                    if( typeName )
                        setPiece( propName.c_str(), typeName );
                    propName.resize( texturePropSize );

                    //Note we're comparing pointers, not string comparison!
                    if( propTexture == ComputeProperty::Uav )
                    {
                        uint32 mipLevel = std::min<uint32>( itor->mipmapLevel,
                                                            texture->getNumMipmaps() );

                        propName.a( "_width_with_lod" );    //uav0_width_with_lod
                        setProperty( propName.c_str(), std::max( texture->getWidth() >>
                                                                 (uint32)mipLevel, 1u ) );
                        propName.resize( texturePropSize );

                        propName.a( "_height_with_lod" );   //uav0_height_with_lod
                        setProperty( propName.c_str(), std::max( texture->getHeight() >>
                                                                 (uint32)mipLevel, 1u ) );
                        propName.resize( texturePropSize );
                    }
                }
                else if( itor->buffer )
                {
                    propName.a( "_is_buffer" );             //texture0_is_buffer
                    setProperty( propName.c_str(), 1 );
                    propName.resize( texturePropSize );
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
    void HlmsComputeJob::setNumThreadGroupsBasedOn( ThreadGroupsBasedOn source, uint8 texSlot,
                                                    uint8 divisorX, uint8 divisorY, uint8 divisorZ )
    {
        mThreadGroupsBasedOnTexture = source;
        mThreadGroupsBasedOnTexSlot = texSlot;

        mThreadGroupsBasedDivisor[0] = divisorX;
        mThreadGroupsBasedDivisor[1] = divisorY;
        mThreadGroupsBasedDivisor[2] = divisorZ;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::_calculateNumThreadGroupsBasedOnSetting()
    {
        bool hasChanged = false;

        if( mThreadGroupsBasedOnTexture != ThreadGroupsBasedOnNothing )
        {
            const TextureSlotVec &texSlots = mThreadGroupsBasedOnTexture == ThreadGroupsBasedOnTexture ?
                        mTextureSlots : mUavSlots;

            if( mThreadGroupsBasedOnTexSlot < texSlots.size() &&
                !texSlots[mThreadGroupsBasedOnTexSlot].texture.isNull() )
            {
                const TexturePtr &tex = texSlots[mThreadGroupsBasedOnTexSlot].texture;

                uint32 resolution[3];
                resolution[0] = tex->getWidth();
                resolution[1] = tex->getHeight();
                resolution[2] = tex->getDepth();

                if( tex->getTextureType() == TEX_TYPE_CUBE_MAP )
                    resolution[2] = tex->getNumFaces();

                for( int i=0; i<3; ++i )
                {
                    resolution[i] = (resolution[i] + mThreadGroupsBasedDivisor[i] - 1u) /
                                    mThreadGroupsBasedDivisor[i];

                    uint32 numThreadGroups = (resolution[i] + mThreadsPerGroup[i] - 1u) /
                                             mThreadsPerGroup[i];
                    if( mNumThreadGroups[i] != numThreadGroups )
                    {
                        mNumThreadGroups[i] = numThreadGroups;
                        hasChanged = true;
                    }
                }
            }
            else
            {
                LogManager::getSingleton().logMessage(
                            "WARNING: No texture/uav bound to compute job '" + mName.getFriendlyText() +
                            "' at slot " + StringConverter::toString(mThreadGroupsBasedOnTexSlot) +
                            " while calculating number of thread groups based on texture");
            }
        }

        if( hasChanged )
            mPsoCacheHash = -1;
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
        if( it != mSetProperties.end() && it->keyName == p.keyName )
            mSetProperties.erase( it );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setPiece( IdString pieceName, const String &pieceContent )
    {
        mPieces[pieceName] = pieceContent;

        int32 contentHash = 0;
        MurmurHash3_x86_32( pieceContent.c_str(), pieceContent.size(), IdString::Seed, &contentHash );
        setProperty( pieceName, contentHash );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::removePiece( IdString pieceName )
    {
        PiecesMap::iterator it = mPieces.find( pieceName );
        if( it != mPieces.end() )
        {
            removeProperty( pieceName );
            mPieces.erase( it );
        }
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
    void HlmsComputeJob::createShaderParams( IdString key )
    {
        if( mShaderParams.find( key ) == mShaderParams.end() )
            mShaderParams[key] = ShaderParams();
    }
    //-----------------------------------------------------------------------------------
    ShaderParams& HlmsComputeJob::getShaderParams( IdString key )
    {
        ShaderParams *retVal = 0;

        map<IdString, ShaderParams>::type::iterator itor = mShaderParams.find( key );
        if( itor == mShaderParams.end() )
        {
            createShaderParams( key );
            itor = mShaderParams.find( key );
        }

        retVal = &itor->second;

        return *retVal;
    }
    //-----------------------------------------------------------------------------------
    ShaderParams* HlmsComputeJob::_getShaderParams( IdString key )
    {
        ShaderParams *retVal = 0;

        map<IdString, ShaderParams>::type::iterator itor = mShaderParams.find( key );
        if( itor != mShaderParams.end() )
            retVal = &itor->second;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setBuffer( uint8 slotIdx, BufferPacked *buffer,
                                    size_t offset, size_t sizeBytes,
                                    ResourceAccess::ResourceAccess access,
                                    TextureSlotVec &container )
    {
        assert( slotIdx < container.size() );

        TextureSlot &texSlot = container[slotIdx];

        texSlot.buffer      = buffer;
        texSlot.offset      = offset;
        texSlot.sizeBytes   = sizeBytes;
        texSlot.access      = access;

        if( mInformHlmsOfTextureData && !texSlot.texture.isNull() )
            mPsoCacheHash = -1;

        texSlot.texture.setNull();

        if( texSlot.samplerblock )
        {
            HlmsManager *hlmsManager = mCreator->getHlmsManager();
            hlmsManager->destroySamplerblock( texSlot.samplerblock );
            texSlot.samplerblock = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setNumTexUnits( uint8 numSlots )
    {
        mTextureSlots.resize( numSlots );
        if( mInformHlmsOfTextureData )
            mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::removeTexUnit( uint8 slotIdx )
    {
        mTextureSlots.erase( mTextureSlots.begin() + slotIdx );
        if( mInformHlmsOfTextureData )
            mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    const TexturePtr& HlmsComputeJob::getTexture( uint8 slotIdx ) const
    {
        return mTextureSlots[slotIdx].texture;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setNumUavUnits( uint8 numSlots )
    {
        mUavSlots.resize( numSlots );
        if( mInformHlmsOfTextureData )
            mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::removeUavUnit( uint8 slotIdx )
    {
        mUavSlots.erase( mUavSlots.begin() + slotIdx );
        if( mInformHlmsOfTextureData )
            mPsoCacheHash = -1;
    }
    //-----------------------------------------------------------------------------------
    const TexturePtr& HlmsComputeJob::getUavTexture( uint8 slotIdx ) const
    {
        return mUavSlots[slotIdx].texture;
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* HlmsComputeJob::getUavBuffer( uint8 slotIdx ) const
    {
        return static_cast<UavBufferPacked*>( mUavSlots[slotIdx].buffer );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setTexBuffer( uint8 slotIdx, TexBufferPacked *texBuffer,
                                       size_t offset, size_t sizeBytes )
    {
        setBuffer( slotIdx, texBuffer, offset, sizeBytes, ResourceAccess::Undefined, mTextureSlots );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setTexture( uint8 slotIdx, TexturePtr &texture,
                                     const HlmsSamplerblock *refParams )
    {
        assert( slotIdx < mTextureSlots.size() );

        TextureSlot &texSlot = mTextureSlots[slotIdx];
        texSlot.buffer = 0;

        if( mInformHlmsOfTextureData && texSlot.texture != texture &&
            (texture.isNull() || texSlot.texture.isNull() ||
            texSlot.texture->getWidth() != texture->getWidth() ||
            texSlot.texture->getHeight() != texture->getHeight() ||
            texSlot.texture->getDepth() != texture->getDepth() ||
            texSlot.texture->getNumFaces() != texture->getNumFaces() ||
            texSlot.texture->getNumMipmaps() != texture->getNumMipmaps() ||
            texSlot.texture->getFSAA() != texture->getFSAA() ||
            texSlot.texture->getTextureType() != texture->getTextureType()) )
        {
            mPsoCacheHash = -1;
        }

        texSlot.texture = texture;

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        if( refParams || !texSlot.samplerblock )
        {
            const HlmsSamplerblock *oldSamplerblock = texSlot.samplerblock;
            if( refParams )
                texSlot.samplerblock = hlmsManager->getSamplerblock( *refParams );
            else
                texSlot.samplerblock = hlmsManager->getSamplerblock( HlmsSamplerblock() );

            if( oldSamplerblock )
                hlmsManager->destroySamplerblock( oldSamplerblock );
        }

        texSlot.textureArrayIndex   = 0;
        texSlot.access              = ResourceAccess::Undefined;
        texSlot.mipmapLevel         = 0;
        if( !texture.isNull() )
            texSlot.pixelFormat = texture->getFormat();
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::setSamplerblock( uint8 slotIdx, const HlmsSamplerblock &refParams )
    {
        assert( slotIdx < mTextureSlots.size() );

        TextureSlot &texSlot = mTextureSlots[slotIdx];
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsSamplerblock *oldSamplerblock = texSlot.samplerblock;
        texSlot.samplerblock = hlmsManager->getSamplerblock( refParams );

        if( oldSamplerblock )
            hlmsManager->destroySamplerblock( oldSamplerblock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::_setSamplerblock( uint8 slotIdx, const HlmsSamplerblock *refParams )
    {
        assert( slotIdx < mTextureSlots.size() );

        TextureSlot &texSlot = mTextureSlots[slotIdx];
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsSamplerblock *oldSamplerblock = texSlot.samplerblock;
        texSlot.samplerblock = refParams;

        if( oldSamplerblock )
            hlmsManager->destroySamplerblock( oldSamplerblock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::_setUavBuffer( uint8 slotIdx, UavBufferPacked *uavBuffer,
                                        ResourceAccess::ResourceAccess access,
                                        size_t offset, size_t sizeBytes )
    {
        setBuffer( slotIdx, uavBuffer, offset, sizeBytes, access, mUavSlots );
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::_setUavTexture( uint8 slotIdx, TexturePtr &texture, int32 textureArrayIndex,
                                         ResourceAccess::ResourceAccess access, int32 mipmapLevel,
                                         PixelFormat pixelFormat )
    {
        assert( slotIdx < mUavSlots.size() );

        TextureSlot &texSlot = mUavSlots[slotIdx];
        texSlot.buffer = 0;
        texSlot.samplerblock = 0;

        if( mInformHlmsOfTextureData && texSlot.texture != texture &&
            (texture.isNull() || texSlot.texture.isNull() ||
            texSlot.texture->getWidth() != texture->getWidth() ||
            texSlot.texture->getHeight() != texture->getHeight() ||
            texSlot.texture->getDepth() != texture->getDepth() ||
            texSlot.texture->getNumFaces() != texture->getNumFaces() ||
            texSlot.texture->getNumMipmaps() != texture->getNumMipmaps() ||
            texSlot.texture->getFSAA() != texture->getFSAA() ||
            texSlot.texture->getTextureType() != texture->getTextureType()) )
        {
            mPsoCacheHash = -1;
        }

        texSlot.texture             = texture;
        texSlot.textureArrayIndex   = textureArrayIndex;
        texSlot.access              = access;
        texSlot.mipmapLevel         = mipmapLevel;
        texSlot.pixelFormat         = pixelFormat;

        if( pixelFormat == PF_UNKNOWN && !texture.isNull() )
            texSlot.pixelFormat = texture->getFormat();
    }
    //-----------------------------------------------------------------------------------
    HlmsComputeJob* HlmsComputeJob::clone( const String &cloneName )
    {
        HlmsCompute *compute = static_cast<HlmsCompute*>( mCreator );
        HlmsComputeJob *newJob = compute->createComputeJob( cloneName, cloneName,
                                                            this->mSourceFilename,
                                                            this->mIncludedPieceFiles );

        this->cloneTo( newJob );

        return newJob;
    }
    //-----------------------------------------------------------------------------------
    void HlmsComputeJob::cloneTo( HlmsComputeJob *dstJob )
    {
        IdString originalName = dstJob->mName;
        *dstJob = *this;
        dstJob->mName = originalName;

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        TextureSlotVec::const_iterator itor = dstJob->mTextureSlots.begin();
        TextureSlotVec::const_iterator end  = dstJob->mTextureSlots.end();

        while( itor != end )
        {
            if( itor->samplerblock )
                hlmsManager->addReference( itor->samplerblock );
            ++itor;
        }
    }
}
