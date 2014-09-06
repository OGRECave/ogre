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

#ifndef _Ogre_VaoManager_H_
#define _Ogre_VaoManager_H_

#include "OgrePrerequisites.h"

#include "Vao/OgreVertexBufferPacked.h"
#include "Vao/OgreIndexBufferPacked.h"
#include "OgreRenderOperation.h"

namespace Ogre
{
    typedef vector<StagingBuffer*>::type        StagingBufferVec;
    typedef vector<VertexArrayObject*>::type    VertexArrayObjectVec;

    class _OgreExport VaoManager : public RenderSysAlloc
    {
    protected:
        Timer *mTimer;

    public:
        /// In millseconds. Note: Changing this value won't affect existing staging buffers.
        uint32 mDefaultStagingBufferLifetime;

    protected:
        /// [0] = for uploads, [1] = for downloads
        StagingBufferVec mStagingBuffers[2];
        StagingBufferVec mZeroRefStagingBuffers[2];

        uint8           mDynamicBufferMultiplier;
        uint8           mDynamicBufferCurrentFrame;
        unsigned long   mNextStagingBufferTimestampCheckpoint;

        BufferPackedVec         mVertexBuffers;
        BufferPackedVec         mIndexBuffers;
        BufferPackedVec         mConstBuffers;
        VertexArrayObjectVec    mVertexArrayObjects;

        uint32          mFrameCount;

        virtual VertexBufferPacked* createVertexBufferImpl( size_t numElements,
                                                            uint32 bytesPerElement,
                                                            BufferType bufferType,
                                                            void *initialData, bool keepAsShadow,
                                                            const VertexElement2Vec &vertexElements )
                                                            = 0;

        virtual void destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer ) = 0;

        virtual MultiSourceVertexBufferPool* createMultiSourceVertexBufferPoolImpl(
                                                    const VertexElement2VecVec &vertexElementsBySource,
                                                    size_t maxNumVertices, size_t totalBytesPerVertex,
                                                    BufferType bufferType ) = 0;

        virtual IndexBufferPacked* createIndexBufferImpl( size_t numElements,
                                                          uint32 bytesPerElement,
                                                          BufferType bufferType,
                                                          void *initialData, bool keepAsShadow ) = 0;

        virtual void destroyIndexBufferImpl( IndexBufferPacked *indexBuffer ) = 0;

