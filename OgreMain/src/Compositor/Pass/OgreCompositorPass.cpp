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

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorChannel.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "OgreViewport.h"

#include "OgreRenderSystem.h"
#include "OgreProfiler.h"

#include "OgreStringConverter.h"

namespace Ogre
{
//    const Quaternion CompositorPass::CubemapRotations[6] =
//    {
//        Quaternion( Degree(-90 ), Vector3::UNIT_Y ),          //+X
//        Quaternion( Degree( 90 ), Vector3::UNIT_Y ),          //-X
//        Quaternion( Degree( 90 ), Vector3::UNIT_X ),          //+Y
//        Quaternion( Degree(-90 ), Vector3::UNIT_X ),          //-Y
//        Quaternion::IDENTITY,                                 //+Z
//        Quaternion( Degree(180 ), Vector3::UNIT_Y )           //-Z
//    };
    //Can NOT use UNIT_X/UNIT_Y and Degree here because on iOS C++ initialization
    //order screws us and thus key global variables are still 0. Hardcode the values.
    const Quaternion CompositorPass::CubemapRotations[6] =
    {
        Quaternion( Radian(-1.570796f ), Vector3( 0, 1, 0 ) ),  //+X
        Quaternion( Radian( 1.570796f ), Vector3( 0, 1, 0 ) ),  //-X
        Quaternion( Radian( 1.570796f ), Vector3( 1, 0, 0 ) ),  //+Y
        Quaternion( Radian(-1.570796f ), Vector3( 1, 0, 0 ) ),  //-Y
        Quaternion( 1, 0, 0, 0 ),                               //+Z
        Quaternion( Radian( 3.1415927f), Vector3( 0, 1, 0 ) )   //-Z
    };

