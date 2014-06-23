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
        STALL_FULL
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
    class StagingBuffer
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
        size_t  mInternalBufferStart;
        size_t  mSizeBytes;
        bool    mUploadOnly;

        VaoManager      *mVaoManager;

        MappingState    mMappingState;
        size_t          mMappingStart;
        size_t          mMappingCount;

        void mapChecks( size_t sizeBytes );

        virtual void* mapImpl( size_t sizeBytes ) = 0;
        virtual void unmapImpl( const Destination *destinations, size_t numDestinations ) = 0;

    public:
        StagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                       VaoManager *vaoManager, bool uploadOnly );
        virtual ~StagingBuffer();

        /** Returns true if our next call to @map() with the same parameters will stall.
            @See StagingStallType
        @remarks
            Not all RenderSystems can accurately give this information and will always
            return STALL_PARTIAL (i.e. GLES2)
            The chances of getting a STALL_FULL get higher as sizeBytes gets closer
            to this->getMaxSize()
        */
        virtual StagingStallType willStall( size_t sizeBytes ) const;

        /** Maps the given amount of bytes. May block if not ready.
            @See willStall if you wish to know.
        @remarks
            Will throw if sizeBytes > this->getMaxSize()
        */
        virtual void* map( size_t sizeBytes );

        void unmap( const Destination *destinations, size_t numDestinations );

        /// Unmaps the mapped region and copies the data to the given region. @See Destination
        void unmap( const Destination &destination )    { unmap( &destination, 1 ); }

        /// Unmaps the mapped region and copies the data to multiple buffers. Useful when
        /// loading many meshes or textures at once (i.e. from multiple threads)
        void unmap( const DestinationVec &destinations )
                                { unmap( &destinations.front(), destinations.size() ); }

        size_t getMaxSize(void)                     { return mSizeBytes; }
    };
}

#endif