        virtual ConstBufferPacked* createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                          void *initialData, bool keepAsShadow ) = 0;

        virtual void destroyConstBufferImpl( ConstBufferPacked *constBuffer ) = 0;

        virtual VertexArrayObject* createVertexArrayObjectImpl( const VertexBufferPackedVec &vertexBuffers,
                                                                IndexBufferPacked *indexBuffer,
                                                                v1::RenderOperation::OperationType opType ) = 0;

        virtual void destroyVertexArrayObjectImpl( VertexArrayObject *vao ) = 0;

        /// Destroys all created VAOs. Warning: If you still have references to
        /// those pointers, they will become dangling. Don't call this if you
        /// are unsure; unless you're shutting down. It gets called automatically
        /// on shutdown though.
        void destroyAllVertexArrayObjects(void);

        /// Just deletes the pointers, but may not destroy/free the API constructs.
        /// Utility helper for derived classes. Also clears the container.
        static void deleteAllBuffers( BufferPackedVec &buffersContainer );

    public:
        VaoManager();
        virtual ~VaoManager();

        /// Returns the size of a single vertex buffer source with the given declaration, in bytes
        static uint32 calculateVertexSize( const VertexElement2Vec &vertexElements );

        /** Creates a vertex buffer based on the given parameters. Behind the scenes, the vertex buffer
            is part of much larger vertex buffer, in order to reduce bindings at runtime.
        @param vertexElements
            A list of element bindings for this vertex buffer. Once created, changing VertexElements
            is not possible, you'll have to create another Vertex Buffer.
        @param numVertices
            The number of vertices for this vertex
        @param bufferType
            The type of buffer for this vertex buffer. @See BufferType::BT_DYNAMIC special case.
        @param initialData
            Initial data the buffer will hold upon creation. Can be null (i.e. you plan to upload later).
            Cannot be null when bufferType is BT_IMMUTABLE. Must have enough room to prevent an overflow.
        @param keepAsShadow
            Whether to keep the pointer "initialData" as a shadow copy of the contents.
            @See BufferPacked::BufferPacked regarding on who is responsible for freeing this pointer
            and what happens if an exception was raised.
        @return
            The desired vertex buffer pointer
        */
        VertexBufferPacked* createVertexBuffer( const VertexElement2Vec &vertexElements,
                                                size_t numVertices, BufferType bufferType,
                                                void *initialData, bool keepAsShadow );

        MultiSourceVertexBufferPool* createMultiSourceVertexBufferPool(
                                const VertexElement2VecVec &vertexElementsBySource,
                                size_t maxNumVertices, BufferType bufferType );

        /** Destroys the given vertex buffer created with createVertexBuffer.
            NOTE: Vertex Buffers created by a MultiSourceVertexBufferPool
            must be freed by the pool that created it, don't use this function for those.
        @remarks
            Performs an O(N) lookup. Where N is the number of created vertex buffers
        */
        void destroyVertexBuffer( VertexBufferPacked *vertexBuffer );

        /** Creates an index buffer based on the given parameters. Behind the scenes, the buffer
            is actually part of much larger buffer, in order to reduce bindings at runtime.
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
        @param indexType
            Whether this Index Buffer should be 16-bit (recommended) or 32-bit
        @param numIndices
            The number of indices
        @return
            The desired index buffer pointer
        */
        IndexBufferPacked* createIndexBuffer( IndexBufferPacked::IndexType indexType,
                                              size_t numIndices, BufferType bufferType,
                                              void *initialData, bool keepAsShadow );

        /** Destroys the given index buffer created with createIndexBuffer.
        @param indexBuffer
            Index Buffer created with createIndexBuffer
        */
        void destroyIndexBuffer( IndexBufferPacked *indexBuffer );

        /** Creates an constant buffer based on the given parameters. Behind the scenes, the buffer
            is actually part of much larger buffer, in order to reduce bindings at runtime.
            (depends on the RenderSystem, on D3D11 we're forced to give its own buffer)
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
        @param sizeBytes
            The size in bytes of the given constant buffer. API restrictions may apply.
            To stay safe keep it multiple of 16, don't request more than 64kb per buffer.
        @return
            The desired const buffer pointer
        */
        ConstBufferPacked* createConstBuffer( size_t sizeBytes, BufferType bufferType,
                                              void *initialData, bool keepAsShadow );

        /** Destroys the given constant buffer created with createConstBuffer.
        @param constBuffer
            Constant Buffer created with createConstBuffer
        */
        void destroyConstBuffer( ConstBufferPacked *constBuffer );

        /** Creates a VertexArrayObject that binds all the vertex buffers with their respective
            declarations, and the index buffers. The returned value is immutable and thus cannot
            be modified.
        @param vertexBuffers
            An array of vertex buffers to be bound to the vertex array object.
        @param indexBuffer
            The index buffer to be bound.
        @param opType
            Type of operation. Cannot be changed later.
        @return
            VertexArrayObject that can be rendered.
        */
        VertexArrayObject* createVertexArrayObject( const VertexBufferPackedVec &vertexBuffers,
                                                    IndexBufferPacked *indexBuffer,
                                                    v1::RenderOperation::OperationType opType );

        /** Destroys the input pointer. After this call, it's no longer valid
        @remarks
            API memory may or may not be released since VertexArrayObjects
            may internally share the same API constructs.
        @par
            The vertex & index buffers associated with this vao are not destroyed by
            this function.
        */
        void destroyVertexArrayObject( VertexArrayObject *vao );

        /** Creates a new staging buffer and adds it to the pool. @see getStagingBuffer.
        @remarks
            The returned buffer starts with a reference count of 1. You should decrease
            it when you're done using it.
        */
        virtual StagingBuffer* createStagingBuffer( size_t sizeBytes, bool forUpload ) = 0;

        /** Retrieves a staging buffer for use. We'll search for existing ones that can
            hold minSizeBytes. We first prioritize those that won't cause a stall at all.
            Then those that will cause a partial stall, and otherwise return one that will
            cause full stall.
            If we can't find any existing buffer that can hold the requested number bytes,
            we'll create a new one.
        @remarks
            The reference count is increasesd before returning the staging buffer.
            You should decrease the reference count after you're done with the returned
            pointer. @See StagingBuffer::removeReferenceCount regarding ref. counting.
        @param sizeBytes
            Minimum size, in bytes, of the staging buffer.
            The returned buffer may be bigger.
        @param forUpload
            True if it should be used to upload data to GPU, false to download.
        @return
            The staging buffer.
        */
        StagingBuffer* getStagingBuffer( size_t minSizeBytes, bool forUpload );

        virtual void _update(void);

        void _notifyStagingBufferEnteredZeroRef( StagingBuffer *stagingBuffer );
        void _notifyStagingBufferLeftZeroRef( StagingBuffer *stagingBuffer );

        Timer* getTimer(void)               { return mTimer; }

        uint32 getFrameCount(void)          { return mFrameCount; }

        uint8 _getDynamicBufferCurrentFrameNoWait(void) const   { return mDynamicBufferCurrentFrame; }
        uint8 getDynamicBufferMultiplier(void) const            { return mDynamicBufferMultiplier; }
    };
}

#endif
