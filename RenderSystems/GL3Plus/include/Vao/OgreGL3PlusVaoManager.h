/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#ifndef _Ogre_GL3PlusVaoManager_H_
#define _Ogre_GL3PlusVaoManager_H_

#include "OgreGL3PlusPrerequisites.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    struct GLStagingBuffer
    {
        size_t  bufferSize;
        GLuint  bufferName;
        GLenum  target;         /// Either GL_COPY_WRITE_BUFFER or GL_COPY_READ_BUFFER

        struct Checkpoint
        {
            GLsync syncName;
            size_t offsetLocation;
        };

        typedef vector<Checkpoint>::type CheckpointVec;
        CheckpointVec checkpoints;

        GLStagingBuffer( size_t sizeBytes, GLenum _target ) :
            bufferSize( sizeBytes ),
            bufferName( 0 ),
            target( _target )
        {
        }
    };

    typedef vector<GLStagingBuffer>::type GLStagingBufferVec;

    class _OgreGL3PlusExport GL3PlusVaoManager : public VaoManager
    {
    protected:
        enum VboFlag
        {
            CPU_INACCESSIBLE,
            CPU_ACCESSIBLE,
            MAX_VBO_FLAG
        };

        struct Block
        {
            size_t offset;
            size_t size;

            Block( size_t _offset, size_t _size ) : offset( _offset ), size( _size ) {}
        };
        struct StrideChanger
        {
            size_t offsetAfterPadding;
            size_t paddedBytes;

            StrideChanger() : offsetAfterPadding( 0 ), paddedBytes( 0 ) {}
            StrideChanger( size_t _offsetAfterPadding, size_t _paddedBytes ) :
                offsetAfterPadding( _offsetAfterPadding ), paddedBytes( _paddedBytes ) {}

            bool operator () ( const StrideChanger &left, size_t right ) const
            {
                return left.offsetAfterPadding < right;
            }
            bool operator () ( size_t left, const StrideChanger &right ) const
            {
                return left < right.offsetAfterPadding;
            }
        };

        typedef vector<Block>::type BlockVec;
        typedef vector<StrideChanger>::type StrideChangerVec;

        struct Vbo
        {
            GLuint vboName;
            size_t sizeBytes;

            BlockVec            freeBlocks;
            StrideChangerVec    strideChangers;
        };

        typedef vector<Vbo>::type VboVec;
        VboVec  mVbos[MAX_VBO_FLAG];
        size_t  mDefaultPoolSize[MAX_VBO_FLAG];
        uint8   mDynamicBufferMultiplier;
        uint8   mDynamicBufferCurrentFrame;

        uint16  mMinStagingFrameLatency[2];
        uint16  mMaxStagingFrameLatency[2];
        GLStagingBufferVec mStagingBuffers[2];

        /// True if ARB_buffer_storage is supported (Persistent Mapping and immutable buffers)
        bool    mArbBufferStorage;

        /** Asks for allocating buffer space in a VBO (Vertex Buffer Object).
            If the VBO doesn't exist, all VBOs are full or can't fit this request,
            then a new VBO will be created.
        @remarks
            Can throw if out of video memory
        @param sizeBytes
            The requested size, in bytes.
        @param bytesPerElement
            The number of bytes per vertex or per index (i.e. 16-bit indices = 2).
            Cannot be 0.
        @param bufferType
            The type of buffer
        @param outVboIdx [out]
            The index to the mVbos.
        @param outBufferOffset [out]
            The offset in bytes at which the buffer data should be placed.
        */
        void allocateVbo( size_t sizeBytes, size_t bytesPerElement, BufferType bufferType,
                          size_t &outVboIdx, size_t &outBufferOffset );

        /** Deallocates a buffer allocated with @allocateVbo.
        @remarks
            All four parameters *must* match with the ones provided to or
            returned from allocateVbo, otherwise the behavior is undefined.
        @param vboIdx
            The index to the mVbos pool that was returned by allocateVbo
        @param bufferOffset
            The buffer offset that was returned by allocateVbo
        @param sizeBytes
            The sizeBytes parameter that was passed to allocateVbos.
        @param bufferType
            The type of buffer that was passed to allocateVbo.
        */
        void deallocateVbo( size_t vboIdx, size_t bufferOffset, size_t sizeBytes,
                            BufferType bufferType );

        /** Starts with "blockToMerge" and recursively merges all blocks which
            end up being affected by "blockToMerge".
        @remarks
            Example: if "blocks" contains A, C, E, and blockToMerge is B,
            then A will be merged with B, and a recursive call will merge AB with C
            So now "blocks" contains ABC and E.
        @param blockToMerge
            Iterator to a block to merge. Must belong to "blocks". Iterator may
            be invalidated after calling this function.
        @param blocks
            Vector of blocks where blockToMerge belongs to.
        */
        void mergeContiguousBlocks( BlockVec::iterator blockToMerge,
                                    BlockVec &blocks );

        virtual VertexBufferPacked* createVertexBufferImpl( size_t numElements,
                                                            uint32 bytesPerElement,
                                                            BufferType bufferType,
                                                            void *initialData, bool keepAsShadow,
                                                            const VertexElement2Vec &vertexElements );

        virtual IndexBufferPacked* createIndexBufferImpl( size_t numElements,
                                                          uint32 bytesPerElement,
                                                          BufferType bufferType,
                                                          void *initialData, bool keepAsShadow );
    public:

        bool supportsArbBufferStorage(void) const       { return mArbBufferStorage; }


        /** When uploading data to BT_DEFAULT buffers, we need to use intermediate buffers, as the
            CPU can't directly access them. These buffers are called "staging" buffers.
        @par
            Once the data is uploaded to the staging buffer, it may take a while until the GPU
            actually memcpy's GPU-to-GPU this data to the real buffer. If we try to use the
            staging buffer again while the GPU hasn't yet done this copy, we will hit a stall.
        @par
            The manager will keep a pool of staging buffers (of different sizes) and keep track
            of the last frame it was used in, so we return the oldest buffers. And across the oldest
            buffers, we will try to return the smallest one that can fit the request.
        @par
            Applications that perform a lot of uploads (i.e. lots of assets, streaming video) may
            want to increase the minimumFrameLatency to avoid stalling. Note that these settings
            can be changed at any time (i.e. during loading time, background loading, etc). But
            beware that if the pool was shrink and now needs to grow again, this can cause stalls.

        @remarks
            OpenGL provides the facilities to have knowledge of whether a buffer is done copying.
            However, for staging buffers this often is not worth the effort nor the overhead.
            If you really need to perform that many uploads, use persistent BT_DYNAMIC buffers.
            And for downloads @see AsyncTicket to perform async transfers.

        @param minimumFrameLatency
            Specifies the minimum frames that a buffer needs to have since last use before it can be
            used again. Increasing this value reduces the chance of stalling, but will consume
            more GPU memory.
        @param safeFrameLatency
            Number of frames at which a buffer should be considered consumed by the GPU.
            Increasing this number might reduce the chance of stalling, but can significantly
            increase the GPU memory consumption. Can't be lower than minimumFrameLatency
        */
        void setMaxUploadStagingBufferFrameLatency( uint16 minimumFrameLatency, uint16 safeFrameLatency );

        /// @See setMaxUploadStagingFrameLatency. This are the hints for downloads.
        void setMaxDownloadStagingBufferFrameLatency( uint16 minimumFrameLatency, uint16 safeFrameLatency );

        /** Retrieves a staging buffer for our use. We'll search for the oldest and smallest buffer
            we can find using the hints from @see setMaxUploadStagingBufferFrameLatency and
            @see setMaxDownloadStagingBufferFrameLatency. If none of the buffers is old enough
            (or all of them are too small), we'll create a new staging buffer.
        @remarks
            The returned buffer is already bound to GL_COPY_READ_BUFFER (forUpload = true) or to
            GL_COPY_WRITE_BUFFER (forUpload ? false)
        @param sizeBytes
            Minimum size, in bytes, of the staging buffer.
        @param forUpload
            True if it should be used to upload data to GPU, false to download.
        @return
            The staging buffer.
         */
        const GLStagingBuffer& getStagingBuffer( size_t sizeBytes, bool forUpload );

        uint8 getDynamicBufferCurrentFrame(void) const;
        uint8 getDynamicBufferMultiplier(void) const        { return mDynamicBufferMultiplier; }
    };
}

#endif
