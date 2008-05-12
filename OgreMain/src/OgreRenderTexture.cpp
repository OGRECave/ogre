/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreRenderTexture.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreImageCodec.h"

namespace Ogre
{

    //-----------------------------------------------------------------------------
	RenderTexture::RenderTexture(HardwarePixelBuffer *buffer, size_t zoffset):
		mBuffer(buffer), mZOffset(zoffset)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
		mWidth = static_cast<unsigned int>(mBuffer->getWidth());
		mHeight = static_cast<unsigned int>(mBuffer->getHeight());
        mColourDepth = static_cast<unsigned int>(
			Ogre::PixelUtil::getNumElemBits(mBuffer->getFormat()));
    }
    RenderTexture::~RenderTexture()
    {
		mBuffer->_clearSliceRTT(0);
    }

	void RenderTexture::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
		if (buffer == FB_AUTO) buffer = FB_FRONT;
		if (buffer != FB_FRONT)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid buffer.",
						"RenderTexture::copyContentsToMemory" );
		}

		mBuffer->blitToMemory(dst);
	}
	//---------------------------------------------------------------------
	PixelFormat RenderTexture::suggestPixelFormat() const
	{
		return mBuffer->getFormat();
	}
	//-----------------------------------------------------------------------------
	MultiRenderTarget::MultiRenderTarget(const String &name)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
		mName = name;
		/// Width and height is unknown with no targets attached
		mWidth = mHeight = 0;
    }
	//-----------------------------------------------------------------------------
	void MultiRenderTarget::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Cannot get MultiRenderTargets pixels",
					"MultiRenderTarget::copyContentsToMemory");
	}
}
