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

#ifndef _Ogre_BufferPacked_H_
#define _Ogre_BufferPacked_H_

#include "OgrePrerequisites.h"

namespace Ogre
{
    enum BufferType
    {
        /** Read and write access from GPU. No access for CPU at all.
            i.e. RenderTextures, vertex buffers that will be used for R2VB
        @remarks
            Ogre will use staging buffers to upload and download contents,
            but don't do this frequently (except where needed,
            i.e. live video capture)
        */
        BT_DEFAULT,

        /// Read access from GPU. Write access for CPU.
        /// i.e. Particles, dynamic textures. Dynamic buffers don't put a
        /// hidden buffer behind the scenes. You get what you ask, therefore it's
        /// your responsability to ensure you don't lock a region that is currently
        /// in use by the GPU (or else stall).
        BT_DYNAMIC,

        /// Read access from GPU.
        /// i.e. Textures, most meshes.
        BT_IMMUTABLE,
    };

    enum MappingState
    {
        MS_UNMAPPED,
        MS_MAPPED,
        MS_PERSISTENT_INCOHERENT,
        MS_PERSISTENT_COHERENT,
        NUM_MAPPING_STATE
    };

    enum UnmapOptions
    {
        /// Unmaps all types of mapping, including persistent buffers.
        UO_UNMAP_ALL,

        /// When unmapping, unmap() will keep persistent buffers mapped.
        /// Further calls to map will only do some error checking
        UO_KEEP_PERSISTENT
    };

    class _OgreExport BufferPacked : public BufferPackedAlloc
    {
        friend class BufferInterface;
        friend class GL3PlusBufferInterface;

    protected:
        size_t mInternalBufferStart;    /// In elements
        size_t mNumElements;
        uint32 mBytesPerElement;

        BufferType      mBufferType;
        VaoManager      *mVaoManager;

        MappingState    mMappingState;
        size_t          mMappingStart;
        size_t          mMappingCount;

        BufferInterface *mBufferInterface;

        /// For non-persistent mapping, mLastMapping* will match its mMapping* counterparts.
        /// However for persistent mapping, the first call to map() will map a given range,
        /// and subsequent calls to map() can map a subregion of that first region, and thus
        /// their values won't match. The following assertions should always be true:
        ///     mLastMappingStart >= mMappingStart
        ///     mLastMappingCount <= mMappingStart - mLastMappingStart + mMappingCount
        /// This way we can efficiently emulate the behavior on OpenGL implementations that
        /// don't have persistent mapping while also efficiently flushing buffers mapped
        /// with PERSISTENT_INCOHERENT
        size_t          mLastMappingStart;
        size_t          mLastMappingCount;

        void *mShadowCopy;

    public:

        /** Generic constructor.
        @param initialData
            Initial data to populate. If bufferType == BT_IMMUTABLE, can't be null.
        @param keepAsShadow
            Keeps "intialData" as a shadow copy for reading from CPU without querying the GPU
            (can be useful for reconstructing buffers on device/context loss or for efficient
            reading of the data without streaming back from GPU.)

            If keepAsShadow is false, caller is responsible for freeing the data

            If keepAsShadow is true, we're responsible for freeing the pointer. We will free the
            pointer using OGRE_FREE_SIMD( MEMCATEGORY_GEOMETRY ), in which case the pointer
            *must* have been allocated using OGRE_MALLOC_SIMD( MEMCATEGORY_GEOMETRY )

            If the constructor throws, then data will NOT be freed, and caller will have to do it.

            Must be false if bufferType == BT_DYNAMIC
        */
        BufferPacked( size_t internalBufferStart, size_t numElements, uint32 bytesPerElement,
                      BufferType bufferType, void *initialData, bool keepAsShadow,
                      VaoManager *vaoManager, BufferInterface *bufferInterface );
        virtual ~BufferPacked();

        BufferType getBufferType(void) const                    { return mBufferType; }
        BufferInterface* getBufferInterface(void) const         { return mBufferInterface; }

        /** Sends the provided data to the GPU
        @param data
            The data to transfer to the GPU. Caller is responsible for freeing the pointer.
            "data" starts at offset zero. i.e.
            dst[elementStart * mBytesPerElement] = data[0];
        @param elementStart
            The start region, usually zero.
        @param elementCount
            Size, in number of elements, of data. Must be less than @getNumElements - elementStart
        */
        virtual void upload( void *data, size_t elementStart, size_t elementCount );

        /// Async data read request. A ticket will be returned. Once the async transfer finishes,
        /// you can use the ticket to read the data from CPU. @See AsyncTicket
        virtual AsyncTicket* readRequest( size_t elementStart, size_t elementCount ) = 0;

        virtual void disposeTicket( AsyncTicket *ticket ) = 0;

        void* map( size_t elementStart, size_t elementCount, MappingState persistentMethod );
        void unmap( UnmapOptions unmapOption );

        uint32 getNumElements(void) const       { return mNumElements; }
        uint32 getBytesPerElement(void) const   { return mBytesPerElement; }
        uint32 getTotalSizeBytes(void) const    { return mNumElements * mBytesPerElement; }
    };
}

#endif
