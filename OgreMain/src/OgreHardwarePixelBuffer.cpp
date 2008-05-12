/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreException.h"

namespace Ogre 
{
  
    //-----------------------------------------------------------------------------    
    HardwarePixelBuffer::HardwarePixelBuffer(size_t width, size_t height, size_t depth,
            PixelFormat format,
            HardwareBuffer::Usage usage, bool useSystemMemory, bool useShadowBuffer):
        HardwareBuffer(usage, useSystemMemory, useShadowBuffer),
        mWidth(width), mHeight(height), mDepth(depth),
        mFormat(format)
    {
        // Default
        mRowPitch = mWidth;
        mSlicePitch = mHeight*mWidth;
		mSizeInBytes = mHeight*mWidth*PixelUtil::getNumElemBytes(mFormat);
    }
    
    //-----------------------------------------------------------------------------    
    HardwarePixelBuffer::~HardwarePixelBuffer()
    {
    }
    
    //-----------------------------------------------------------------------------    
    void* HardwarePixelBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        assert(!isLocked() && "Cannot lock this buffer, it is already locked!");
        assert(offset == 0 && length == mSizeInBytes && "Cannot lock memory region, most lock box or entire buffer");
        
        Image::Box myBox(0, 0, 0, mWidth, mHeight, mDepth);
        const PixelBox &rv = lock(myBox, options);
        return rv.data;
    }
    
    //-----------------------------------------------------------------------------    
    const PixelBox& HardwarePixelBuffer::lock(const Image::Box& lockBox, LockOptions options)
    {
        if (mUseShadowBuffer)
        {
            if (options != HBL_READ_ONLY)
            {
                // we have to assume a read / write lock so we use the shadow buffer
                // and tag for sync on unlock()
                mShadowUpdated = true;
            }

            mCurrentLock = static_cast<HardwarePixelBuffer*>(mpShadowBuffer)->lock(lockBox, options);
        }
        else
        {
            // Lock the real buffer if there is no shadow buffer 
            mCurrentLock = lockImpl(lockBox, options);
            mIsLocked = true;
        }

        return mCurrentLock;
    }
    
    //-----------------------------------------------------------------------------    
    const PixelBox& HardwarePixelBuffer::getCurrentLock() 
	{ 
        assert(isLocked() && "Cannot get current lock: buffer not locked");
        
        return mCurrentLock; 
    }
    
    //-----------------------------------------------------------------------------    
    /// Internal implementation of lock()
    void* HardwarePixelBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "lockImpl(offset,length) is not valid for PixelBuffers and should never be called",
            "HardwarePixelBuffer::lockImpl");
    }

    //-----------------------------------------------------------------------------    

    void HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox)
	{
		if(isLocked() || src->isLocked())
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"Source and destination buffer may not be locked!",
				"HardwarePixelBuffer::blit");
		}
		if(src.getPointer() == this)
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                "Source must not be the same object",
                "HardwarePixelBuffer::blit" ) ;
		}
		const PixelBox &srclock = src->lock(srcBox, HBL_READ_ONLY);

		LockOptions method = HBL_NORMAL;
		if(dstBox.left == 0 && dstBox.top == 0 && dstBox.front == 0 &&
		   dstBox.right == mWidth && dstBox.bottom == mHeight &&
		   dstBox.back == mDepth)
			// Entire buffer -- we can discard the previous contents
			method = HBL_DISCARD;
			
		const PixelBox &dstlock = lock(dstBox, method);
		if(dstlock.getWidth() != srclock.getWidth() ||
        	dstlock.getHeight() != srclock.getHeight() ||
        	dstlock.getDepth() != srclock.getDepth())
		{
			// Scaling desired
			Image::scale(srclock, dstlock);
		}
		else
		{
			// No scaling needed
			PixelUtil::bulkPixelConversion(srclock, dstlock);
		}

		unlock();
		src->unlock();
	}
    //-----------------------------------------------------------------------------       
    void HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &src)
    {
        blit(src, 
            Box(0,0,0,src->getWidth(),src->getHeight(),src->getDepth()), 
            Box(0,0,0,mWidth,mHeight,mDepth)
        );
    }
    //-----------------------------------------------------------------------------    
	void HardwarePixelBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		// TODO
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Reading a byte range is not implemented. Use blitToMemory.",
				"HardwarePixelBuffer::readData");
	}
	//-----------------------------------------------------------------------------    

	void HardwarePixelBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		// TODO
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Writing a byte range is not implemented. Use blitFromMemory.",
				"HardwarePixelBuffer::writeData");
	}
    //-----------------------------------------------------------------------------    
    
    RenderTexture *HardwarePixelBuffer::getRenderTarget(size_t)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Not yet implemented for this rendersystem.",
				"HardwarePixelBuffer::getRenderTarget");
    }

    //-----------------------------------------------------------------------------    
    
    HardwarePixelBufferSharedPtr::HardwarePixelBufferSharedPtr(HardwarePixelBuffer* buf)
        : SharedPtr<HardwarePixelBuffer>(buf)
    {

    }   
	//-----------------------------------------------------------------------------    

	void HardwarePixelBuffer::_clearSliceRTT(size_t zoffset)
	{
	}

};
