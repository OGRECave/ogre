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

#ifndef _Ogre_StagingBuffer_H_
#define _Ogre_StagingBuffer_H_

#include "OgrePrerequisites.h"
#include "OgreBufferPacked.h"

namespace Ogre
{
    enum StagingStallType
    {
        /// Next map will not stall.
        STALL_NONE,

        /// Next map call will cause a stall. We can't predict how long, but
        /// on average should be small. You should consider doing something
        /// else then try again.
        STALL_PARTIAL,

        /// The whole pipeline is brought to a stop. We have to wait for the GPU
        /// to finish all operations issued so far. This can be very expensive.
        /// Grab a different StagingBuffer.
        STALL_FULL,

        NUM_STALL_TYPES
    };

    /** A staging buffer is a buffer that resides on the GPU and be written to/from both CPU & GPU
        However the access in both cases is limited. GPUs can only copy (i.e. memcpy) to another
        real buffer (can't be used directly as i.e. texture or vertex buffer) and CPUs can only
        map it.
        In other words, a staging buffer is an intermediate buffer to transfer data between
        CPU & GPU
    @remarks
        The interface is very similar to BufferPacked (@see BufferPacked) but it's not the same.
        A staging buffer can only map/unmap, and it's mapping operations don't accept an "elementStart"
        argument. Staging buffers always deal with bytes, not elements or bytes per element.
        RenderSystem implementations also synchronize the mapping differently to avoid excessive
        memory waste or stalling.
    @par
        Internally, the staging buffer will have a maximum size and use it as a ring buffer. i.e. if
        you have a 32MB staging buffer, you can upload 4 meshes of 8 MBs each. On the 5th one the
        system will first check if the first mesh has already been copied, otherwise it will stall.
        Trying to map more bytes than the total size is an error.
    @par
        Staging buffers can't be persistently mapped, since it beats the point.
    */
    class _OgreExport StagingBuffer : public StagingBufferAlloc
    {
    public:
        struct Destination
        {
            /// Buffer where the contents of the staging buffer will be copied to after unmapping
            BufferPacked *destination;

            /// The offset in the destination buffer to copy to.
            size_t dstOffset;

            /// 0-started offset relative to the mapped region
            size_t srcOffset;

            /// Amount of bytes to copy
            size_t length;

            Destination( BufferPacked *_destination, size_t _dstOffset,
                         size_t _srcOffset, size_t _length ) :
                destination( _destination ),
                dstOffset( _dstOffset ),
                srcOffset( _srcOffset ),
                length( _length )
            {
            }
        };

        typedef vector<Destination>::type DestinationVec;

    protected:
        struct Fence
        {
            size_t  start;
            size_t  end;

            Fence( size_t _start, size_t _end ) :
                start( _start ), end( _end )
            {
                assert( _start <= _end );
            }

            bool overlaps( const Fence &fence ) const
            {
                return !( fence.end <= this->start || fence.start >= this->end );
            }

            size_t length(void) const { return end - start; }
        };

        typedef vector<Fence>::type FenceVec;

        size_t  mInternalBufferStart;
        size_t  mSizeBytes;
        bool    mUploadOnly;

        VaoManager      *mVaoManager;

        MappingState    mMappingState;
        size_t          mMappingStart;
        size_t          mMappingCount;

        /// Manual reference count. Note on creation it starts already at 1.
        int16           mRefCount;
        uint32          mUnfenceTimeThreshold;
        uint32          mLifetimeThreshold;
        unsigned long   mLastUsedTimestamp;

        //------------------------------------
        // Begin used for downloads
        //------------------------------------
        FenceVec mAvailableDownloadRegions;
        //------------------------------------
        // End used for downloads
        //------------------------------------

        void mapChecks( size_t sizeBytes );

        virtual const void* _mapForReadImpl( size_t offset, size_t sizeBytes ) = 0;

        virtual void* mapImpl( size_t sizeBytes ) = 0;
        virtual void unmapImpl( const Destination *destinations, size_t numDestinations ) = 0;

        /// Returns the offset that can hold length bytes
        size_t getFreeDownloadRegion( size_t length );

        /** Starts with "blockToMerge" and recursively merges all blocks which
            end up being affected by "blockToMerge".
        @remarks
            Example: if "blocks" contains C, A, E, B and blockToMerge is B,
            then A will be merged with B, then AB will be merged with C
            So now "blocks" contains ABC and E.
        @param blockToMerge
            Iterator to a block to merge. Must belong to "blocks". Iterator may
            be invalidated after calling this function.
        @param blocks
            Vector of blocks where blockToMerge belongs to.
        */
        static void mergeContiguousBlocks( FenceVec::iterator blockToMerge,
                                           FenceVec &blocks );

    public:
        StagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                       VaoManager *vaoManager, bool uploadOnly );
        virtual ~StagingBuffer();

        /// When true, this buffer can only be used for uploading to GPU.
        /// When false, can only be used for downloading from GPU
        bool getUploadOnly(void) const                  { return mUploadOnly; }

