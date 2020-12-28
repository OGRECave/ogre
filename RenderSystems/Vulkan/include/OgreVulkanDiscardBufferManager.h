/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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

#ifndef _OgreVulkanDiscardBufferManager_H_
#define _OgreVulkanDiscardBufferManager_H_

#include "OgreVulkanPrerequisites.h"

#include "Vao/OgreVulkanVaoManager.h"

namespace Ogre
{
    class VulkanDiscardBuffer;
    typedef vector<VulkanDiscardBuffer *>::type VulkanDiscardBufferVec;

    /// Vulkan doesn't support "DISCARD" like D3D9/D3D11 (and OpenGL but often it's broken)
    /// where we requested to map a write-only buffer and the API would discard the previous
    /// contents (thus allowing us to avoid a stall until the GPU is done with the region)
    ///
    /// We need Discard for the v1 interfaces. So we need to emulate it.
    /// This class does exactly this.
    class _OgreVulkanExport VulkanDiscardBufferManager : public BufferAlloc
    {
    public:
        struct UnsafeBlock : public VulkanVaoManager::Block
        {
            uint32 lastFrameUsed;

            UnsafeBlock( size_t _offset, size_t _size, uint32 _lastFrameUsed ) :
                VulkanVaoManager::Block( _offset, _size ),
                lastFrameUsed( _lastFrameUsed )
            {
            }

            bool operator<( const UnsafeBlock &other ) const
            {
                return this->lastFrameUsed < other.lastFrameUsed;
            }
        };
        typedef vector<UnsafeBlock>::type UnsafeBlockVec;

    private:
        VulkanRawBuffer mBuffer;
        VulkanDevice *mDevice;
        VaoManager *mVaoManager;
        VulkanVaoManager::BlockVec mFreeBlocks;

        UnsafeBlockVec mUnsafeBlocks;

        VulkanDiscardBufferVec mDiscardBuffers;

        /** Moves our current mBuffer into a new one (bigger one). Used when we've ran
            out of usable space. This operation can be slow and will increase GPU memory
            consumption. Until the old buffer stops being used, memory usage will stay
            higher than normal.
        @param extraBytes
            Extra bytes are required. We may grow bigger than that, but no less.
        @param forDiscardBuffer
            Optional. The block owned by the input won't be copied from old buffer
            into the new one, so that it can be reused. Can be null.
            @see _getBlock's usage code for reference.
        */
        void growToFit( size_t extraBytes, VulkanDiscardBuffer *forDiscardBuffer );

        /// Puts unsafe blocks that are now safe back to the free blocks pool.
        void updateUnsafeBlocks( void );

    public:
        VulkanDiscardBufferManager( VulkanDevice *device, VaoManager *vaoManager );
        ~VulkanDiscardBufferManager();

        void _notifyDeviceStalled( void );

        /** For internal use. Retrieves a fresh new block. The old block will be returned
            to a pool until it's safe to reuse again. Used by the DiscardBuffer when
            map( DISCARD ) is called.
        @param discardBuffer
            Buffer to assign a new region.
        */
        void _getBlock( VulkanDiscardBuffer *discardBuffer );

        /** Creates a buffer that supports discarding to hold the required size.
        @param bufferSize
            Requested size in bytes.
        @param alignment
            Alignment requirements.
        @return
            A new VulkanDiscardBuffer
        */
        VulkanDiscardBuffer *createDiscardBuffer( size_t bufferSize, uint16 alignment );

        /** Destroys an existing VulkanDiscardBuffer, releasing its memory.
        @param discardBuffer
            VulkanDiscardBuffer to destroy
        */
        void destroyDiscardBuffer( VulkanDiscardBuffer *discardBuffer );

        VulkanDevice *getDevice( void ) const { return mDevice; }
        VaoManager *getVaoManager( void ) const { return mVaoManager; }

        VulkanRawBuffer &getBuffer( void ) { return mBuffer; }
    };

    class _OgreVulkanExport VulkanDiscardBuffer : public BufferAlloc
    {
        friend class VulkanDiscardBufferManager;

        VkBuffer mBuffer;
        VulkanDevice *mDevice;
        size_t mBlockPrePadding;
        size_t mBufferOffset;
        size_t mBufferSize;

        uint16 mAlignment;
        uint32 mLastFrameUsed;

        VaoManager *mVaoManager;
        VulkanDiscardBufferManager *mOwner;

    public:
        VulkanDiscardBuffer( size_t bufferSize, uint16 alignment, VaoManager *vaoManager,
                             VulkanDevice *device, VulkanDiscardBufferManager *owner );

        /** Returns a pointer that maps to the beginning of this buffer to begin writing.
        @param noOverwrite
            When true, noOverwrite is slow
        @return
        */
        void *map( bool noOverwrite );
        void unmap( void );

        uint16 getAlignment( void ) const { return mAlignment; }
        /// Size of the buffer, may be bigger than requested due to 4-byte alignment required by Vulkan.
        size_t getSizeBytes( void ) const { return mBufferSize; }

        size_t getOffset( void ) const { return mBufferOffset; }
        /** Returns the actual API buffer, but first sets mLastFrameUsed as we
            assume you're calling this function to use the buffer in the GPU.
        @param outOffset
            Out. Guaranteed to be written. Used to point to the start
            of our data in the internal ring buffer we've allocated.
        @return
            The MTLBuffer in question.
        */
        VkBuffer getBufferName( size_t &outOffset );

        /// For internal use by VulkanDiscardBufferManager
        size_t getBlockStart( void ) const { return mBufferOffset - mBlockPrePadding; }
        size_t getBlockSize( void ) const { return mBufferSize + mBlockPrePadding; }

        VulkanDiscardBufferManager *getOwner( void ) { return mOwner; }
    };
}  // namespace Ogre

#endif
