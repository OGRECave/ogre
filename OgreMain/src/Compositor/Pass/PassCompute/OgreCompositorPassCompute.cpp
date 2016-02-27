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

#include "Compositor/Pass/PassCompute/OgreCompositorPassCompute.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassComputeDef.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"

#include "OgreRenderTexture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    void CompositorPassComputeDef::addTextureSource( size_t texUnitIdx, const String &textureName,
                                                     size_t mrtIndex )
    {
        if( textureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( textureName, 0,
                                                  TextureDefinitionBase::TEXTURE_GLOBAL );
        }
        mTextureSources.push_back( ComputeTextureSource( texUnitIdx, textureName, mrtIndex ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassComputeDef::addUavSource( size_t texUnitIdx, const String &textureName,
                                                 size_t mrtIndex,
                                                 ResourceAccess::ResourceAccess access,
                                                 int32 textureArrayIndex, int32 mipmapLevel,
                                                 PixelFormat pixelFormat,
                                                 bool allowWriteAfterWrite )
    {
        if( textureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( textureName, 0,
                                                  TextureDefinitionBase::TEXTURE_GLOBAL );
        }
        mUavSources.push_back( ComputeTextureSource( texUnitIdx, textureName, mrtIndex, access,
                                                     mipmapLevel, textureArrayIndex, pixelFormat,
                                                     allowWriteAfterWrite ) );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassCompute::CompositorPassCompute( const CompositorPassComputeDef *definition,
                                                  CompositorNode *parentNode,
                                                  const CompositorChannel &target ) :
        CompositorPass( definition, target, parentNode ),
        mDefinition( definition )
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

        mComputeJob = hlmsCompute->findComputeJob( mDefinition->mJobName );

        assert( mDefinition->mExposedTextures.empty() &&
                "Invalid parameters set to the pass definition. Barriers may not behave correctly" );

        //List all our RTT dependencies
		const CompositorPassComputeDef::TextureSources &textureSources = mDefinition->getTextureSources();
		CompositorPassComputeDef::TextureSources::const_iterator itor = textureSources.begin();
		CompositorPassComputeDef::TextureSources::const_iterator end  = textureSources.end();
        while( itor != end )
        {
            const CompositorChannel *channel = mParentNode->_getDefinedTexture( itor->textureName );
            CompositorTextureVec::const_iterator it = mTextureDependencies.begin();
            CompositorTextureVec::const_iterator en = mTextureDependencies.end();
            while( it != en && it->name != itor->textureName )
                ++it;

            if( it != en )
            {
                mTextureDependencies.push_back( CompositorTexture( itor->textureName,
                                                                   &channel->textures ) );
            }

            ++itor;
        }

        itor = textureSources.begin();
        end  = textureSources.end();
        while( itor != end )
        {
            TexturePtr texture = mParentNode->getDefinedTexture( itor->textureName, itor->mrtIndex );
            mComputeJob->setTexture( itor->texUnitIdx, texture );
            ++itor;
        }

        const CompositorPassComputeDef::TextureSources &uavSources = mDefinition->getUavSources();
        itor = uavSources.begin();
        end  = uavSources.end();
        while( itor != end )
        {
            TexturePtr texture = mParentNode->getDefinedTexture( itor->textureName, itor->mrtIndex );
            mComputeJob->setUavTexture( itor->texUnitIdx, texture, itor->textureArrayIndex,
                                        itor->access, itor->mipmapLevel, itor->pixelFormat );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
	void CompositorPassCompute::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        //Call beginUpdate if we're the first to use this RT
        if( mDefinition->mBeginRtUpdate )
            mTarget->_beginUpdate();

        executeResourceTransitions();

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        assert( dynamic_cast<HlmsCompute*>( mComputeJob->getCreator() ) );

        HlmsCompute *hlmsCompute = static_cast<HlmsCompute*>( mComputeJob->getCreator() );
        hlmsCompute->dispatch( mComputeJob );

        if( listener )
            listener->passPosExecute( this );

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassCompute::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                                      ResourceAccessMap &uavsAccess,
                                                                      ResourceLayoutMap &resourcesLayout )
    {
        CompositorPass::_placeBarriersAndEmulateUavExecution( boundUavs, uavsAccess, resourcesLayout );

        const CompositorPassComputeDef::TextureSources &uavSources = mDefinition->getUavSources();
        CompositorPassComputeDef::TextureSources::const_iterator itor = uavSources.begin();
        CompositorPassComputeDef::TextureSources::const_iterator end  = uavSources.end();

        while( itor != end )
        {
            TexturePtr uavTex = mParentNode->getDefinedTexture( itor->textureName, itor->mrtIndex );

            //TODO: Do we have to do this for all slices? Or just one? Refactor is needed.
            RenderTarget *uavRt = uavTex->getBuffer( 0, 0 )->getRenderTarget( 0 );

            ResourceAccessMap::iterator itResAccess = uavsAccess.find( uavRt );

            if( itResAccess == uavsAccess.end() )
            {
                //First time accessing the UAV. If we need a barrier,
                //we will see it in the second pass.
                uavsAccess[uavRt] = ResourceAccess::Undefined;
                itResAccess = uavsAccess.find( uavRt );
            }

            ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( uavRt );

            if( currentLayout->second != ResourceLayout::Uav ||
                !( (itor->access == ResourceAccess::Read &&
                    itResAccess->second == ResourceAccess::Read) ||
                   (itor->access == ResourceAccess::Write &&
                    itResAccess->second == ResourceAccess::Write &&
                    itor->allowWriteAfterWrite) ||
                   itResAccess->second == ResourceAccess::Undefined ) )
            {
                //Not RaR (or not WaW when they're explicitly allowed). Insert the barrier.
                //We also may need the barrier if the resource wasn't an UAV.
                addResourceTransition( currentLayout, ResourceLayout::Uav, ReadBarrier::Uav );
            }

            itResAccess->second = itor->access;
            ++itor;
        }
    }
}
