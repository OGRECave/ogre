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

#include "Compositor/OgreCompositorNodeDef.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    void CompositorNodeDef::getTextureSource( size_t outputChannel, size_t &index,
                                                TextureSource &textureSource ) const
    {
        decodeTexSource( mOutChannelMapping[outputChannel], index, textureSource );
        assert( textureSource != TEXTURE_GLOBAL && "Can't use global textures in the output channel!" );
    }
    //-----------------------------------------------------------------------------------
    CompositorTargetDef* CompositorNodeDef::addTargetPass( const String &renderTargetName,
                                                            uint32 rtIndex )
    {
        assert( mTargetPasses.size() < mTargetPasses.capacity() &&
                "setNumTargetPass called improperly!" );

        if( renderTargetName.find( "global_" ) == 0 )
            addTextureSourceName( renderTargetName, 0, TEXTURE_GLOBAL );

        mTargetPasses.push_back( CompositorTargetDef( renderTargetName, rtIndex, this ) );
        return &mTargetPasses.back();
    }
    //-----------------------------------------------------------------------------------
    void CompositorNodeDef::mapOutputChannel( size_t outChannel, IdString textureName )
    {
        size_t index;
        TextureSource textureSource;
        getTextureSource( textureName, index, textureSource );

        mOutChannelMapping.resize( outChannel+1 );

        if( textureSource != TEXTURE_GLOBAL )
        {
            mOutChannelMapping[outChannel] = encodeTexSource( (uint32)index, textureSource );
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Can't use global textures as an output channel!. Node: '" +
                        mName.getFriendlyText() + "'", "CompositorNodeDef::mapOutputChannel" );
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNodeDef::removeTexture( IdString name )
    {
        size_t index;
        TextureSource textureSource;
        getTextureSource( name, index, textureSource );

        assert( mDefaultLocalTextureSource == TEXTURE_LOCAL );

        //Update the references from the output channels to our local textures
        if( textureSource == mDefaultLocalTextureSource )
        {
            ChannelMappings::iterator itor = mOutChannelMapping.begin();
            ChannelMappings::iterator end  = mOutChannelMapping.end();

            while( itor != end )
            {
                size_t otherIndex;
                decodeTexSource( *itor, otherIndex, textureSource );

                if( textureSource == mDefaultLocalTextureSource )
                {
                    if( otherIndex > index )
                    {
                        *itor = encodeTexSource( otherIndex - 1, textureSource );
                    }
                    else
                    {
                        //Purposedly cause a crash if left unfilled. This
                        //entry will only be removed if it's the last one.
                        *itor = encodeTexSource( 0x3FFFFFFF, textureSource );
                    }
                }

                ++itor;
            }

            //Mappings in the last channels that are no longer valid can be removed
            bool stopIterating = false;
            size_t mappingsToRemove = 0;
            ChannelMappings::const_reverse_iterator ritor = mOutChannelMapping.rbegin();
            ChannelMappings::const_reverse_iterator rend  = mOutChannelMapping.rend();

            while( ritor != rend && !stopIterating )
            {
                size_t otherIndex;
                decodeTexSource( *ritor, otherIndex, textureSource );

                stopIterating = true;

                if( otherIndex == 0x3FFFFFFF && textureSource == mDefaultLocalTextureSource )
                {
                    stopIterating = false;
                    ++mappingsToRemove;
                }

                ++ritor;
            }

            mOutChannelMapping.erase( mOutChannelMapping.begin() +
                                        (mOutChannelMapping.size() - mappingsToRemove),
                                      mOutChannelMapping.end() );
        }

        TextureDefinitionBase::removeTexture( name );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNodeDef::mapOutputBufferChannel( size_t outChannel, IdString bufferName )
    {
        IdStringVec::const_iterator inputIt = std::find( mInputBuffers.begin(),
                                                         mInputBuffers.end(), bufferName );

        if( inputIt == mInputBuffers.end() )
        {
            BufferDefinitionVec::const_iterator itor = mLocalBufferDefs.begin();
            BufferDefinitionVec::const_iterator end  = mLocalBufferDefs.end();

            while( itor != end && itor->getName() != bufferName )
                ++itor;

            if( itor == mLocalBufferDefs.end() )
            {
                OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Trying to map buffer '" +
                             bufferName.getFriendlyText() + "' to input channel #" +
                             StringConverter::toString( outChannel ) +
                             " but no buffer with such name exists.",
                             "CompositorNodeDef::mapOutputBufferChannel" );
            }
        }

        mOutBufferChannelMapping.resize( outChannel+1u );

        mOutBufferChannelMapping[outChannel] = bufferName;
    }
    //-----------------------------------------------------------------------------------
    size_t CompositorNodeDef::getPassNumber( const CompositorPassDef *passDef ) const
    {
        size_t passCount = 0;
        CompositorTargetDefVec::const_iterator itor = mTargetPasses.begin();
        CompositorTargetDefVec::const_iterator end  = mTargetPasses.end();

        while( itor != end )
        {
            CompositorPassDefVec::const_iterator itPassDef = itor->getCompositorPasses().begin();
            CompositorPassDefVec::const_iterator enPassDef = itor->getCompositorPasses().end();

            while( itPassDef != enPassDef )
            {
                if( passDef == *itPassDef )
                    return passCount;

                ++passCount;
                ++itPassDef;
            }
            ++itor;
        }

        return -1;
    }
}
