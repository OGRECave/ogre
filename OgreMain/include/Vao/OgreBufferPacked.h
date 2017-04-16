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

#include "OgreResourceTransition.h"

namespace Ogre
{
    enum BufferType
    {
        /// Read access from GPU.
        /// i.e. Textures, most meshes.
        BT_IMMUTABLE,

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
        BT_DYNAMIC_DEFAULT,

        /// Same as BT_DYNAMIC, but mapping will be persistent.
        BT_DYNAMIC_PERSISTENT,

        /// Same as BT_DYNAMIC_PERSISTENT, but mapping will be persistent and cache coherent.
        BT_DYNAMIC_PERSISTENT_COHERENT,
    };

    enum MappingState
    {
        MS_UNMAPPED,
        MS_MAPPED,
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

    enum BufferPackedTypes
    {
        BP_TYPE_VERTEX,
        BP_TYPE_INDEX,
        BP_TYPE_CONST,
        BP_TYPE_TEX,
        BP_TYPE_UAV,
        BP_TYPE_INDIRECT,
        NUM_BUFFER_PACKED_TYPES
    };

    enum BufferBindFlags
    {
        BB_FLAG_VERTEX      = 1u << BP_TYPE_VERTEX,
        BB_FLAG_INDEX       = 1u << BP_TYPE_INDEX,
        BB_FLAG_CONST       = 1u << BP_TYPE_CONST,
        BB_FLAG_TEX         = 1u << BP_TYPE_TEX,
        BB_FLAG_UAV         = 1u << BP_TYPE_UAV,
        BB_FLAG_INDIRECT    = 1u << BP_TYPE_INDIRECT
    };

    /** Helper class to that will free the pointer on the destructor. Usage:
            SafeDelete dataPtrContainer( data );
            vaoManager->createVertexBuffer( vertexElements, vertexCount,
                                            mVertexBufferDefaultType,
                                            data, keepAsShadow );

            if( !keepAsShadow )
                dataPtrContainer.ptr = 0;

        In this example, "data" wouln't normally be freed if createVertexBuffer
        raises an exception according to BufferPacked's constructor rules, but
        thanks to FreeOnDestructor, the pointer will be freed accordingly.
        Once the BufferPacked has been created, we need to zero the ptr member
        to avoid freeing it (since BufferPacked will do it).
    */
    struct FreeOnDestructor
    {
        void *ptr;
        FreeOnDestructor( void *_ptr ) : ptr( _ptr ) {}
        ~FreeOnDestructor()
        {
            if( ptr )
                OGRE_FREE_SIMD( ptr, MEMCATEGORY_GEOMETRY );
        }
    };

    class _OgreExport BufferPacked : public GpuResource, public BufferPackedAlloc
    {
        friend class BufferInterface;
        friend class D3D11BufferInterface;
        friend class D3D11CompatBufferInterface;
        friend class GL3PlusBufferInterface;
        friend class MetalBufferInterface;
        friend class NULLBufferInterface;

    protected:
        size_t mInternalBufferStart;  /// In elements
        size_t mFinalBufferStart;     /// In elements, includes dynamic buffer frame offset
        size_t mNumElements;
        uint32 mBytesPerElement;
        uint32 mNumElementsPadding;

        BufferType      mBufferType;
        VaoManager      *mVaoManager;

        MappingState    mMappingState;

        BufferInterface *mBufferInterface;

        /// Stores the range of the last map() call so that
        /// we can flush it correctly when calling unmap
        size_t          mLastMappingStart;
        size_t          mLastMappingCount;

        void *mShadowCopy;

#if OGRE_DEBUG_MODE
        /// Used by Dynamic buffers only
        uint32 mLastFrameMapped;
        uint32 mLastFrameMappedAndAdvanced;
#endif

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
            @see FreeOnDestructor to help you with exceptions and correctly freeing the data.

            Must be false if bufferType >= BT_DYNAMIC
        */
        BufferPacked( size_t internalBufferStartBytes, size_t numElements, uint32 bytesPerElement,
                      uint32 numElementsPadding, BufferType bufferType,
                      void *initialData, bool keepAsShadow,
                      VaoManager *vaoManager, BufferInterface *bufferInterface );
        virtual ~BufferPacked();

        /// Useful to query which one is the derived class.
        virtual BufferPackedTypes getBufferPackedType(void) const = 0;

        /// For internal use.
        void _setBufferInterface( BufferInterface *bufferInterface );

        BufferType getBufferType(void) const                    { return mBufferType; }
        BufferInterface* getBufferInterface(void) const         { return mBufferInterface; }

