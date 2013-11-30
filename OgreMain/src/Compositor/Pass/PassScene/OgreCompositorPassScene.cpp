/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreSceneManager.h"

namespace Ogre
{
	CompositorPassScene::CompositorPassScene( const CompositorPassSceneDef *definition,
												Camera *defaultCamera,
												CompositorWorkspace *workspace,
												RenderTarget *target ) :
				CompositorPass( definition, target ),
				mDefinition( definition ),
				mShadowNode( 0 ),
				mCamera( 0 ),
				mLodCamera( 0 ),
				mUpdateShadowNode( false )
	{
		if( mDefinition->mShadowNode != IdString() )
		{
			bool shadowNodeCreated;
			mShadowNode	= workspace->findOrCreateShadowNode( mDefinition->mShadowNode,
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
		/*if( mShadowNode && mUpdateShadowNode )
		{
			//We need to prepare for rendering another RT (we broke the contiguous chain)
			if( !mDefinition->mEndRtUpdate )
				mTarget->_endUpdate();

			//mShadowNode->_update();

			//We need to restore the previous RT's update
			if( !mDefinition->mBeginRtUpdate )
				mTarget->_beginUpdate();
		}*/

		Camera const *usedLodCamera = mLodCamera;
		if( lodCamera && mCamera == mLodCamera )
			usedLodCamera = lodCamera;

		//Call beginUpdate if we're the first to use this RT
		if( mDefinition->mBeginRtUpdate )
			mTarget->_beginUpdate();

		SceneManager *sceneManager = mCamera->getSceneManager();

		if( mDefinition->mUpdateLodLists )
		{
			sceneManager->updateAllLods( usedLodCamera, mDefinition->mLodBias,
										 mDefinition->mFirstRQ, mDefinition->mLastRQ );
		}

		//Passes belonging to a ShadowNode should not override their parent.
		if( mDefinition->mShadowNodeRecalculation != SHADOW_NODE_CASTER_PASS )
			sceneManager->_setCurrentShadowNode( mShadowNode );

		mViewport->_setVisibilityMask( mDefinition->mVisibilityMask );

		mTarget->_updateViewportCullPhase01( mViewport, mCamera, usedLodCamera,
											 mDefinition->mFirstRQ, mDefinition->mLastRQ );

		if( mShadowNode && mUpdateShadowNode )
		{
			//We need to prepare for rendering another RT (we broke the contiguous chain)
			mTarget->_endUpdate();

			sceneManager->_swapVisibleObjectsForShadowMapping();
			mShadowNode->_update( mCamera, usedLodCamera );
			sceneManager->_swapVisibleObjectsForShadowMapping();

			//ShadowNode passes may've overriden this setting.
			sceneManager->_setCurrentShadowNode( mShadowNode );

			//We need to restore the previous RT's update
			mTarget->_beginUpdate();
		}

		mTarget->setFsaaResolveDirty();
		mTarget->_updateViewportRenderPhase02( mViewport, mCamera, usedLodCamera,
											   mDefinition->mFirstRQ, mDefinition->mLastRQ, true );

		//Call endUpdate if we're the last pass in a row to use this RT
		if( mDefinition->mEndRtUpdate )
			mTarget->_endUpdate();
	}
}
