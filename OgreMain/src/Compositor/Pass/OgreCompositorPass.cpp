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

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorChannel.h"

#include "OgreRenderTarget.h"
#include "OgreViewport.h"

namespace Ogre
{
	CompositorPass::CompositorPass( const CompositorPassDef *definition, RenderTarget *target,
									CompositorNode *parentNode ) :
			mDefinition( definition ),
			mTarget( target ),
			mViewport( 0 ),
			mNumPassesLeft( definition->mNumInitialPasses ),
			mParentNode( parentNode )
	{
		assert( definition->mNumInitialPasses && "Definition is broken, pass will never execute!" );

		const Real EPSILON = 1e-6f;

		const unsigned short numViewports = mTarget->getNumViewports();
		for( unsigned short i=0; i<numViewports && !mViewport; ++i )
		{
			Viewport *vp = mTarget->getViewport(i);
			if( Math::Abs( vp->getLeft() - mDefinition->mVpLeft )	< EPSILON &&
				Math::Abs( vp->getTop() - mDefinition->mVpTop )		< EPSILON &&
				Math::Abs( vp->getWidth() - mDefinition->mVpWidth ) < EPSILON &&
				Math::Abs( vp->getHeight() - mDefinition->mVpHeight )<EPSILON &&
				vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
			{
				mViewport = vp;
			}
		}

		if( !mViewport )
		{
			mViewport = mTarget->addViewport( mDefinition->mVpLeft, mDefinition->mVpTop,
												mDefinition->mVpWidth, mDefinition->mVpHeight );
			mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
		}
	}
	//-----------------------------------------------------------------------------------
	CompositorPass::~CompositorPass()
	{
	}
	//-----------------------------------------------------------------------------------
	void CompositorPass::notifyRecreated( const CompositorChannel &oldChannel,
											const CompositorChannel &newChannel )
	{
		const Real EPSILON = 1e-6f;

		if( mTarget == oldChannel.target )
		{
			mTarget = newChannel.target;

			mNumPassesLeft = mDefinition->mNumInitialPasses;

			const unsigned short numViewports = mTarget->getNumViewports();
			for( unsigned short i=0; i<numViewports && !mViewport; ++i )
			{
				Viewport *vp = mTarget->getViewport(i);
				if( Math::Abs( vp->getLeft() - mDefinition->mVpLeft )	< EPSILON &&
					Math::Abs( vp->getTop() - mDefinition->mVpTop )		< EPSILON &&
					Math::Abs( vp->getWidth() - mDefinition->mVpWidth ) < EPSILON &&
					Math::Abs( vp->getHeight() - mDefinition->mVpHeight )<EPSILON &&
					vp->getOverlaysEnabled() == mDefinition->mIncludeOverlays )
				{
					mViewport = vp;
				}
			}

			if( !mViewport )
			{
				mViewport = mTarget->addViewport( mDefinition->mVpLeft, mDefinition->mVpTop,
													mDefinition->mVpWidth, mDefinition->mVpHeight );
				mViewport->setOverlaysEnabled( mDefinition->mIncludeOverlays );
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void CompositorPass::notifyDestroyed( const CompositorChannel &channel )
	{
		if( mTarget == channel.target )
			mTarget = 0;
	}
	//-----------------------------------------------------------------------------------
	void CompositorPass::notifyCleared(void)
	{
		mTarget = 0;
	}
}