        MappingState getMappingState(void) const        { return mMappingState; }

        /** Returns true if our next call to @map() with the same parameters will stall.
            @See StagingStallType
        @remarks
            Not all RenderSystems can accurately give this information and will always
            return STALL_PARTIAL (i.e. GLES2)
            The chances of getting a STALL_FULL get higher as sizeBytes gets closer
            to this->getMaxSize()
            mUploadOnly must be false.
            It is the counter side of @see canDownload
        */
        virtual StagingStallType uploadWillStall( size_t sizeBytes );

        /** Checks if this staging buffer has enough free space to use _asyncDownload.
            Otherwise such function would raise an exception.
        @remarks
            mUploadOnly must be true.
            It is the counter side of @see uploadWillStall
        @param length
            The size in bytes that need to be downloaded.
        */
        virtual bool canDownload( size_t length ) const;

        /** Copies the GPU data in BufferPacked to the StagingBuffer so that it can be
            later read by the CPU using an AsyncTicket. @see AsyncTicket.
        @remarks
            For internal use.
            May throw if it can't handle the request (i.e. requested size is too big,
            or too many _asyncDownload operations are pending until calling _mapForRead)
            @see canDownload
            mUploadOnly must be true.
        @param source
            The buffer to copy from.
        @param srcOffset
            The offset, in bytes, of the buffer to copy from.
        @param srcLength
            The size in bytes, of the data to transfer to this staging buffer.
        @return
            The offset in bytes that will be used by @see _mapForRead
        */
        virtual size_t _asyncDownload( BufferPacked *source, size_t srcOffset, size_t srcLength ) = 0;

        /// Releases memory assigned to a download that hasn't been mapped yet,
        /// to make space for another _asyncDownload call. Useful when you
        /// suddenly don't intend to call _mapForRead.
        virtual void _cancelDownload( size_t offset, size_t sizeBytes );

        /** Maps the buffer for read acces for the CPU.
        @remarks
            For internal use.
            mUploadOnly must be true.
            Attempting to const cast the returned pointer and write to it is undefined behavior.
            Call unmap( 0, 0 ) to unmap.
            Once mapped and unmapped, the same region shouldn't be remapped.
        @param offset
            The returned value from _asyncDownload.
        @param sizeBytes
            The size in bytes of the data to map. Should be parameter 'srcLength' passed
            to _asyncDownload.
        @return
            The pointer with the data read from the GPU. Read only.
        */
        const void* _mapForRead( size_t offset, size_t sizeBytes );

        /** Maps the given amount of bytes. May block if not ready.
            @See uploadWillStall if you wish to know.
        @remarks
            Will throw if sizeBytes > this->getMaxSize()
        */
        void* map( size_t sizeBytes );

        void unmap( const Destination *destinations, size_t numDestinations );

        /// Unmaps the mapped region and copies the data to the given region. @See Destination
        void unmap( const Destination &destination )    { unmap( &destination, 1 ); }

        /// Unmaps the mapped region and copies the data to multiple buffers. Useful when
        /// loading many meshes or textures at once (i.e. from multiple threads)
        void unmap( const DestinationVec &destinations )
                                { unmap( &destinations.front(), destinations.size() ); }

        size_t getMaxSize(void)                     { return mSizeBytes; }

        /// Adds a reference count to the StagingBuffer. @See removeReferenceCount
        void addReferenceCount(void);

        /** Decreases the reference count by one. StagingBuffers are manually reference counted.
            The first reason is performance. The second main reason is that the pointer doesn't
            get immediately deleted when the reference hits 0.
        @par
            Instead, a reference count of 0 means the Vao manager will monitor its lifetime.
            If it has been 0 for too long (past certain time threshold) the Vao manager will
            destroy this staging buffer.
        @par
            Meanwhile, the Staging Buffer will live in a pool until it's requested again or
            the time threshold is met. This prevents unwanted hiccups due to buffers getting
            recreated and destroyed all the time.
            Keep a non-zero ref. count to ensure the StagingBuffer won't be deleted due
            to timeouts (i.e. you know this buffer will get used at long regular intervals,
            like once every 15 minutes)
        @par
            Having a non-zero reference count doesn't mean the pointer will live forever though,
            as the memory is owned by the VaoManager: if the VaoManager is shutdown, this
            StagingBuffer will be freed.
        */
        void removeReferenceCount(void);

        uint16 getReferenceCount(void) const        { return mRefCount; }

        /// Returns the time in milliseconds in which a StagingBuffer should
        /// hazards unfenced while with a reference count of 0. @see getLifetimeThreshold
        uint32 getUnfencedTimeThreshold(void) const { return mUnfenceTimeThreshold; }

        /// Returns the time in milliseconds in which a StagingBuffer should
        /// live with a reference count of 0 before being deleted.
        uint32 getLifetimeThreshold(void) const     { return mLifetimeThreshold; }

        /// Returns the time in millisecond when the ref. count became 0.
        unsigned long getLastUsedTimestamp(void)    { return mLastUsedTimestamp; }
    };
}

#endif
