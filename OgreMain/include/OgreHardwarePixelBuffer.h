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
#ifndef __HardwarePixelBuffer__
#define __HardwarePixelBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"
#include "OgrePixelFormat.h"
#include "OgreImage.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Specialisation of HardwareBuffer for a pixel buffer. The
    	HardwarePixelbuffer abstracts an 1D, 2D or 3D quantity of pixels
    	stored by the rendering API. The buffer can be located on the card
    	or in main memory depending on its usage. One mipmap level of a
    	texture is an example of a HardwarePixelBuffer.
    */
    class _OgreExport HardwarePixelBuffer : public HardwareBuffer
    {
    protected: 
        /// Extents
        uint32 mWidth, mHeight, mDepth;
        /// Pitches (offsets between rows and slices)
        size_t mRowPitch, mSlicePitch;
        /// Internal format
        PixelFormat mFormat;
        /// Currently locked region (local coords)
        PixelBox mCurrentLock;
		/// The current locked box of this surface (entire surface coords)
		Image::Box mLockedBox;

        
        /// Internal implementation of lock(), must be overridden in subclasses
        virtual PixelBox lockImpl(const Image::Box lockBox,  LockOptions options) = 0;

        /** Internal implementation of lock(), do not OVERRIDE or CALL this
            for HardwarePixelBuffer implementations, but override the previous method */
        virtual void* lockImpl(size_t offset, size_t length, LockOptions options);

		/** Notify TextureBuffer of destruction of render target.
			Called by RenderTexture when destroyed.
		*/
		virtual void _clearSliceRTT(size_t zoffset);
		friend class RenderTexture;
    public:
        /// Should be called by HardwareBufferManager
        HardwarePixelBuffer(uint32 mWidth, uint32 mHeight, uint32 mDepth,
                PixelFormat mFormat,
                HardwareBuffer::Usage usage, bool useSystemMemory, bool useShadowBuffer);
        ~HardwarePixelBuffer();

        /** Make every lock method from HardwareBuffer available.
        See http://www.research.att.com/~bs/bs_faq2.html#overloadderived
        */
        using HardwareBuffer::lock;	

        /** Lock the buffer for (potentially) reading / writing.
		    @param lockBox Region of the buffer to lock
		    @param options Locking options
		    @return PixelBox containing the locked region, the pitches and
		    	the pixel format
		*/
		virtual const PixelBox& lock(const Image::Box& lockBox, LockOptions options);
		/// @copydoc HardwareBuffer::lock
        virtual void* lock(size_t offset, size_t length, LockOptions options);

		/** Get the current locked region. This is the same value as returned
		    by lock(const Image::Box, LockOptions)
		    @return PixelBox containing the locked region
		*/        
        const PixelBox& getCurrentLock();
		
		/// @copydoc HardwareBuffer::readData
		virtual void readData(size_t offset, size_t length, void* pDest);
		/// @copydoc HardwareBuffer::writeData
		virtual void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
        
        /** Copies a box from another PixelBuffer to a region of the 
        	this PixelBuffer. 
			@param src		Source pixel buffer
        	@param srcBox	Image::Box describing the source region in src
        	@param dstBox	Image::Box describing the destination region in this buffer
			@remarks The source and destination regions dimensions don't have to match, in which
		   	case scaling is done. This scaling is generally done using a bilinear filter in hardware,
            but it is faster to pass the source image in the right dimensions.
			@note Only call this function when both  buffers are unlocked. 
         */        
        virtual void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);

		/** Convenience function that blits the entire source pixel buffer to this buffer. 
			If source and destination dimensions don't match, scaling is done.
			@param src		PixelBox containing the source pixels and format in memory
			@note Only call this function when the buffer is unlocked. 
		*/
		void blit(const HardwarePixelBufferSharedPtr &src); 
		
		/** Copies a region from normal memory to a region of this pixelbuffer. The source
			image can be in any pixel format supported by OGRE, and in any size. 
		   	@param src		PixelBox containing the source pixels and format in memory
		   	@param dstBox	Image::Box describing the destination region in this buffer
            @remarks The source and destination regions dimensions don't have to match, in which
            case scaling is done. This scaling is generally done using a bilinear filter in hardware,
            but it is faster to pass the source image in the right dimensions.
			@note Only call this function when the buffer is unlocked. 
		*/
		virtual void blitFromMemory(const PixelBox &src, const Image::Box &dstBox) = 0;
		
		/** Convenience function that blits a pixelbox from memory to the entire 
			buffer. The source image is scaled as needed.
			@param src		PixelBox containing the source pixels and format in memory
			@note Only call this function when the buffer is unlocked. 
		*/
		void blitFromMemory(const PixelBox &src)
		{
			blitFromMemory(src, Box(0,0,0,mWidth,mHeight,mDepth));
		}
		
		/** Copies a region of this pixelbuffer to normal memory.
		   	@param srcBox	Image::Box describing the source region of this buffer
		   	@param dst		PixelBox describing the destination pixels and format in memory
		   	@remarks The source and destination regions don't have to match, in which
		   	case scaling is done.
			@note Only call this function when the buffer is unlocked. 
		 */
		virtual void blitToMemory(const Image::Box &srcBox, const PixelBox &dst) = 0;

		/** Convenience function that blits this entire buffer to a pixelbox.
			The image is scaled as needed.
			@param dst		PixelBox describing the destination pixels and format in memory
			@note Only call this function when the buffer is unlocked. 
		*/
		void blitToMemory(const PixelBox &dst)
		{
			blitToMemory(Box(0,0,0,mWidth,mHeight,mDepth), dst);
		}
        
        /** Get a render target for this PixelBuffer, or a slice of it. The texture this
            was acquired from must have TU_RENDERTARGET set, otherwise it is possible to
            render to it and this method will throw an ERR_RENDERSYSTEM exception.
            @param slice    Which slice
            @return A pointer to the render target. This pointer has the lifespan of this
            PixelBuffer.
        */
        virtual RenderTexture *getRenderTarget(size_t slice=0);
        
        /// Gets the width of this buffer
        uint32 getWidth() const { return mWidth; }
        /// Gets the height of this buffer
        uint32 getHeight() const { return mHeight; }
        /// Gets the depth of this buffer
        uint32 getDepth() const { return mDepth; }
        /// Gets the native pixel format of this buffer
        PixelFormat getFormat() const { return mFormat; }
    };

    /** Shared pointer implementation used to share pixel buffers. */
    class _OgreExport HardwarePixelBufferSharedPtr : public SharedPtr<HardwarePixelBuffer>
    {
    public:
        HardwarePixelBufferSharedPtr() : SharedPtr<HardwarePixelBuffer>() {}
        explicit HardwarePixelBufferSharedPtr(HardwarePixelBuffer* buf);


    };

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif

