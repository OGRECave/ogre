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

#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClear.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassCompute.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassComputeDef.h"
#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopy.h"
#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h"
#include "Compositor/Pass/PassMipmap/OgreCompositorPassMipmap.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuad.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/Pass/PassStencil/OgreCompositorPassStencil.h"
#include "Compositor/Pass/PassUav/OgreCompositorPassUav.h"
#include "Compositor/Pass/PassUav/OgreCompositorPassUavDef.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/Pass/OgreCompositorPassProvider.h"
#include "Vao/OgreUavBufferPacked.h"

#include "OgreRenderSystem.h"
#include "OgreSceneManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"

#include "OgreLogManager.h"

namespace Ogre
{
    CompositorNode::CompositorNode( IdType id, IdString name, const CompositorNodeDef *definition,
                                    CompositorWorkspace *workspace, RenderSystem *renderSys,
                                    const RenderTarget *finalTarget ) :
            IdObject( id ),
            mName( name ),
            mEnabled( definition->mStartEnabled ),
            mNumConnectedInputs( 0 ),
            mNumConnectedBufferInputs( 0 ),
            mWorkspace( workspace ),
            mRenderSystem( renderSys ),
            mDefinition( definition )
    {
        mInTextures.resize( mDefinition->getNumInputChannels(), CompositorChannel() );
        mOutTextures.resize( mDefinition->mOutChannelMapping.size() );

        //Create local textures
        TextureDefinitionBase::createTextures( definition->mLocalTextureDefs, mLocalTextures,
                                                id, finalTarget, mRenderSystem );

        const CompositorNamedBufferVec &globalBuffers = workspace->getGlobalBuffers();

        //Create local buffers
        mBuffers.reserve( mDefinition->mLocalBufferDefs.size() + mDefinition->mInputBuffers.size() +
                          globalBuffers.size() );
        TextureDefinitionBase::createBuffers( definition->mLocalBufferDefs, mBuffers,
                                              finalTarget, renderSys );

        //Local textures will be routed now.
        routeOutputs();
    }
    //-----------------------------------------------------------------------------------
    CompositorNode::~CompositorNode()
    {
        //Don't leave dangling pointers
        disconnectOutput();

        {
            //Destroy all passes
            CompositorPassVec::const_iterator itor = mPasses.begin();
            CompositorPassVec::const_iterator end  = mPasses.end();
            while( itor != end )
                OGRE_DELETE *itor++;
        }

        //Destroy our local buffers
        TextureDefinitionBase::destroyBuffers( mDefinition->mLocalBufferDefs, mBuffers, mRenderSystem );

        //Destroy our local textures
        TextureDefinitionBase::destroyTextures( mLocalTextures, mRenderSystem );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::routeOutputs()
    {
        CompositorChannelVec::iterator itor = mOutTextures.begin();
        CompositorChannelVec::iterator end  = mOutTextures.end();
        CompositorChannelVec::iterator begin= mOutTextures.begin();

        while( itor != end )
        {
            size_t index;
            TextureDefinitionBase::TextureSource textureSource;
            mDefinition->getTextureSource( itor - begin, index, textureSource );

            assert( textureSource == TextureDefinitionBase::TEXTURE_LOCAL ||
                    textureSource == TextureDefinitionBase::TEXTURE_INPUT );

            if( textureSource == TextureDefinitionBase::TEXTURE_LOCAL )
                *itor = mLocalTextures[index];
            else
                *itor = mInTextures[index];
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::disconnectOutput()
    {
        CompositorNodeVec::const_iterator itor = mConnectedNodes.begin();
        CompositorNodeVec::const_iterator end  = mConnectedNodes.end();

        while( itor != end )
        {
            CompositorChannelVec::const_iterator texIt = mLocalTextures.begin();
            CompositorChannelVec::const_iterator texEn = mLocalTextures.end();

            while( texIt != texEn )
                (*itor)->notifyDestroyed( *texIt++ );

            CompositorNodeDef::BufferDefinitionVec::const_iterator bufIt =
                    mDefinition->mLocalBufferDefs.begin();
            CompositorNodeDef::BufferDefinitionVec::const_iterator bufEn =
                    mDefinition->mLocalBufferDefs.end();

            while( bufIt != bufEn )
            {
                UavBufferPacked *uavBuffer = this->getDefinedBufferNoThrow( bufIt->name );
                if( uavBuffer )
                    (*itor)->notifyDestroyed( uavBuffer );
                ++bufIt;
            }

            ++itor;
        }

        mConnectedNodes.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::populateGlobalBuffers(void)
    {
        //Makes global buffers visible to our passes.
        CompositorNamedBuffer cmp;
        const CompositorNamedBufferVec &globalBuffers = mWorkspace->getGlobalBuffers();
        CompositorNamedBufferVec::const_iterator itor = globalBuffers.begin();
        CompositorNamedBufferVec::const_iterator end  = globalBuffers.end();
        while( itor != end )
        {
            CompositorNamedBufferVec::iterator itBuf = std::lower_bound( mBuffers.begin(),
                                                                         mBuffers.end(),
                                                                         itor->name, cmp );
            if( itBuf != mBuffers.end() && itBuf->name == itor->name )
            {
                LogManager::getSingleton().logMessage(
                            "WARNING: Locally defined buffer '" + itBuf->name.getFriendlyText() +
                            "' in Node '" + mDefinition->mNameStr + "' occludes global texture"
                            " of the same name." );
            }
            else
            {
                mBuffers.insert( itBuf, 1, *itor );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::notifyRecreated( const CompositorChannel &oldChannel,
                                            const CompositorChannel &newChannel )
    {
        //Clear our inputs
        CompositorChannelVec::iterator texIt = mInTextures.begin();
        CompositorChannelVec::iterator texEn = mInTextures.end();

        //We can't early out, it's possible to assign the same output to two different
        //input channels (though it would work very unintuitively...)
        while( texIt != texEn )
        {
            if( *texIt == oldChannel )
                *texIt = newChannel;
            ++texIt;
        }

        //Clear our outputs
        bool bFoundOuts = false;
        texIt = mOutTextures.begin();
        texEn = mOutTextures.end();

        while( texIt != texEn )
        {
            if( *texIt == oldChannel )
            {
                bFoundOuts = true;
                *texIt = newChannel;
            }
            ++texIt;
        }

        if( bFoundOuts )
        {
            //Our attachees may have that texture too.
            CompositorNodeVec::const_iterator itor = mConnectedNodes.begin();
            CompositorNodeVec::const_iterator end  = mConnectedNodes.end();

            while( itor != end )
            {
                (*itor)->notifyRecreated( oldChannel, newChannel );
                ++itor;
            }
        }

        CompositorPassVec::const_iterator passIt = mPasses.begin();
        CompositorPassVec::const_iterator passEn = mPasses.end();
        while( passIt != passEn )
        {
            (*passIt)->notifyRecreated( oldChannel, newChannel );
            ++passIt;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::notifyRecreated( const UavBufferPacked *oldBuffer, UavBufferPacked *newBuffer )
    {
        //Clear our inputs
        CompositorNamedBufferVec::iterator bufIt = mBuffers.begin();
        CompositorNamedBufferVec::iterator bufEn = mBuffers.end();

        bool bFoundOuts = false;

        //We can't early out, it's possible to assign the same output to two different
        //input channels (though it would work very unintuitively...)
        while( bufIt != bufEn )
        {
            if( bufIt->buffer == oldBuffer )
            {
                bufIt->buffer = newBuffer;

                //Check if we'll need to clear our outputs
                IdStringVec::const_iterator itor = mDefinition->mOutBufferChannelMapping.begin();
                IdStringVec::const_iterator end  = mDefinition->mOutBufferChannelMapping.end();

                while( itor != end && !bFoundOuts )
                {
                    if( *itor == bufIt->name )
                        bFoundOuts = true;
                    ++itor;
                }
            }

            ++bufIt;
        }

        if( bFoundOuts )
        {
            //Our attachees may be using that buffer too.
            CompositorNodeVec::const_iterator itor = mConnectedNodes.begin();
            CompositorNodeVec::const_iterator end  = mConnectedNodes.end();

            while( itor != end )
            {
                (*itor)->notifyRecreated( oldBuffer, newBuffer );
                ++itor;
            }
        }

        CompositorPassVec::const_iterator passIt = mPasses.begin();
        CompositorPassVec::const_iterator passEn = mPasses.end();
        while( passIt != passEn )
        {
            (*passIt)->notifyRecreated( oldBuffer, newBuffer );
            ++passIt;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::notifyDestroyed( const CompositorChannel &channel )
    {
        //Clear our inputs
        CompositorChannelVec::iterator texIt = mInTextures.begin();
        CompositorChannelVec::iterator texEn = mInTextures.end();

        //We can't early out, it's possible to assign the same output to two different
        //input channels (though it would work very unintuitively...)
        while( texIt != texEn )
        {
            if( *texIt == channel )
            {
                *texIt = CompositorChannel();
                --mNumConnectedInputs;
            }
            ++texIt;
        }

        //Clear our outputs
        bool bFoundOuts = false;
        texIt = mOutTextures.begin();
        texEn = mOutTextures.end();

        while( texIt != texEn )
        {
            if( *texIt == channel )
            {
                bFoundOuts = true;
                *texIt = CompositorChannel();
                --mNumConnectedInputs;
            }
            ++texIt;
        }

        if( bFoundOuts )
        {
            //Our attachees may have that texture too.
            CompositorNodeVec::const_iterator itor = mConnectedNodes.begin();
            CompositorNodeVec::const_iterator end  = mConnectedNodes.end();

            while( itor != end )
            {
                (*itor)->notifyDestroyed( channel );
                ++itor;
            }
        }

        CompositorPassVec::const_iterator passIt = mPasses.begin();
        CompositorPassVec::const_iterator passEn = mPasses.end();
        while( passIt != passEn )
        {
            (*passIt)->notifyDestroyed( channel );
            ++passIt;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::notifyDestroyed( const UavBufferPacked *buffer )
    {
        //Clear our inputs
        CompositorNamedBufferVec::iterator bufIt = mBuffers.begin();
        CompositorNamedBufferVec::iterator bufEn = mBuffers.end();

        bool bFoundOuts = false;

        //We can't early out, it's possible to assign the same output to two different
        //input channels (though it would work very unintuitively...)
        while( bufIt != bufEn )
        {
            if( bufIt->buffer == buffer )
            {
                //Check if we'll need to clear our outputs
                IdStringVec::const_iterator itor = mDefinition->mOutBufferChannelMapping.begin();
                IdStringVec::const_iterator end  = mDefinition->mOutBufferChannelMapping.end();

                while( itor != end && !bFoundOuts )
                {
                    if( *itor == bufIt->name )
                        bFoundOuts = true;
                    ++itor;
                }

                //Remove this buffer.
                bufIt = mBuffers.erase( bufIt );
                bufEn = mBuffers.end();
                --mNumConnectedBufferInputs;
            }
            else
            {
                ++bufIt;
            }
        }

        if( bFoundOuts )
        {
            //Our attachees may be using that buffer too.
            CompositorNodeVec::const_iterator itor = mConnectedNodes.begin();
            CompositorNodeVec::const_iterator end  = mConnectedNodes.end();

            while( itor != end )
            {
                (*itor)->notifyDestroyed( buffer );
                ++itor;
            }
        }

        CompositorPassVec::const_iterator passIt = mPasses.begin();
        CompositorPassVec::const_iterator passEn = mPasses.end();
        while( passIt != passEn )
        {
            (*passIt)->notifyDestroyed( buffer );
            ++passIt;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_notifyCleared(void)
    {
        //Clear our inputs
        CompositorChannelVec::iterator texIt = mInTextures.begin();
        CompositorChannelVec::iterator texEn = mInTextures.end();

        while( texIt != texEn )
            *texIt++ = CompositorChannel();

        mNumConnectedInputs = 0;

        //Clear our inputs (buffers)
        CompositorNamedBuffer cmp;
        IdStringVec::const_iterator bufNameIt = mDefinition->mInputBuffers.begin();
        IdStringVec::const_iterator bufNameEn = mDefinition->mInputBuffers.end();

        while( bufNameIt != bufNameEn )
        {
            CompositorNamedBufferVec::iterator itBuf = std::lower_bound( mBuffers.begin(),
                                                                         mBuffers.end(),
                                                                         *bufNameIt, cmp );
            mBuffers.erase( itBuf );
            ++bufNameIt;
        }

        mNumConnectedBufferInputs = 0;

        //This call will clear only our outputs that come from input channels.
        routeOutputs();

        CompositorPassVec::const_iterator passIt = mPasses.begin();
        CompositorPassVec::const_iterator passEn = mPasses.end();
        while( passIt != passEn )
            OGRE_DELETE *passIt++;

        mPasses.clear();
        mConnectedNodes.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::setEnabled( bool bEnabled )
    {
        if( mEnabled != bEnabled )
        {
            mEnabled = bEnabled;
            mWorkspace->_notifyBarriersDirty();
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::connectBufferTo( size_t outChannelA, CompositorNode *nodeB, size_t inChannelB )
    {
        if( inChannelB >= nodeB->mDefinition->mInputBuffers.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no input channel #" +
                         StringConverter::toString( inChannelB ) + " for node " +
                         nodeB->mDefinition->mNameStr + " when trying to connect it "
                         "from " + this->mDefinition->mNameStr + " channel #" +
                         StringConverter::toString( outChannelA ),
                         "CompositorNode::connectBufferTo" );
        }
        if( outChannelA >= this->mDefinition->mOutBufferChannelMapping.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no output channel #" +
                         StringConverter::toString( outChannelA ) + " for node " +
                         this->mDefinition->mNameStr + " when trying to connect it "
                         "to " + nodeB->mDefinition->mNameStr + " channel #" +
                         StringConverter::toString( inChannelB ),
                         "CompositorNode::connectBufferTo" );
        }

        CompositorNamedBuffer cmp;

        const IdString &outBufferName = this->mDefinition->mOutBufferChannelMapping[outChannelA];
        CompositorNamedBufferVec::iterator outBuf = std::lower_bound( this->mBuffers.begin(),
                                                                      this->mBuffers.end(),
                                                                      outBufferName, cmp );

        //Nodes must be connected in the right order. We know for sure there is a buffer named
        //outBufferName in either input/local because we checked in mapOutputBufferChannel.
        assert( (outBuf != mBuffers.end() && outBuf->name == outBufferName) &&
                "Compositor node got connected in the wrong order!" );

        const IdString &inBufferName = nodeB->mDefinition->mInputBuffers[inChannelB];
        CompositorNamedBufferVec::iterator inBuf = std::lower_bound( nodeB->mBuffers.begin(),
                                                                     nodeB->mBuffers.end(),
                                                                     inBufferName, cmp );

        if( inBufferName == IdString() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Input buffer channels must not have gaps."
                         " Channel #" + StringConverter::toString( inChannelB ) + " from node '" +
                         nodeB->mDefinition->mNameStr + "' is not defined.",
                         "CompositorNode::connectBufferTo" );
        }

        if( inBuf != nodeB->mBuffers.end() && inBuf->name == inBufferName )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Buffer with name '" +
                         inBufferName.getFriendlyText() + "' was already defined in node " +
                         nodeB->mDefinition->mNameStr + " while trying to connect to its "
                         "channel #" + StringConverter::toString( inChannelB ) +
                         "from " + this->mDefinition->mNameStr + " channel #" +
                         StringConverter::toString( outChannelA ),
                         "CompositorNode::connectBufferTo" );
        }
        else
        {
            nodeB->mBuffers.insert( inBuf, 1, *outBuf );
            ++mNumConnectedBufferInputs;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::connectTo( size_t outChannelA, CompositorNode *nodeB, size_t inChannelB )
    {
        if( inChannelB >= nodeB->mInTextures.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no input channel #" +
                         StringConverter::toString( inChannelB ) + " for node " +
                         nodeB->mDefinition->mNameStr + " when trying to connect it "
                         "from " + this->mDefinition->mNameStr + " channel #" +
                         StringConverter::toString( outChannelA ),
                         "CompositorNode::connectTo" );
        }
        if( outChannelA >= this->mOutTextures.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no output channel #" +
                         StringConverter::toString( outChannelA ) + " for node " +
                         this->mDefinition->mNameStr + " when trying to connect it "
                         "to " + nodeB->mDefinition->mNameStr + " channel #" +
                         StringConverter::toString( inChannelB ),
                         "CompositorNode::connectTo" );
        }

        //Nodes must be connected in the right order (and after routeOutputs was called)
        //to avoid passing null pointers (which is probably not what we wanted)
        assert( this->mOutTextures[outChannelA].isValid() &&
                "Compositor node got connected in the wrong order!" );

        if( !nodeB->mInTextures[inChannelB].isValid() )
            ++nodeB->mNumConnectedInputs;
        nodeB->mInTextures[inChannelB] = this->mOutTextures[outChannelA];

        if( nodeB->mNumConnectedInputs >= nodeB->mInTextures.size() )
            nodeB->routeOutputs();

        this->mConnectedNodes.push_back( nodeB );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::connectExternalRT( const CompositorChannel &externalTexture, size_t inChannelA )
    {
        if( inChannelA >= mInTextures.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no input channel #" +
                         StringConverter::toString( inChannelA ) + " for node " +
                         mDefinition->mNameStr, "CompositorNode::connectFinalRT" );
        }

        if( !mInTextures[inChannelA].target )
            ++mNumConnectedInputs;
        mInTextures[inChannelA] = externalTexture;

        if( this->mNumConnectedInputs >= this->mInTextures.size() )
            this->routeOutputs();
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::connectExternalBuffer( UavBufferPacked *buffer, size_t inChannelA )
    {
        if( inChannelA >= this->mDefinition->mInputBuffers.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "There is no input buffer channel #" +
                         StringConverter::toString( inChannelA ) + " for node " +
                         mDefinition->mNameStr, "CompositorNode::connectExternalBuffer" );
        }

        CompositorNamedBuffer cmp;
        const IdString &inBufferName = mDefinition->mInputBuffers[inChannelA];
        CompositorNamedBufferVec::iterator inBuf = std::lower_bound( mBuffers.begin(),
                                                                     mBuffers.end(),
                                                                     inBufferName, cmp );

        if( inBufferName == IdString() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Input buffer channels must not have gaps."
                         " Channel #" + StringConverter::toString( inChannelA ) + " from node '" +
                         mDefinition->mNameStr + "' is not defined.",
                         "CompositorNode::connectExternalBuffer" );
        }

        if( inBuf != mBuffers.end() && inBuf->name == inBufferName )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Buffer with name '" +
                         inBufferName.getFriendlyText() + "' was already defined in node " +
                         mDefinition->mNameStr + " while trying to connect to its "
                         "channel #" + StringConverter::toString( inChannelA ) +
                         ". Did you define the buffer twice?",
                         "CompositorNode::connectExternalBuffer" );
        }
        else
        {
            const CompositorNamedBuffer namedBuffer( inBufferName, buffer );
            mBuffers.insert( inBuf, 1, namedBuffer );
            ++mNumConnectedBufferInputs;
        }
    }
    //-----------------------------------------------------------------------------------
    bool CompositorNode::areAllInputsConnected() const
    {
        return mNumConnectedInputs == mInTextures.size() &&
               mNumConnectedBufferInputs == mDefinition->mInputBuffers.size();
    }
    //-----------------------------------------------------------------------------------
    TexturePtr CompositorNode::getDefinedTexture( IdString textureName, size_t mrtIndex ) const
    {
        CompositorChannel const * channel = 0;
        size_t index;
        TextureDefinitionBase::TextureSource textureSource;
        mDefinition->getTextureSource( textureName, index, textureSource );
        switch( textureSource )
        {
        case TextureDefinitionBase::TEXTURE_INPUT:
            channel = &mInTextures[index];
            break;
        case TextureDefinitionBase::TEXTURE_LOCAL:
            channel = &mLocalTextures[index];
            break;
        case TextureDefinitionBase::TEXTURE_GLOBAL:
            channel = &mWorkspace->getGlobalTexture( textureName );
            break;
        default:
            break;
        }

        assert( !channel->textures.empty() && "Are you trying to use the RenderWindow as a texture???" );

        TexturePtr retVal;
        if( channel )
        {
            if( mrtIndex < channel->textures.size() )
                retVal = channel->textures[mrtIndex];
            else
                retVal = channel->textures.back();
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const CompositorChannel* CompositorNode::_getDefinedTexture( IdString textureName ) const
    {
        CompositorChannel const * channel = 0;
        size_t index;
        TextureDefinitionBase::TextureSource textureSource;
        mDefinition->getTextureSource( textureName, index, textureSource );
        switch( textureSource )
        {
        case TextureDefinitionBase::TEXTURE_INPUT:
            channel = &mInTextures[index];
            break;
        case TextureDefinitionBase::TEXTURE_LOCAL:
            channel = &mLocalTextures[index];
            break;
        case TextureDefinitionBase::TEXTURE_GLOBAL:
            channel = &mWorkspace->getGlobalTexture( textureName );
            break;
        default:
            break;
        }

        assert( !channel->textures.empty() && "Are you trying to use the RenderWindow as a texture???" );

        return channel;
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* CompositorNode::getDefinedBuffer( IdString bufferName ) const
    {
        UavBufferPacked *retVal = getDefinedBufferNoThrow( bufferName );

        if( !retVal )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Cannot find UAV buffer " +
                         bufferName.getFriendlyText() + " in node '" +
                         mDefinition->mNameStr + "'", "CompositorNode::getDefinedBuffer" );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    UavBufferPacked* CompositorNode::getDefinedBufferNoThrow( IdString bufferName ) const
    {
        UavBufferPacked *retVal = 0;

        CompositorNamedBuffer cmp;
        CompositorNamedBufferVec::const_iterator itBuf = std::lower_bound( mBuffers.begin(),
                                                                           mBuffers.end(),
                                                                           bufferName, cmp );
        if( itBuf != mBuffers.end() && itBuf->name == bufferName )
            retVal = itBuf->buffer;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::createPasses(void)
    {
        populateGlobalBuffers();

        CompositorTargetDefVec::const_iterator itor = mDefinition->mTargetPasses.begin();
        CompositorTargetDefVec::const_iterator end  = mDefinition->mTargetPasses.end();

        while( itor != end )
        {
            CompositorChannel const * channel = 0;
            size_t index;
            TextureDefinitionBase::TextureSource textureSource;
            mDefinition->getTextureSource( itor->getRenderTargetName(), index, textureSource );
            switch( textureSource )
            {
            case TextureDefinitionBase::TEXTURE_INPUT:
                channel = &mInTextures[index];
                break;
            case TextureDefinitionBase::TEXTURE_LOCAL:
                channel = &mLocalTextures[index];
                break;
            case TextureDefinitionBase::TEXTURE_GLOBAL:
                channel = &mWorkspace->getGlobalTexture( itor->getRenderTargetName() );
                break;
            default:
                continue;
            }

            const CompositorPassDefVec &passes = itor->getCompositorPasses();
            CompositorPassDefVec::const_iterator itPass = passes.begin();
            CompositorPassDefVec::const_iterator enPass = passes.end();

            while( itPass != enPass )
            {
                CompositorPass *newPass = 0;
                switch( (*itPass)->getType() )
                {
                case PASS_CLEAR:
                    newPass = OGRE_NEW CompositorPassClear(
                                            static_cast<CompositorPassClearDef*>(*itPass),
                                            mWorkspace->getSceneManager(), *channel, this );
                    break;
                case PASS_QUAD:
                    newPass = OGRE_NEW CompositorPassQuad(
                                            static_cast<CompositorPassQuadDef*>(*itPass),
                                            mWorkspace->getDefaultCamera(), this, *channel,
                                            mRenderSystem->getHorizontalTexelOffset(),
                                            mRenderSystem->getVerticalTexelOffset() );
                    break;
                case PASS_SCENE:
                    newPass = OGRE_NEW CompositorPassScene(
                                            static_cast<CompositorPassSceneDef*>(*itPass),
                                            mWorkspace->getDefaultCamera(), *channel, this );
                    break;
                case PASS_STENCIL:
                    newPass = OGRE_NEW CompositorPassStencil(
                                            static_cast<CompositorPassStencilDef*>(*itPass),
                                            *channel, this, mRenderSystem );
                    break;
                case PASS_DEPTHCOPY:
                    newPass = OGRE_NEW CompositorPassDepthCopy(
                                            static_cast<CompositorPassDepthCopyDef*>(*itPass),
                                            *channel, this );
                    break;
                case PASS_UAV:
                    newPass = OGRE_NEW CompositorPassUav(
                                            static_cast<CompositorPassUavDef*>(*itPass),
                                            this, *channel );
                    break;
                case PASS_MIPMAP:
                    newPass = OGRE_NEW CompositorPassMipmap(
                                            static_cast<CompositorPassMipmapDef*>(*itPass),
                                            *channel, this );
                    break;
                case PASS_COMPUTE:
                    newPass = OGRE_NEW CompositorPassCompute(
                                            static_cast<CompositorPassComputeDef*>(*itPass),
                                            mWorkspace->getDefaultCamera(),  this, *channel );
                    break;
                case PASS_CUSTOM:
                    {
                        CompositorPassProvider *passProvider = mWorkspace->getCompositorManager()->
                                                                        getCompositorPassProvider();
                        newPass = passProvider->addPass( *itPass, mWorkspace->getDefaultCamera(), this,
                                                         *channel, mWorkspace->getSceneManager() );
                    }
                    break;
                default:
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                "Pass type not implemented or not recognized",
                                "CompositorNode::initializePasses" );
                    break;
                }

                postInitializePass( newPass );

                mPasses.push_back( newPass );
                ++itPass;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::fillResourcesLayout( ResourceLayoutMap &outResourcesLayout,
                                              const CompositorChannelVec &compositorChannels,
                                              ResourceLayout::Layout layout )
    {
        CompositorChannelVec::const_iterator itor = compositorChannels.begin();
        CompositorChannelVec::const_iterator end  = compositorChannels.end();

        while( itor != end )
        {
            outResourcesLayout[itor->target] = layout;

            TextureVec::const_iterator itTex = itor->textures.begin();
            TextureVec::const_iterator enTex = itor->textures.end();

            while( itTex != enTex )
            {
                const Ogre::TexturePtr tex = *itTex;
                const size_t numFaces = tex->getNumFaces();
                const uint8 numMips = tex->getNumMipmaps() + 1;
                const uint32 numSlices = tex->getTextureType() == TEX_TYPE_CUBE_MAP ? 1u :
                                                                                      tex->getDepth();
                for( size_t face=0; face<numFaces; ++face )
                {
                    for( uint8 mip=0; mip<numMips; ++mip )
                    {
                        for( uint32 slice=0; slice<numSlices; ++slice )
                        {
                            RenderTarget *rt = tex->getBuffer( face, mip )->getRenderTarget( slice );
                            outResourcesLayout[rt] = layout;
                        }
                    }
                }

                ++itTex;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::initResourcesLayout( ResourceLayoutMap &outResourcesLayout,
                                              const CompositorChannelVec &compositorChannels,
                                              ResourceLayout::Layout layout )
    {
        CompositorChannelVec::const_iterator itor = compositorChannels.begin();
        CompositorChannelVec::const_iterator end  = compositorChannels.end();

        while( itor != end )
        {
            if( outResourcesLayout.find( itor->target ) == outResourcesLayout.end() )
                outResourcesLayout[itor->target] = layout;

            TextureVec::const_iterator itTex = itor->textures.begin();
            TextureVec::const_iterator enTex = itor->textures.end();

            while( itTex != enTex )
            {
                const Ogre::TexturePtr tex = *itTex;
                const size_t numFaces = tex->getNumFaces();
                const uint8 numMips = tex->getNumMipmaps() + 1;
                const uint32 numSlices = tex->getTextureType() == TEX_TYPE_CUBE_MAP ? 1u :
                                                                                      tex->getDepth();
                for( size_t face=0; face<numFaces; ++face )
                {
                    for( uint8 mip=0; mip<numMips; ++mip )
                    {
                        for( uint32 slice=0; slice<numSlices; ++slice )
                        {
                            RenderTarget *rt = tex->getBuffer( face, mip )->getRenderTarget( slice );
                            if( outResourcesLayout.find( rt ) == outResourcesLayout.end() )
                                outResourcesLayout[rt] = layout;
                        }
                    }
                }

                ++itTex;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::initResourcesLayout( ResourceLayoutMap &outResourcesLayout,
                                              const CompositorNamedBufferVec &buffers,
                                              ResourceLayout::Layout layout )
    {
        CompositorNamedBufferVec::const_iterator itor = buffers.begin();
        CompositorNamedBufferVec::const_iterator end  = buffers.end();

        while( itor != end )
        {
            if( outResourcesLayout.find( itor->buffer ) == outResourcesLayout.end() )
                outResourcesLayout[itor->buffer] = layout;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                               ResourceAccessMap &uavsAccess,
                                                               ResourceLayoutMap &resourcesLayout )
    {
        //All locally defined textures start as 'undefined'.
        fillResourcesLayout( resourcesLayout, mLocalTextures, ResourceLayout::Undefined );
        initResourcesLayout( resourcesLayout, mBuffers, ResourceLayout::Undefined );

        CompositorPassVec::const_iterator itPasses = mPasses.begin();
        CompositorPassVec::const_iterator enPasses = mPasses.end();

        while( itPasses != enPasses )
        {
            CompositorPass *pass = *itPasses;
            pass->_placeBarriersAndEmulateUavExecution( boundUavs, uavsAccess, resourcesLayout );

            ++itPasses;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_removeAllBarriers(void)
    {
        CompositorPassVec::const_iterator itPasses = mPasses.begin();
        CompositorPassVec::const_iterator enPasses = mPasses.end();

        while( itPasses != enPasses )
        {
            CompositorPass *pass = *itPasses;
            pass->_removeAllBarriers();

            ++itPasses;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_setFinalTargetAsRenderTarget(
                                                ResourceLayoutMap::iterator finalTargetCurrentLayout )
    {
        if( mPasses.empty() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Node " + mName.getFriendlyText() + " has no passes!",
                         "CompositorNode::_setFinalTargetAsRenderTarget" );
        }

        CompositorPass *pass = mPasses.back();
        pass->addResourceTransition( finalTargetCurrentLayout,
                                     ResourceLayout::RenderTarget,
                                     ReadBarrier::RenderTarget );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_update( const Camera *lodCamera, SceneManager *sceneManager )
    {
        //If we're in a caster pass, we need to skip shadow map passes that have no light associated
        const CompositorShadowNode *shadowNode = 0;
        if( sceneManager->_getCurrentRenderStage() == SceneManager::IRS_RENDER_TO_TEXTURE )
            shadowNode = sceneManager->getCurrentShadowNode();
        uint8 executionMask = mWorkspace->getExecutionMask();

        RenderTarget *lastTarget = 0;

        if( !mPasses.empty() )
            lastTarget = mPasses.front()->getRenderTarget();

        //Execute our passes
        CompositorPassVec::const_iterator itor = mPasses.begin();
        CompositorPassVec::const_iterator end  = mPasses.end();

        while( itor != end )
        {
            CompositorPass *pass = *itor;
            const CompositorPassDef *passDef = pass->getDefinition();

            if( lastTarget != pass->getRenderTarget() )
            {
                mRenderSystem->_notifyCompositorNodeSwitchedRenderTarget( lastTarget );
                lastTarget = pass->getRenderTarget();
            }

            const CompositorTargetDef *targetDef = passDef->getParentTargetDef();

            if( executionMask & passDef->mExecutionMask &&
                (!shadowNode || (!shadowNode->isShadowMapIdxInValidRange( passDef->mShadowMapIdx )
                || (shadowNode->_shouldUpdateShadowMapIdx( passDef->mShadowMapIdx )
                && (shadowNode->getShadowMapLightTypeMask( passDef->mShadowMapIdx ) &
                    targetDef->getShadowMapSupportedLightTypes())))) )
            {
                //Make explicitly exposed textures available to materials during this pass.
                const size_t oldNumTextures = sceneManager->getNumCompositorTextures();
                IdStringVec::const_iterator itExposed = passDef->mExposedTextures.begin();
                IdStringVec::const_iterator enExposed = passDef->mExposedTextures.end();

                while( itExposed != enExposed )
                {
                    const CompositorChannel *exposedChannel = this->_getDefinedTexture( *itExposed );
                    sceneManager->_addCompositorTexture( *itExposed, &exposedChannel->textures );
                    ++itExposed;
                }

                sceneManager->_setCompositorTarget( pass->getTargetTexture() );

                //Execute pass
                pass->execute( lodCamera );

                //Remove our textures
                sceneManager->_removeCompositorTextures( oldNumTextures );
            }
            ++itor;
        }

        if( !mPasses.empty() )
            mRenderSystem->_notifyCompositorNodeSwitchedRenderTarget( lastTarget );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::finalTargetResized( const RenderTarget *finalTarget )
    {
        TextureDefinitionBase::recreateResizableTextures( mDefinition->mLocalTextureDefs, mLocalTextures,
                                                            finalTarget, mRenderSystem, mConnectedNodes,
                                                            &mPasses );
        TextureDefinitionBase::recreateResizableBuffers( mDefinition->mLocalBufferDefs, mBuffers,
                                                         finalTarget, mRenderSystem, mConnectedNodes,
                                                         &mPasses );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::resetAllNumPassesLeft(void)
    {
        CompositorPassVec::const_iterator itor = mPasses.begin();
        CompositorPassVec::const_iterator end  = mPasses.end();

        while( itor != end )
        {
            (*itor)->resetNumPassesLeft();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    size_t CompositorNode::getPassNumber( CompositorPass *pass ) const
    {
        return mDefinition->getPassNumber( pass->getDefinition() );
    }
}
