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

#ifndef _Ogre_GL3PlusStagingBuffer_H_
#define _Ogre_GL3PlusStagingBuffer_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreStagingBuffer.h"

namespace Ogre
{
    /** A staging buffer is a buffer that resides on the GPU and be written to/from both CPU & GPU
        However the access in both cases is limited. GPUs can only copy (i.e. memcpy) to another
        real buffer (can't be used directly as i.e. texture or vertex buffer) and CPUs can only
        map it.
        In other words, a staging buffer is an intermediate buffer to transfer data between
        CPU & GPU
    */
    class _OgreGL3PlusExport GL3PlusStagingBuffer : public StagingBuffer
    {
    protected:
        /// mVboName is not deleted by us (the VaoManager does) as we may have
        /// only been assigned a chunk of the buffer, not the whole thing.
        GLuint  mVboName;
        void    *mMappedPtr;

        /** How many bytes between the last fence and our current offset do we need to let
            through before we place another fence?
        @remarks
            When the ring buffer wraps around, a fence is always put.
            A threshold of zero means to put a fence after every unmap operation.
        */
        size_t  mFenceThreshold;

        struct GLFence : Fence
        {
            GLsync  fenceName;

            GLFence( size_t _start, size_t _end ) :
                Fence( _start , _end ), fenceName( 0 )
            {
            }
        };

        //------------------------------------
        // Begin used for uploads
        //------------------------------------
        typedef vector<GLFence>::type GLFenceVec;
        GLFenceVec mFences;

        /// Regions of memory that were unmapped but haven't
        /// been fenced due to not passing the threshold yet.
        GLFenceVec mUnfencedHazards;
        size_t mUnfencedBytes;
        //------------------------------------
        // End used for uploads
        //------------------------------------

        /** Adds a fence to the region [from; to]. The region may actually be kept in
            mUnfencedHazards until the threshold is achieved or we're forced to fence
            it (i.e. when the internal buffer wraps around)
        @param forceFence
            When true, the region is merged with all the other unfenced hazards, and
            put behind a fence
        */
        void addFence( size_t from, size_t to, bool forceFence );

        void deleteFences( GLFenceVec::iterator itor, GLFenceVec::iterator end );

        /// Waits undefinitely on the given GLSync object. Can throw if failed to wait.
        void wait( GLsync syncObj );

        /// Checks all the fences and compares them against current state (i.e. mMappingStart &
        /// mMappingCount), and stalls if needed (synchronize); also book-keeps mFences and
        /// mUnfencedHazards.
        /// May modify mMappingStart.
        void waitIfNeeded(void);

        virtual void* mapImpl( size_t sizeBytes );
        virtual void unmapImpl( const Destination *destinations, size_t numDestinations );

        virtual const void* _mapForReadImpl( size_t offset, size_t sizeBytes );

    public:
        GL3PlusStagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                              VaoManager *vaoManager, bool uploadOnly, GLuint vboName );
        virtual ~GL3PlusStagingBuffer();

        virtual StagingStallType uploadWillStall( size_t sizeBytes );

        void cleanUnfencedHazards(void);

        virtual size_t _asyncDownload( BufferPacked *source, size_t srcOffset, size_t srcLength );

        GLuint getBufferName(void) const           { return mVboName; }
    };
}

#endif