        /// Async data read request. A ticket will be returned. Once the async transfer finishes,
        /// you can use the ticket to read the data from CPU. @See AsyncTicket
        AsyncTicketPtr readRequest( size_t elementStart, size_t elementCount );

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
        virtual void upload( const void *data, size_t elementStart, size_t elementCount );

        /** Maps the specified region to a pointer the CPU can access. Only dynamic buffers
            can use this function. The region [elementStart; elementStart + elementCount)
            will be mapped.
        @remarks
            You can only map once per frame, regardless of parameters (except for advanceFrame).
            map( 0, 1 ) followed by map( 1, 1 ); is invalid.
            If you plan modifying elements 0 and 1; you should call map( 0, 2 )
        @par
            Note that even if you use persistent mapping, you still need to call @see unmap.
        @param elementStart
            Start of the region to be mapped, in elements. Normally you want this to be 0.
        @param elementCount
            Length of the region to map, in elements. @see getNumElements to map the whole range.
            Can't be 0.
        @param bAdvanceFrame
            When true, the Buffer will be usable after unmapping it (or earlier if persistent mapped).
            However you won't be able to call map() again until the next frame.
            Calling this with false allows to call map multiple times. However ater calling unmap,
            you must call advanceFrame. THIS IS ONLY FOR VERY ADVANCED USERS.
        */
        void* RESTRICT_ALIAS_RETURN map( size_t elementStart, size_t elementCount, bool bAdvanceFrame=true );

        /** Unmaps or flushes the region mapped with @see map. Alternatively, you can flush a smaller region
            (i.e. you didn't know which regions you were to update when mapping, but now that you're done,
            you know).
            The region being flushed is [flushStart; flushStart + flushSize)
        @param unmapOption
            When using persistent mapping, UO_KEEP_PERSISTENT will keep the map alive; but you will
            have to call map again to use it. This requirement allows Ogre to:
                1. Synchronize if needed (avoid mapping a region that is still in use)
                2. Emulate persistent mapping on Hardware/Drivers that don't support it.
        @param flushStartElem
            In elements, 0-based index (based on the mapped region) on where to start flushing from.
            Default is 0.
        @param flushSizeElem
            The length of the flushing region, which can't be bigger than 'elementCount' passed
            to @see map. When this value is 0, we flush until the end of the buffer starting
            from flushStartElem
        */
        void unmap( UnmapOptions unmapOption, size_t flushStartElem = 0, size_t flushSizeElem = 0 );

        /// @see map. Do NOT call this function more than once per frame,
        /// or if you've called map( advanceFrame = true )
        void advanceFrame(void);

        /// Performs the opposite of @see advanceFrame. Only call this after having called
        /// advanceFrame. i.e. restore the buffer to the state it was before calling
        /// advanceFrame.
        void regressFrame(void);

        /// Returns the mapping state. Note that if you call map with MS_PERSISTENT_INCOHERENT or
        /// MS_PERSISTENT_COHERENT, then call unmap( UO_KEEP_PERSISTENT ); the returned value will
        /// still be MS_PERSISTENT_INCOHERENT/_COHERENT when persistent mapping is supported.
        /// This differs from isCurrentlyMapped
        MappingState getMappingState(void) const                { return mMappingState; }

        /// Returns whether the buffer is currently mapped. If you've persistently mapped the buffer
        /// and then called unmap( UO_KEEP_PERSISTENT ); this function will return false; which
        /// differs from getMappingState's behavior.
        bool isCurrentlyMapped(void) const;

        uint32 getNumElements(void) const       { return mNumElements; }
        uint32 getBytesPerElement(void) const   { return mBytesPerElement; }
        uint32 getTotalSizeBytes(void) const    { return mNumElements * mBytesPerElement; }

        size_t _getInternalBufferStart(void) const              { return mInternalBufferStart; }
        size_t _getFinalBufferStart(void) const                 { return mFinalBufferStart; }
        uint32 _getInternalTotalSizeBytes(void) const   { return (mNumElements + mNumElementsPadding) *
                                                                 mBytesPerElement; }
        uint32 _getInternalNumElements(void) const      { return mNumElements + mNumElementsPadding; }

        const void* getShadowCopy(void) const   { return mShadowCopy; }

        /// This will not delete the existing shadow copy so it can be used for other purposes
        /// if it is not needed call OGRE_FREE_SIMD( m->getShadowCopy(), MEMCATEGORY_GEOMETRY )
        /// before calling this function.
        /// This will also not automatically upload the shadow data to the GPU. The user must call
        /// upload or use a staging buffer themselves to achieve this.
        void _setShadowCopy( void* copy );
    };

    typedef vector<BufferPacked*>::type BufferPackedVec;
    typedef unordered_set<BufferPacked*>::type BufferPackedSet;
}

#endif
