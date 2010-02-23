/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2010 Torus Knot Software Ltd

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
#include "OgreD3D10DepthBuffer.h"
#include "OgreD3D10HardwarePixelBuffer.h"
#include "OgreD3D10Texture.h"

namespace Ogre
{
	D3D10DepthBuffer::D3D10DepthBuffer( uint16 poolId,
										ID3D10DepthStencilView *depthBufferView,
										uint32 width, uint32 height,
										uint32 fsaa, uint32 multiSampleQuality, bool isManual ) :
				DepthBuffer( poolId, 0, width, height, fsaa, "", isManual ),
				mDepthStencilView( depthBufferView ),
				mMultiSampleQuality( multiSampleQuality )
	{
		/*D3D10_DEPTH_STENCIL_VIEW_DESC pDesc;
		mDepthStencilView->GetDesc( &pDesc );
		switch( pDesc.Format )
		{
		case :
			mBitDepth = 16;
			break;
		case :
			mBitDepth = 32;
			break;
		}*/
		//TODO: Do not just assume it's 32-bit
		mBitDepth = 32;
	}

	D3D10DepthBuffer::~D3D10DepthBuffer()
	{
		if( !mManual )
			mDepthStencilView->Release();
		mDepthStencilView = 0;
	}
	//---------------------------------------------------------------------
	bool D3D10DepthBuffer::isCompatible( RenderTarget *renderTarget ) const
	{
		//TODO:
		//Urgh! This bit of "isTexture" is a bit ugly and potentially unsafe
		bool isTexture = true;
		renderTarget->getCustomAttribute( "isTexture", &isTexture );

		ID3D10Texture2D *d3d10texture;
		if( isTexture )
		{
			D3D10HardwarePixelBuffer *pBuffer;
			renderTarget->getCustomAttribute( "BUFFER", &pBuffer );
			d3d10texture = static_cast<ID3D10Texture2D*>( pBuffer->getParentTexture()->getTextureResource() );
		}
		else
		{
			renderTarget->getCustomAttribute( "ID3D10Texture2D", &d3d10texture );
		}

		D3D10_TEXTURE2D_DESC BBDesc;
		d3d10texture->GetDesc( &BBDesc );

		//TODO: Needs to check format too!
		if( mFsaa == BBDesc.SampleDesc.Count &&
			mMultiSampleQuality == BBDesc.SampleDesc.Quality &&
			this->getWidth() >= renderTarget->getWidth() &&
			this->getHeight() >= renderTarget->getHeight() )
		{
			return true;
		}

		return false;
	}
	//---------------------------------------------------------------------
	ID3D10DepthStencilView* D3D10DepthBuffer::getDepthStencilView() const
	{
		return mDepthStencilView;
	}
}
