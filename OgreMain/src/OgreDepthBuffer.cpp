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
#include "OgreDepthBuffer.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
	DepthBuffer::DepthBuffer( uint16 poolId, uint16 bitDepth, uint32 width, uint32 height,
							  uint32 fsaa, const String &fsaaHint, bool manual ) :
				mPoolId(poolId),
				mBitDepth(bitDepth),
				mWidth(width),
				mHeight(height),
				mFsaa(fsaa),
				mFsaaHint(fsaaHint),
				mManual(manual)
	{
	}

	DepthBuffer::~DepthBuffer()
	{
		detachFromAllRenderTargets();
	}

	void DepthBuffer::_setPoolId( uint16 poolId )
	{
		//Change the pool Id
		mPoolId = poolId;

		//Render Targets were attached to us, but they have a different pool Id,
		//so detach ourselves from them
		detachFromAllRenderTargets();
	}
	//-----------------------------------------------------------------------
	uint16 DepthBuffer::getPoolId() const
	{
		return mPoolId;
	}
	//-----------------------------------------------------------------------
	uint16 DepthBuffer::getBitDepth() const
	{
		return mBitDepth;
	}
	//-----------------------------------------------------------------------
	uint32 DepthBuffer::getWidth() const
	{
		return mWidth;
	}
	//----------------------------------------------------------------------
	uint32 DepthBuffer::getHeight() const
	{
		return mHeight;
	}
	//-----------------------------------------------------------------------
	uint32 DepthBuffer::getFsaa() const
	{
		return mFsaa;
	}
	//-----------------------------------------------------------------------
	const String& DepthBuffer::getFsaaHint() const
	{
		return mFsaaHint;
	}
	//-----------------------------------------------------------------------
	bool DepthBuffer::isManual() const
	{
		return mManual;
	}
	//-----------------------------------------------------------------------
	bool DepthBuffer::isCompatible( RenderTarget *renderTarget ) const
	{
		if( this->getWidth() >= renderTarget->getWidth() &&
			this->getHeight() >= renderTarget->getHeight() &&
			this->getFsaa() == renderTarget->getFSAA() )
		{
			return true;
		}

		return false;
	}
	//-----------------------------------------------------------------------
	void DepthBuffer::_notifyRenderTargetAttached( RenderTarget *renderTarget )
	{
		assert( mAttachedRenderTargets.find( renderTarget ) == mAttachedRenderTargets.end() );

		mAttachedRenderTargets.insert( renderTarget );
	}
	//-----------------------------------------------------------------------
	void DepthBuffer::_notifyRenderTargetDetached( RenderTarget *renderTarget )
	{
		RenderTargetSet::iterator itor = mAttachedRenderTargets.find( renderTarget );
		assert( itor != mAttachedRenderTargets.end() );

		mAttachedRenderTargets.erase( itor );
	}
	//-----------------------------------------------------------------------
	void DepthBuffer::detachFromAllRenderTargets()
	{
		RenderTargetSet::const_iterator itor = mAttachedRenderTargets.begin();
		RenderTargetSet::const_iterator end  = mAttachedRenderTargets.end();
		while( itor != end )
		{
			//If we call, detachDepthBuffer, we'll invalidate the iterators
			(*itor)->_detachDepthBuffer();
			++itor;
		}

		mAttachedRenderTargets.clear();
	}
}
