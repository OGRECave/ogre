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

namespace Ogre
{
    const Quaternion CubemapRotations[6] =
    {
        Quaternion( Degree(-90 ), Vector3::UNIT_Y ),        //+X
        Quaternion( Degree( 90 ), Vector3::UNIT_Y ),        //-X
        Quaternion( Degree( 90 ), Vector3::UNIT_X ),        //+Y
        Quaternion( Degree(-90 ), Vector3::UNIT_X ),        //-Y
        Quaternion::IDENTITY,                               //+Z
        Quaternion( Degree(180 ), Vector3::UNIT_Y )         //-Z
    };

    CompositorPassScene::CompositorPassScene( const CompositorPassSceneDef *definition,
                                                Camera *defaultCamera, const CompositorChannel &target,
                                                CompositorNode *parentNode ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition ),
                mShadowNode( 0 ),
                mCamera( 0 ),
                mLodCamera( 0 ),
                mUpdateShadowNode( false )
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
            mLodCamera = workspace->findCamera( mDefinition->mCameraName );
        else
            mLodCamera = mCamera;
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

        Camera const *usedLodCamera = mLodCamera;
        if( lodCamera && mCamera == mLodCamera )
            usedLodCamera = lodCamera;

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
        if( mDefinition->mShadowNodeRecalculation != SHADOW_NODE_CASTER_PASS )
            sceneManager->_setCurrentShadowNode( mShadowNode );

        mViewport->_setVisibilityMask( mDefinition->mVisibilityMask );

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        mTarget->_updateViewportCullPhase01( mViewport, mCamera, usedLodCamera,
                                             mDefinition->mFirstRQ, mDefinition->mLastRQ );

        if( mShadowNode && mUpdateShadowNode )
        {
            //We need to prepare for rendering another RT (we broke the contiguous chain)
            mTarget->_endUpdate();

            sceneManager->_swapVisibleObjectsForShadowMapping();
            mShadowNode->_update( mCamera, usedLodCamera, sceneManager );
            sceneManager->_swapVisibleObjectsForShadowMapping();

            //ShadowNode passes may've overriden this setting.
            sceneManager->_setCurrentShadowNode( mShadowNode );

            //We need to restore the previous RT's update
            mTarget->_beginUpdate();
        }

        mTarget->setFsaaResolveDirty();
        mTarget->_updateViewportRenderPhase02( mViewport, mCamera, usedLodCamera,
                                               mDefinition->mFirstRQ, mDefinition->mLastRQ, true );

        if( mDefinition->mCameraCubemapReorient )
        {
            //Restore orientation
            mCamera->setOrientation( oldCameraOrientation );
        }

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassScene::notifyCleared(void)
    {
        mShadowNode = 0; //Allow changes to our shadow nodes too.
        CompositorPass::notifyCleared();
    }
}
