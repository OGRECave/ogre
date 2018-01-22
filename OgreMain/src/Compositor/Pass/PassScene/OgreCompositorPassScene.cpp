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

#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreSceneManager.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"

namespace Ogre
{
    CompositorPassScene::CompositorPassScene( const CompositorPassSceneDef *definition,
                                                Camera *defaultCamera, const CompositorChannel &target,
                                                CompositorNode *parentNode ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition ),
                mShadowNode( 0 ),
                mCamera( 0 ),
                mLodCamera( 0 ),
                mCullCamera( 0 ),
                mUpdateShadowNode( false ),
                mPrePassTextures( 0 ),
                mPrePassDepthTexture( 0 ),
                mSsrTexture( 0 )
    {
        CompositorWorkspace *workspace = parentNode->getWorkspace();

        if( mDefinition->mShadowNode != IdString() )
        {
            bool shadowNodeCreated;
            mShadowNode = workspace->findOrCreateShadowNode( mDefinition->mShadowNode,
                                                             shadowNodeCreated );

            // Passes with "first_only" option are set in CompositorWorkspace::setupPassesShadowNodes
            if( mDefinition->mShadowNodeRecalculation != SHADOW_NODE_FIRST_ONLY )
                mUpdateShadowNode = mDefinition->mShadowNodeRecalculation == SHADOW_NODE_RECALCULATE;
        }

        if( mDefinition->mCameraName != IdString() )
            mCamera = workspace->findCamera( mDefinition->mCameraName );
        else
            mCamera = defaultCamera;

        if( mDefinition->mLodCameraName != IdString() )
            mLodCamera = workspace->findCamera( mDefinition->mLodCameraName );
        else
            mLodCamera = mCamera;

        if( mDefinition->mCullCameraName != IdString() )
            mCullCamera = workspace->findCamera( mDefinition->mCullCameraName );
        else
            mCullCamera = mCamera;

        if( mDefinition->mPrePassMode == PrePassUse && mDefinition->mPrePassTexture != IdString() )
        {
            {
                const CompositorChannel *channel = parentNode->_getDefinedTexture(
                            mDefinition->mPrePassTexture );
                mPrePassTextures = &channel->textures;
            }

            if( mDefinition->mPrePassDepthTexture != IdString() )
            {
                const CompositorChannel *channel = parentNode->_getDefinedTexture(
                            mDefinition->mPrePassDepthTexture );
                mPrePassDepthTexture = &channel->textures;
                assert( mPrePassDepthTexture->size() == 1u );
            };

            if( mDefinition->mPrePassSsrTexture != IdString() )
            {
                const CompositorChannel *ssrChannel = parentNode->_getDefinedTexture(
                            mDefinition->mPrePassSsrTexture );
                mSsrTexture = &ssrChannel->textures;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    CompositorPassScene::~CompositorPassScene()
    {
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassScene::execute( const Camera *lodCamera )
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

        Camera const *usedLodCamera = mLodCamera;
        if( lodCamera && mDefinition->mLodCameraName == IdString() )
            usedLodCamera = lodCamera;

        //store the viewports current material scheme and use the one set in the scene pass def
        String oldViewportMatScheme = mViewport->getMaterialScheme();
        mViewport->setMaterialScheme(mDefinition->mMaterialScheme);

        //Let the code receive valid camera->getLastViewport() return values.
        mCamera->_notifyViewport( mViewport );
        const_cast<Camera*>(usedLodCamera)->_notifyViewport( mViewport ); //TODO: Ugly const_cast

        //Call beginUpdate if we're the first to use this RT
        if( mDefinition->mBeginRtUpdate )
            mTarget->_beginUpdate();

        SceneManager *sceneManager = mCamera->getSceneManager();

        const Quaternion oldCameraOrientation( mCamera->getOrientation() );

        //We have to do this first in case usedLodCamera == mCamera
        if( mDefinition->mCameraCubemapReorient )
        {
            uint32 sliceIdx = std::min<uint32>( mDefinition->getRtIndex(), 5 );
            mCamera->setOrientation( oldCameraOrientation * CubemapRotations[sliceIdx] );
        }

        if( mDefinition->mUpdateLodLists )
        {
            sceneManager->updateAllLods( usedLodCamera, mDefinition->mLodBias,
                                         mDefinition->mFirstRQ, mDefinition->mLastRQ );
        }

        //Passes belonging to a ShadowNode should not override their parent.
        CompositorShadowNode* shadowNode = ( mShadowNode && mShadowNode->getEnabled() ) ? mShadowNode : 0;
        if( mDefinition->mShadowNodeRecalculation != SHADOW_NODE_CASTER_PASS )
        {
            sceneManager->_setCurrentShadowNode( shadowNode, mDefinition->mShadowNodeRecalculation ==
                                                                                    SHADOW_NODE_REUSE );
        }

        mViewport->_setVisibilityMask( mDefinition->mVisibilityMask, mDefinition->mLightVisibilityMask );

        //Fire the listener in case it wants to change anything
        if( listener )
            listener->passPreExecute( this );

        if( mUpdateShadowNode && shadowNode )
        {
            //We need to prepare for rendering another RT (we broke the contiguous chain)
            mTarget->_endUpdate();

            //Save the value in case the listener changed it
            const uint32 oldVisibilityMask = mViewport->getVisibilityMask();
            const uint32 oldLightVisibilityMask = mViewport->getLightVisibilityMask();

            shadowNode->_update( mCamera, usedLodCamera, sceneManager );

            //ShadowNode passes may've overriden these settings.
            sceneManager->_setCurrentShadowNode( shadowNode, mDefinition->mShadowNodeRecalculation ==
                                                                                    SHADOW_NODE_REUSE );
            sceneManager->_setCompositorTarget( mTargetTexture );
            mViewport->_setVisibilityMask( oldVisibilityMask, oldLightVisibilityMask );
            mCamera->_notifyViewport( mViewport );

            //We need to restore the previous RT's update
            mTarget->_beginUpdate();
        }
        sceneManager->_setForwardPlusEnabledInPass( mDefinition->mEnableForwardPlus );
        sceneManager->_setPrePassMode( mDefinition->mPrePassMode, mPrePassTextures,
                                       mPrePassDepthTexture, mSsrTexture );
        sceneManager->_setCurrentCompositorPass( this );

        if( !mDefinition->mReuseCullData )
        {
            mTarget->_updateViewportCullPhase01( mViewport, mCullCamera, usedLodCamera,
                                                 mDefinition->mFirstRQ, mDefinition->mLastRQ );
        }

        executeResourceTransitions();

        mTarget->setFsaaResolveDirty();
        mTarget->_updateViewportRenderPhase02( mViewport, mCamera, usedLodCamera,
                                               mDefinition->mFirstRQ, mDefinition->mLastRQ, true );

        if( mDefinition->mCameraCubemapReorient )
        {
            //Restore orientation
            mCamera->setOrientation( oldCameraOrientation );
        }

        //restore viewport material scheme
        mViewport->setMaterialScheme(oldViewportMatScheme);

        sceneManager->_setPrePassMode( PrePassNone, 0, 0, 0 );
        sceneManager->_setCurrentCompositorPass( 0 );

        if( mDefinition->mShadowNodeRecalculation != SHADOW_NODE_CASTER_PASS )
        {
            sceneManager->_setCurrentShadowNode( 0, false );
            sceneManager->_setForwardPlusEnabledInPass( false );
        }

        if( listener )
            listener->passPosExecute( this );

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();

        profilingEnd();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassScene::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                                    ResourceAccessMap &uavsAccess,
                                                                    ResourceLayoutMap &resourcesLayout )
    {
        if( mShadowNode && mUpdateShadowNode )
        {
            mShadowNode->_placeBarriersAndEmulateUavExecution( boundUavs, uavsAccess,
                                                               resourcesLayout );
        }

        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
        const bool explicitApi = caps->hasCapability( RSC_EXPLICIT_API );

        //Check <anything> -> Texture (GBuffers)
        if( mPrePassTextures )
        {
            TextureVec::const_iterator itor = mPrePassTextures->begin();
            TextureVec::const_iterator end  = mPrePassTextures->end();

            while( itor != end )
            {
                const TexturePtr &texture = *itor;
                RenderTarget *renderTarget = texture->getBuffer()->getRenderTarget();

                ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( renderTarget );

                if( (currentLayout->second != ResourceLayout::Texture && explicitApi) ||
                        currentLayout->second == ResourceLayout::Uav )
                {
                    addResourceTransition( currentLayout,
                                           ResourceLayout::Texture,
                                           ReadBarrier::Texture );
                }

                ++itor;
            }
        }

        //Check <anything> -> DepthTexture (Depth Texture)
        if( mPrePassDepthTexture )
        {
            const TexturePtr &texture = (*mPrePassDepthTexture)[0];
            RenderTarget *renderTarget = texture->getBuffer()->getRenderTarget();

            ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( renderTarget );

            if( (currentLayout->second != ResourceLayout::Texture && explicitApi) ||
                currentLayout->second == ResourceLayout::Uav )
            {
                addResourceTransition( currentLayout,
                                       ResourceLayout::Texture,
                                       ReadBarrier::Texture );
            }
        }

        //Check <anything> -> Texture (SSR Texture)
        if( mSsrTexture )
        {
            TextureVec::const_iterator itor = mSsrTexture->begin();
            TextureVec::const_iterator end  = mSsrTexture->end();

            while( itor != end )
            {
                const TexturePtr &texture = *itor;
                RenderTarget *renderTarget = texture->getBuffer()->getRenderTarget();

                ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( renderTarget );

                if( (currentLayout->second != ResourceLayout::Texture && explicitApi) ||
                        currentLayout->second == ResourceLayout::Uav )
                {
                    addResourceTransition( currentLayout,
                                           ResourceLayout::Texture,
                                           ReadBarrier::Texture );
                }

                ++itor;
            }
        }

        CompositorPass::_placeBarriersAndEmulateUavExecution( boundUavs, uavsAccess, resourcesLayout );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassScene::notifyCleared(void)
    {
        mShadowNode = 0; //Allow changes to our shadow nodes too.
        CompositorPass::notifyCleared();
    }
}
