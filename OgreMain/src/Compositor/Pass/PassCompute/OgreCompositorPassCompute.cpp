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

#include "Vao/OgreUavBufferPacked.h"

#include "OgreRenderTexture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    void CompositorPassComputeDef::addTextureSource( uint32 texUnitIdx, const String &textureName,
                                                     uint32 mrtIndex )
    {
        if( textureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( textureName, 0,
                                                  TextureDefinitionBase::TEXTURE_GLOBAL );
        }
        mTextureSources.push_back( ComputeTextureSource( texUnitIdx, textureName, mrtIndex ) );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassComputeDef::addUavSource( uint32 texUnitIdx, const String &textureName,
                                                 uint32 mrtIndex,
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
//    void CompositorPassComputeDef::addTexBuffer( uint32 slotIdx, const String &bufferName,
//                                                 size_t offset, size_t sizeBytes )
//    {
//        //TODO.
//    }
    //-----------------------------------------------------------------------------------
    void CompositorPassComputeDef::addUavBuffer( uint32 slotIdx, const String &bufferName,
                                                 ResourceAccess::ResourceAccess access, size_t offset,
                                                 size_t sizeBytes, bool allowWriteAfterWrite )
    {
        assert( access != ResourceAccess::Undefined );
        mBufferSources.push_back( BufferSource( slotIdx, bufferName, access, offset,
                                                sizeBytes, allowWriteAfterWrite ) );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassCompute::CompositorPassCompute( const CompositorPassComputeDef *definition,
                                                  Camera *defaultCamera,
                                                  CompositorNode *parentNode,
                                                  const CompositorChannel &target ) :
        CompositorPass( definition, target, parentNode ),
        mDefinition( definition ),
        mCamera( 0 )
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

            if( it == en )
            {
                mTextureDependencies.push_back( CompositorTexture( itor->textureName,
                                                                   &channel->textures ) );
            }

            ++itor;
        }

        {
            //Ensure our compute job has enough UAV units available.
            uint8 maxUsedSlot = 0u;
            const CompositorPassComputeDef::TextureSources &uavSources = mDefinition->getUavSources();
            CompositorPassComputeDef::TextureSources::const_iterator it = uavSources.begin();
            CompositorPassComputeDef::TextureSources::const_iterator en = uavSources.end();
            while( it != en )
            {
                maxUsedSlot = std::max( maxUsedSlot, static_cast<uint8>(it->texUnitIdx) );
                ++it;
            }

            if( maxUsedSlot >= mComputeJob->getNumUavUnits() )
                mComputeJob->setNumUavUnits( maxUsedSlot + 1u );
        }

        setResourcesToJob();

        const CompositorWorkspace *workspace = parentNode->getWorkspace();
        if( mDefinition->mCameraName != IdString() )
            mCamera = workspace->findCamera( mDefinition->mCameraName );
        else
            mCamera = defaultCamera;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassCompute::setResourcesToJob(void)
    {
        {
            const CompositorPassComputeDef::TextureSources &textureSources =
                    mDefinition->getTextureSources();
            CompositorPassComputeDef::TextureSources::const_iterator itor = textureSources.begin();
            CompositorPassComputeDef::TextureSources::const_iterator end  = textureSources.end();
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
                mComputeJob->_setUavTexture( itor->texUnitIdx, texture, itor->textureArrayIndex,
                                             itor->access, itor->mipmapLevel, itor->pixelFormat );
                ++itor;
            }
        }

        {
            const CompositorPassComputeDef::BufferSourceVec &bufferSources =
                    mDefinition->getBufferSources();
            CompositorPassComputeDef::BufferSourceVec::const_iterator itor = bufferSources.begin();
            CompositorPassComputeDef::BufferSourceVec::const_iterator end  = bufferSources.end();

            while( itor != end )
            {
                UavBufferPacked *uavBuffer = mParentNode->getDefinedBuffer( itor->bufferName );
                mComputeJob->_setUavBuffer( itor->slotIdx, uavBuffer, itor->access,
                                            itor->offset, itor->sizeBytes );
                ++itor;
            }
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

        profilingBegin();

        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passEarlyPreExecute( this );

        //Call beginUpdate if we're the first to use this RT
        if( mDefinition->mBeginRtUpdate )
            mTarget->_beginUpdate();

        executeResourceTransitions();

        //Set textures/uavs every frame
        setResourcesToJob();

        //Fire the listener in case it wants to change anything
        if( listener )
            listener->passPreExecute( this );

        assert( dynamic_cast<HlmsCompute*>( mComputeJob->getCreator() ) );

        SceneManager *sceneManager = 0;
        if( mCamera )
            sceneManager = mCamera->getSceneManager();

        HlmsCompute *hlmsCompute = static_cast<HlmsCompute*>( mComputeJob->getCreator() );
        hlmsCompute->dispatch( mComputeJob, sceneManager, mCamera );

        if( listener )
            listener->passPosExecute( this );

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();

        profilingEnd();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassCompute::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                                      ResourceAccessMap &uavsAccess,
                                                                      ResourceLayoutMap &resourcesLayout )
    {
        CompositorPass::_placeBarriersAndEmulateUavExecution( boundUavs, uavsAccess, resourcesLayout );

        {
            //<anything> -> Texture UAVs
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

        {
            //<anything> -> Buffer UAVs
            const CompositorPassComputeDef::BufferSourceVec &bufferSources =
                    mDefinition->getBufferSources();
            CompositorPassComputeDef::BufferSourceVec::const_iterator itor = bufferSources.begin();
            CompositorPassComputeDef::BufferSourceVec::const_iterator end  = bufferSources.end();

            while( itor != end )
            {
                UavBufferPacked *uavBuffer = mParentNode->getDefinedBuffer( itor->bufferName );

                ResourceAccessMap::iterator itResAccess = uavsAccess.find( uavBuffer );

                if( itResAccess == uavsAccess.end() )
                {
                    //First time accessing the UAV. If we need a barrier,
                    //we will see it in the second pass.
                    uavsAccess[uavBuffer] = ResourceAccess::Undefined;
                    itResAccess = uavsAccess.find( uavBuffer );
                }

                ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( uavBuffer );

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
}
