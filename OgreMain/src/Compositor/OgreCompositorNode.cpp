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
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuad.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/Pass/PassStencil/OgreCompositorPassStencil.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreRenderSystem.h"
#include "OgreSceneManager.h"

namespace Ogre
{
    CompositorNode::CompositorNode( IdType id, IdString name, const CompositorNodeDef *definition,
                                    CompositorWorkspace *workspace, RenderSystem *renderSys,
                                    const RenderTarget *finalTarget ) :
            IdObject( id ),
            mName( name ),
            mEnabled( definition->mStartEnabled ),
            mNumConnectedInputs( 0 ),
            mWorkspace( workspace ),
            mRenderSystem( renderSys ),
            mDefinition( definition )
    {
        mInTextures.resize( mDefinition->getNumInputChannels(), CompositorChannel() );
        mOutTextures.resize( mDefinition->mOutChannelMapping.size() );

        //Create local textures
        TextureDefinitionBase::createTextures( definition->mLocalTextureDefs, mLocalTextures,
                                                id, false, finalTarget, mRenderSystem );

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

            ++itor;
        }

        mConnectedNodes.clear();
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
    void CompositorNode::_notifyCleared(void)
    {
        //Clear our inputs
        CompositorChannelVec::iterator texIt = mInTextures.begin();
        CompositorChannelVec::iterator texEn = mInTextures.end();

        while( texIt != texEn )
            *texIt++ = CompositorChannel();

        mNumConnectedInputs = 0;

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
    void CompositorNode::connectTo( size_t outChannelA, CompositorNode *nodeB, size_t inChannelB )
    {
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
    void CompositorNode::connectFinalRT( RenderTarget *rt, CompositorChannel::TextureVec &textures,
                                            size_t inChannelA )
    {
        if( !mInTextures[inChannelA].target )
            ++mNumConnectedInputs;
        mInTextures[inChannelA].target      = rt;
        mInTextures[inChannelA].textures    = textures;

        if( this->mNumConnectedInputs >= this->mInTextures.size() )
            this->routeOutputs();
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
    void CompositorNode::createPasses(void)
    {
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
                    postInitializePassScene( static_cast<CompositorPassScene*>( newPass ) );
                    break;
                case PASS_STENCIL:
                    newPass = OGRE_NEW CompositorPassStencil(
                                            static_cast<CompositorPassStencilDef*>(*itPass),
                                            *channel, this, mRenderSystem );
                    break;
                default:
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                "Pass type not implemented or not recognized",
                                "CompositorNode::initializePasses" );
                    break;
                }

                mPasses.push_back( newPass );
                ++itPass;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::_update( const Camera *lodCamera, SceneManager *sceneManager )
    {
        //Add local & input textures to the SceneManager so they can be referenced by materials
        size_t oldNumTextures = sceneManager->getNumCompositorTextures();
        TextureDefinitionBase::NameToChannelMap::const_iterator it =
                                                                mDefinition->mNameToChannelMap.begin();
        TextureDefinitionBase::NameToChannelMap::const_iterator en =
                                                                mDefinition->mNameToChannelMap.end();

        while( it != en )
        {
            size_t index;
            TextureDefinitionBase::TextureSource texSource;
            mDefinition->decodeTexSource( it->second, index, texSource );
            if( texSource == TextureDefinitionBase::TEXTURE_INPUT )
                sceneManager->_addCompositorTexture( it->first, &mInTextures[index].textures );
            else if( texSource == TextureDefinitionBase::TEXTURE_LOCAL )
                sceneManager->_addCompositorTexture( it->first, &mLocalTextures[index].textures );

            ++it;
        }

        //Execute our passes
        CompositorPassVec::const_iterator itor = mPasses.begin();
        CompositorPassVec::const_iterator end  = mPasses.end();

        while( itor != end )
        {
            CompositorPass *pass = *itor;
            pass->execute( lodCamera );
            ++itor;
        }

        //Remove our textures
        sceneManager->_removeCompositorTextures( oldNumTextures );
    }
    //-----------------------------------------------------------------------------------
    void CompositorNode::finalTargetResized( const RenderTarget *finalTarget )
    {
        TextureDefinitionBase::recreateResizableTextures( mDefinition->mLocalTextureDefs, mLocalTextures,
                                                            finalTarget, mRenderSystem, mConnectedNodes,
                                                            &mPasses );
    }
}