    CompositorPass::CompositorPass( const CompositorPassDef *definition, const CompositorChannel &target,
                                    CompositorNode *parentNode ) :
            mDefinition( definition ),
            mTarget( 0 ),
            mViewport( 0 ),
            mNumPassesLeft( definition->mNumInitialPasses ),
            mParentNode( parentNode ),
            mTargetTexture( IdString(), &target.textures ),
            mNumValidResourceTransitions( 0 )
    {
        assert( definition->mNumInitialPasses && "Definition is broken, pass will never execute!" );

        mTarget = calculateRenderTarget( mDefinition->getRtIndex(), target );

        //TODO: Merging RenderTarget & Texture in a refactor would get rid of these missmatches
        //between a "target" and a "texture" in mTargetTexture.
        if( !target.textures.empty() )
            mTargetTexture.name = IdString( target.textures[0]->getName() );

        const Real EPSILON = 1e-6f;

        CompositorWorkspace *workspace = mParentNode->getWorkspace();
        uint8 workspaceVpMask = workspace->getViewportModifierMask();

        bool applyModifier = (workspaceVpMask & mDefinition->mViewportModifierMask) != 0;
        Vector4 vpModifier = applyModifier ? workspace->getViewportModifier() : Vector4( 0, 0, 1, 1 );

        Real left   = mDefinition->mVpLeft      + vpModifier.x;
        Real top    = mDefinition->mVpTop       + vpModifier.y;
        Real width  = mDefinition->mVpWidth     * vpModifier.z;
        Real height = mDefinition->mVpHeight    * vpModifier.w;

        Real scLeft   = mDefinition->mVpScissorLeft     + vpModifier.x;
        Real scTop    = mDefinition->mVpScissorTop      + vpModifier.y;
        Real scWidth  = mDefinition->mVpScissorWidth    * vpModifier.z;
        Real scHeight = mDefinition->mVpScissorHeight   * vpModifier.w;

        const unsigned short numViewports = mTarget->getNumViewports();
        for( unsigned short i=0; i<numViewports && !mViewport; ++i )
        {
            Viewport *vp = mTarget->getViewport(i);
            if( Math::Abs( vp->getLeft() - left )   < EPSILON &&
                Math::Abs( vp->getTop() - top )     < EPSILON &&
                Math::Abs( vp->getWidth() - width ) < EPSILON &&
                Math::Abs( vp->getHeight() - height )<EPSILON &&
                Math::Abs( vp->getScissorLeft() - scLeft )   < EPSILON &&
                Math::Abs( vp->getScissorTop() - scTop )     < EPSILON &&
                Math::Abs( vp->getScissorWidth() - scWidth ) < EPSILON &&
                Math::Abs( vp->getScissorHeight() - scHeight )<EPSILON &&
                vp->getColourWrite() == mDefinition->mColourWrite &&
                vp->getReadOnlyDepth() == mDefinition->mReadOnlyDepth &&
                vp->getReadOnlStencil() == mDefinition->mReadOnlyStencil &&
                vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
            {
                mViewport = vp;
            }
        }

        if( !mViewport )
        {
            mViewport = mTarget->addViewport( left, top, width, height );
            mViewport->setScissors( scLeft, scTop, scWidth, scHeight );
            mViewport->setColourWrite( mDefinition->mColourWrite );
            mViewport->setReadOnly( mDefinition->mReadOnlyDepth, mDefinition->mReadOnlyStencil );
            mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
        }

        populateTextureDependenciesFromExposedTextures();
    }
    //-----------------------------------------------------------------------------------
    CompositorPass::~CompositorPass()
    {
        _removeAllBarriers();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::populateTextureDependenciesFromExposedTextures(void)
    {
        IdStringVec::const_iterator itor = mDefinition->mExposedTextures.begin();
        IdStringVec::const_iterator end  = mDefinition->mExposedTextures.end();

        while( itor != end )
        {
            const CompositorChannel *channel = mParentNode->_getDefinedTexture( *itor );
            mTextureDependencies.push_back( CompositorTexture( *itor, &channel->textures ) );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::executeResourceTransitions(void)
    {
        RenderSystem *renderSystem = mParentNode->getRenderSystem();

        assert( mNumValidResourceTransitions <= mResourceTransitions.size() );
        ResourceTransitionVec::iterator itor = mResourceTransitions.begin();

        for( size_t i=0; i<mNumValidResourceTransitions; ++i )
        {
            renderSystem->_executeResourceTransition( &(*itor) );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::profilingBegin(void)
    {
#if OGRE_PROFILING
        if( !mParentNode->getWorkspace()->getAmalgamatedProfiling() )
        {
            OgreProfileBeginDynamic( mDefinition->mProfilingId.c_str() );
            OgreProfileGpuBeginDynamic( mDefinition->mProfilingId );
        }
#endif
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::profilingEnd(void)
    {
#if OGRE_PROFILING
        if( !mParentNode->getWorkspace()->getAmalgamatedProfiling() )
        {
            OgreProfileEnd( mDefinition->mProfilingId );
            OgreProfileGpuEnd( mDefinition->mProfilingId );
        }
#endif
    }
    //-----------------------------------------------------------------------------------
    RenderTarget* CompositorPass::calculateRenderTarget( size_t rtIndex,
                                                         const CompositorChannel &source )
    {
        RenderTarget *retVal;

        if( !source.isMrt() && !source.textures.empty() &&
            source.textures[0]->getTextureType() > TEX_TYPE_2D )
        {
            //Who had the bright idea of handling Cubemaps differently
            //than 3D textures is a mystery. Anyway, deal with it.
            TexturePtr texturePtr = source.textures[0];

            if( rtIndex >= texturePtr->getDepth() && rtIndex >= texturePtr->getNumFaces() )
            {
                size_t maxRTs = std::max<size_t>( source.textures[0]->getDepth(),
                                                    source.textures[0]->getNumFaces() );
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Compositor pass is asking for a 3D/Cubemap/2D_array texture with "
                        "more faces/depth/slices than what's been supplied (Asked for slice '" +
                        StringConverter::toString( rtIndex ) + "', RT has '" +
                        StringConverter::toString( maxRTs ) + "')",
                        "CompositorPass::calculateRenderTarget" );
            }

            /*//If goes out bounds, will reference the last slice/face
            rtIndex = std::min( rtIndex, std::max( source.textures[0]->getDepth(),
                                                    source.textures[0]->getNumFaces() ) - 1 );*/

            TextureType textureType = texturePtr->getTextureType();
            size_t face = textureType == TEX_TYPE_CUBE_MAP ? rtIndex : 0;
            size_t slice= textureType != TEX_TYPE_CUBE_MAP ? rtIndex : 0;
            retVal = texturePtr->getBuffer( face )->getRenderTarget( slice );
        }
        else
        {
            retVal = source.target;
        }

        return retVal;
    }
    inline uint32 transitionWriteBarrierBits( ResourceLayout::Layout oldLayout )
    {
        uint32 retVal = 0;
        switch( oldLayout )
        {
        case ResourceLayout::RenderTarget:
            retVal = WriteBarrier::RenderTarget;
            break;
        case ResourceLayout::RenderDepth:
            retVal = WriteBarrier::DepthStencil;
            break;
        case ResourceLayout::Uav:
            retVal = WriteBarrier::Uav;
            break;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::addResourceTransition( ResourceLayoutMap::iterator currentLayout,
                                                ResourceLayout::Layout newLayout,
                                                uint32 readBarrierBits )
    {
        ResourceTransition transition;
        //transition.resource = ; TODO
        transition.oldLayout = currentLayout->second;
        transition.newLayout = newLayout;
        transition.writeBarrierBits = transitionWriteBarrierBits( transition.oldLayout );
        transition.readBarrierBits  = readBarrierBits;

        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
        if( !caps->hasCapability( RSC_EXPLICIT_API ) )
        {
            //OpenGL. Merge the bits and use only one global barrier.
            //Keep the extra barriers uninitialized for debugging purposes,
            //but we won't be really using them.
            if( mResourceTransitions.empty() )
            {
                ResourceTransition globalBarrier;
                globalBarrier.oldLayout = ResourceLayout::Undefined;
                globalBarrier.newLayout = ResourceLayout::Undefined;
                globalBarrier.writeBarrierBits  = transition.writeBarrierBits;
                globalBarrier.readBarrierBits   = transition.readBarrierBits;
                globalBarrier.mRsData = 0;
                renderSystem->_resourceTransitionCreated( &globalBarrier );
                mResourceTransitions.push_back( globalBarrier );
            }
            else
            {
                ResourceTransition &globalBarrier = mResourceTransitions.front();

                renderSystem->_resourceTransitionDestroyed( &globalBarrier );

                globalBarrier.writeBarrierBits  |= transition.writeBarrierBits;
                globalBarrier.readBarrierBits   |= transition.readBarrierBits;

                renderSystem->_resourceTransitionCreated( &globalBarrier );
            }

            mNumValidResourceTransitions = 1;
        }
        else
        {
            //D3D12, Vulkan, Mantle. Takes advantage of per-resource barriers
            renderSystem->_resourceTransitionCreated( &transition );
            ++mNumValidResourceTransitions;
        }

        mResourceTransitions.push_back( transition );

        currentLayout->second = transition.newLayout;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::_placeBarriersAndEmulateUavExecution(
                                            BoundUav boundUavs[64], ResourceAccessMap &uavsAccess,
                                            ResourceLayoutMap &resourcesLayout )
    {
        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
        const bool explicitApi = caps->hasCapability( RSC_EXPLICIT_API );

        {
            //mResourceTransitions will be non-empty if we call _placeBarriersAndEmulateUavExecution
            //for a second time (i.e. 2nd pass to check frame-to-frame dependencies). We need
            //to tell what is shielded. On the first time, it should be empty.
            ResourceTransitionVec::const_iterator itor = mResourceTransitions.begin();
            ResourceTransitionVec::const_iterator end  = mResourceTransitions.end();

            while( itor != end )
            {
                if( itor->newLayout == ResourceLayout::Uav &&
                    itor->writeBarrierBits & WriteBarrier::Uav &&
                    itor->readBarrierBits & ReadBarrier::Uav )
                {
                    RenderTarget *renderTarget = 0;
                    resourcesLayout[renderTarget] = ResourceLayout::Uav;
                    //Set to undefined so that following passes
                    //can see it's safe / shielded by a barrier
                    uavsAccess[renderTarget] = ResourceAccess::Undefined;
                }
                ++itor;
            }
        }

        {
            //Check <anything> -> RT
            ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( mTarget );
            if( (currentLayout->second != ResourceLayout::RenderTarget && explicitApi) ||
                currentLayout->second == ResourceLayout::Uav )
            {
                addResourceTransition( currentLayout,
                                       ResourceLayout::RenderTarget,
                                       ReadBarrier::RenderTarget );
            }
        }

        {
            //Check <anything> -> Texture
            CompositorTextureVec::const_iterator itDep = mTextureDependencies.begin();
            CompositorTextureVec::const_iterator enDep = mTextureDependencies.end();

            while( itDep != enDep )
            {
                TexturePtr texture = itDep->textures->front();
                RenderTarget *renderTarget = texture->getBuffer()->getRenderTarget();

                ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( renderTarget );

                if( (currentLayout->second != ResourceLayout::Texture && explicitApi) ||
                     currentLayout->second == ResourceLayout::Uav )
                {
                    //TODO: Account for depth textures.
                    addResourceTransition( currentLayout,
                                           ResourceLayout::Texture,
                                           ReadBarrier::Texture );
                }

                ++itDep;
            }
        }

        //Check <anything> -> UAV; including UAV -> UAV
        //Except for RaR (Read after Read) and some WaW (Write after Write),
        //Uavs are always hazardous, an UAV->UAV 'transition' is just a memory barrier.
        CompositorPassDef::UavDependencyVec::const_iterator itor = mDefinition->mUavDependencies.begin();
        CompositorPassDef::UavDependencyVec::const_iterator end  = mDefinition->mUavDependencies.end();

        while( itor != end )
        {
            GpuResource *uavRt = boundUavs[itor->uavSlot].rttOrBuffer;

            if( !uavRt )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "No UAV is bound at slot " + StringConverter::toString( itor->uavSlot ) +
                             " but it is marked as used by node " +
                             mParentNode->getName().getFriendlyText() + "; pass #" +
                             StringConverter::toString( mParentNode->getPassNumber( this ) ),
                             "CompositorPass::emulateUavExecution" );
            }

            if( !(itor->access & boundUavs[itor->uavSlot].boundAccess) )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Node " + mParentNode->getName().getFriendlyText() + "; pass #" +
                             StringConverter::toString( mParentNode->getPassNumber( this ) ) +
                             " marked " + ResourceAccess::toString( itor->access ) +
                             " access to UAV at slot " +
                             StringConverter::toString( itor->uavSlot ) + " but this UAV is bound for " +
                             ResourceAccess::toString( boundUavs[itor->uavSlot].boundAccess ) +
                             " access.", "CompositorPass::emulateUavExecution" );
            }

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
    //-----------------------------------------------------------------------------------
    void CompositorPass::_removeAllBarriers(void)
    {
        assert( mNumValidResourceTransitions <= mResourceTransitions.size() );

        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        ResourceTransitionVec::iterator itor = mResourceTransitions.begin();

        for( size_t i=0; i<mNumValidResourceTransitions; ++i )
        {
            renderSystem->_resourceTransitionDestroyed( &(*itor) );
            ++itor;
        }

        mNumValidResourceTransitions = 0;
        mResourceTransitions.clear();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyRecreated( const CompositorChannel &oldChannel,
                                          const CompositorChannel &newChannel )
    {
        const Real EPSILON = 1e-6f;

        if( mTarget == calculateRenderTarget( mDefinition->getRtIndex(), oldChannel ) )
        {
            mTarget = calculateRenderTarget( mDefinition->getRtIndex(), newChannel );

            mNumPassesLeft = mDefinition->mNumInitialPasses;

            mViewport = 0;

            CompositorWorkspace *workspace = mParentNode->getWorkspace();
            uint8 workspaceVpMask = workspace->getViewportModifierMask();

            bool applyModifier = (workspaceVpMask & mDefinition->mViewportModifierMask) != 0;
            Vector4 vpModifier = applyModifier ? workspace->getViewportModifier() : Vector4( 0, 0, 1, 1 );

            Real left   = mDefinition->mVpLeft      + vpModifier.x;
            Real top    = mDefinition->mVpTop       + vpModifier.y;
            Real width  = mDefinition->mVpWidth     * vpModifier.z;
            Real height = mDefinition->mVpHeight    * vpModifier.w;

            Real scLeft   = mDefinition->mVpScissorLeft     + vpModifier.x;
            Real scTop    = mDefinition->mVpScissorTop      + vpModifier.y;
            Real scWidth  = mDefinition->mVpScissorWidth    * vpModifier.z;
            Real scHeight = mDefinition->mVpScissorHeight   * vpModifier.w;

            const unsigned short numViewports = mTarget->getNumViewports();
            for( unsigned short i=0; i<numViewports && !mViewport; ++i )
            {
                Viewport *vp = mTarget->getViewport(i);
                if( Math::Abs( vp->getLeft() - left )   < EPSILON &&
                    Math::Abs( vp->getTop() - top )     < EPSILON &&
                    Math::Abs( vp->getWidth() - width ) < EPSILON &&
                    Math::Abs( vp->getHeight() - height )<EPSILON &&
                    Math::Abs( vp->getScissorLeft() - scLeft )   < EPSILON &&
                    Math::Abs( vp->getScissorTop() - scTop )     < EPSILON &&
                    Math::Abs( vp->getScissorWidth() - scWidth ) < EPSILON &&
                    Math::Abs( vp->getScissorHeight() - scHeight )<EPSILON &&
                    vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
                {
                    mViewport = vp;
                }
            }

            if( !mViewport )
            {
                mViewport = mTarget->addViewport( mDefinition->mVpLeft, mDefinition->mVpTop,
                                                  mDefinition->mVpWidth, mDefinition->mVpHeight );
                mViewport->setScissors( scLeft, scTop, scWidth, scHeight );
                mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyRecreated( const UavBufferPacked *oldBuffer, UavBufferPacked *newBuffer )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyDestroyed( const CompositorChannel &channel )
    {
        if( mTarget == calculateRenderTarget( mDefinition->getRtIndex(), channel ) )
            mTarget = 0;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyDestroyed( const UavBufferPacked *buffer )
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::notifyCleared(void)
    {
        mTarget = 0;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPass::resetNumPassesLeft(void)
    {
        mNumPassesLeft = mDefinition->mNumInitialPasses;
    }
}
