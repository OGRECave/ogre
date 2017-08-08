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
#include "OgrePixelFormat.h"

namespace Ogre
{
    typedef vector<StagingBuffer*>::type               StagingBufferVec;
    typedef unordered_set<VertexArrayObject*>::type    VertexArrayObjectSet;

    class _OgreExport VaoManager : public RenderSysAlloc
    {
    protected:
        Timer *mTimer;

        /// In millseconds. Note: Changing this value won't affect existing staging buffers.
        uint32 mDefaultStagingBufferUnfencedTime;
        uint32 mDefaultStagingBufferLifetime;

        /// [0] = for uploads, [1] = for downloads
        StagingBufferVec mRefedStagingBuffers[2];
        StagingBufferVec mZeroRefStagingBuffers[2];

        bool            mSupportsPersistentMapping;
        bool            mSupportsIndirectBuffers;
        bool            mSupportsBaseInstance;
        uint8           mDynamicBufferMultiplier;
        uint8           mDynamicBufferCurrentFrame;
        unsigned long   mNextStagingBufferTimestampCheckpoint;
        uint32          mFrameCount;

        BufferPackedSet         mBuffers[NUM_BUFFER_PACKED_TYPES];
        VertexArrayObjectSet    mVertexArrayObjects;
        uint32                  mNumGeneratedVaos; /// Increases on every createVertexArrayObject call

        struct DelayedBuffer
        {
            BufferPacked    *bufferPacked;
            uint32          frame;
            uint8           frameNumDynamic;

            DelayedBuffer( BufferPacked *_bufferPacked, uint32 _frame, uint8 _frameNumDynamic ) :
                bufferPacked( _bufferPacked ), frame( _frame ), frameNumDynamic( _frameNumDynamic ) {}
        };

        typedef vector<DelayedBuffer>::type DelayedBufferVec;
        DelayedBufferVec    mDelayedDestroyBuffers;

        uint32 mConstBufferAlignment;
        uint32 mTexBufferAlignment;
        uint32 mUavBufferAlignment;
        size_t mConstBufferMaxSize;
        size_t mTexBufferMaxSize;
        size_t mUavBufferMaxSize;

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

        virtual TexBufferPacked* createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                      BufferType bufferType,
                                                      void *initialData, bool keepAsShadow ) = 0;
        virtual void destroyTexBufferImpl( TexBufferPacked *texBuffer ) = 0;

        virtual UavBufferPacked* createUavBufferImpl( size_t numElements, uint32 bytesPerElement,
                                                      uint32 bindFlags,
                                                      void *initialData, bool keepAsShadow ) = 0;
        virtual void destroyUavBufferImpl( UavBufferPacked *uavBuffer ) = 0;

        virtual IndirectBufferPacked* createIndirectBufferImpl( size_t sizeBytes,
                                                                BufferType bufferType,
                                                                void *initialData,
                                                                bool keepAsShadow ) = 0;
        virtual void destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer ) = 0;

        virtual VertexArrayObject* createVertexArrayObjectImpl( const VertexBufferPackedVec &vertexBuffers,
                                                                IndexBufferPacked *indexBuffer,
                                                                OperationType opType ) = 0;

        virtual void destroyVertexArrayObjectImpl( VertexArrayObject *vao ) = 0;

        /// Destroys all created VAOs. Warning: If you still have references to
        /// those pointers, they will become dangling. Don't call this if you
        /// are unsure; unless you're shutting down. It gets called automatically
        /// on shutdown though.
        void destroyAllVertexArrayObjects(void);

        /// Just deletes the pointers, but may not destroy/free the API constructs.
        /// Utility helper for derived classes. Also clears the containers.
        void deleteAllBuffers(void);

        /** Removes all the buffers whose destruction was delayed until now.
        @remarks
            Reads mDynamicBufferCurrentFrame and mFrameCount.
            Caller is responsible for hazard checking.
        */
        void destroyDelayedBuffers( uint8 fromDynamicFrame );

        inline void callDestroyBufferImpl( BufferPacked *bufferPacked );

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
            @see BufferPacked::BufferPacked
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

        /** Creates an constant buffer based on the given parameters. Behind the scenes, the buffer
            is actually part of much larger buffer, in order to reduce bindings at runtime.
            (depends on the RenderSystem, on D3D11 we're forced to give its own buffer)
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
        @param pixelFormat
            The pixel format for the texture buffer.
        @param sizeBytes
            The size in bytes of the given constant buffer. API restrictions may apply.
        @return
            The desired texture buffer pointer
        */
        TexBufferPacked* createTexBuffer( PixelFormat pixelFormat, size_t sizeBytes,
                                          BufferType bufferType,
                                          void *initialData, bool keepAsShadow );

        /** Destroys the given texture buffer created with createTexBuffer.
        @param constBuffer
            Texture Buffer created with createTexBuffer
        */
        void destroyTexBuffer( TexBufferPacked *texBuffer );

        /** Creates an UAV buffer based on the given parameters. Behind the scenes, the buffer
            is actually part of much larger buffer, in order to reduce bindings at runtime.
            (depends on the RenderSystem, on D3D11 we're forced to give its own buffer)
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
            There is no BufferType option as the only available one is BT_DEFAULT
        @param sizeBytes
            The size in bytes of the given constant buffer. API restrictions may apply.
        @param bindFlags
            A combination of BufferBindFlags. BB_FLAG_UAV is implicit.
        @return
            The desired UAV buffer pointer
        */
        UavBufferPacked* createUavBuffer( size_t numElements, uint32 bytesPerElement, uint32 bindFlags,
                                          void *initialData, bool keepAsShadow );

        /** Destroys the given UAV buffer created with createUavBuffer.
        @param constBuffer
            Uav Buffer created with createUavBuffer
        */
        void destroyUavBuffer( UavBufferPacked *uavBuffer );

        /** Creates an indirect buffer.
        @remarks
            @See createVertexBuffer for the remaining parameters not documented here.
        */
        IndirectBufferPacked* createIndirectBuffer( size_t sizeBytes, BufferType bufferType,
                                                    void *initialData, bool keepAsShadow );

        /** Destroys the given indirect buffer created with createIndirectBuffer.
        @param indirectBuffer
            Indirect Buffer created with createIndirectBuffer
        */
        void destroyIndirectBuffer( IndirectBufferPacked *indirectBuffer );

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
                                                    OperationType opType );

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
            Calling this function causes the reference count of the returned pointer to
            be increased.
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

        virtual AsyncTicketPtr createAsyncTicket( BufferPacked *creator, StagingBuffer *stagingBuffer,
                                                  size_t elementStart, size_t elementCount ) = 0;

        virtual void _beginFrame(void) {}
        virtual void _update(void);

        void _notifyStagingBufferEnteredZeroRef( StagingBuffer *stagingBuffer );
        void _notifyStagingBufferLeftZeroRef( StagingBuffer *stagingBuffer );

        uint32 getConstBufferAlignment(void) const      { return mConstBufferAlignment; }
        uint32 getTexBufferAlignment(void) const        { return mTexBufferAlignment; }
        uint32 getUavBufferAlignment(void) const        { return mUavBufferAlignment; }
        size_t getConstBufferMaxSize(void) const        { return mConstBufferMaxSize; }
        size_t getTexBufferMaxSize(void) const          { return mTexBufferMaxSize; }
        size_t getUavBufferMaxSize(void) const          { return mUavBufferMaxSize; }

        bool supportsPersistentMapping(void) const      { return mSupportsPersistentMapping; }

        /// When false, IndirectBufferPacked will emulate the mapping behavior,
        /// and we need to use the emulated calls in RenderSystem.
        bool supportsIndirectBuffers(void) const        { return mSupportsIndirectBuffers; }
        bool supportsBaseInstance(void) const           { return mSupportsBaseInstance; }

        Timer* getTimer(void)               { return mTimer; }

        uint32 getFrameCount(void)          { return mFrameCount; }

        /** Sets the default time for staging buffers. Staging buffers are recycled/reused.
            When their reference count reaches 0, this VaoManager will begin to track how
            long since the last time they've reached ref count 0.
            When the time threshold is met, the staging buffer gets removed.
        @remarks
            See getStagingBuffer
        @par
            Small explanation on StagingBuffer lifetime management (so you can make sense on
            what we're doing here):

            StagingBuffers may produce "pops" when created (due to API/driver stalls) so we
            reuse them. We keep track of how long StagingBuffers have been remained unused,
            and delete old ones. The intention is to be able to recycle old buffers,
            while getting rid of the excess if a sudden spike happened. That's what the lifetime
            threshold controls.

            We also have the "unfenced threshold". This controls something very different.
            StagingBuffers usages need fences to check whether we're done using the buffer.
            But we don't issue a fence every time you do something with them, because that could
            result in a lot of fences (i.e. imagine you do a 1000 uploads of 16 bytes each, in
            succession, we shouldn't do 1000 fences); so we fence when certain upload/download
            thresholds are met (i.e. you've uploaded 1MB of data). So if you've uploaded 750kb
            so far, no fence will be issued. But if you upload 300kb more, we will fence.
            But what happens if you've only uploaded 750kb and then nothing more for the last
            10 minutes? Since we haven't fenced, and now you need to upload a lot more, we don't
            know if those 750kb are done uploading because we never fenced it. It probably ended
            10 minutes ago, but we won't know. We would have to fence now and perform a full
            stall waiting for that fence. To solve this edge case, we fence whenever X time has
            elapsed without fencing (and only if there's data that remains unfenced of course).
            That's what the unfenced threshold is for.
        @param lifetime
            Time in milliseconds. The default is 5 minutes.
            A staging buffer that remained at zero ref. count for lifetime milliseconds
            will be destroyed.
        @param unfencedTime
            For efficiency reasons (API overhead), some staging buffers implementations
            will not track all hazards on fences. A staging buffer that remained at
            zero ref. count for unfencedTime milliseconds will be told to clean
            their hazards, creating the missing appropiate fences.
            unfencedTime can't be higher than lifetime.
            unfencedTime should not be too far away from lifetime, and not too soon
            either, to maximize the chances of no stalls when we finally destroy it
            and avoid excessive API overhead in keeping fences alive.
            The default is 4 minutes 59 seconds.
        */
        void setDefaultStagingBufferlifetime( uint32 lifetime, uint32 unfencedTime );

        uint32 getDefaultStagingBufferUnfencedTime(void) const
                                            { return mDefaultStagingBufferUnfencedTime; }
        uint32 getDefaultStagingBufferLifetime(void) const
                                            { return mDefaultStagingBufferLifetime; }

        uint8 _getDynamicBufferCurrentFrameNoWait(void) const   { return mDynamicBufferCurrentFrame; }
        uint8 getDynamicBufferMultiplier(void) const            { return mDynamicBufferMultiplier; }

        /// Returns the current frame # (which wraps to 0 every mDynamicBufferMultiplier
        /// times). But first stalls until that mDynamicBufferMultiplier-1 frame behind
        /// is finished.
        virtual uint8 waitForTailFrameToFinish(void) = 0;

        /** Waits for a specific frame to be ready.
            Calling waitForSpecificFrameToFinish( mFrameCount - mDynamicBufferMultiplier )
            equals to calling waitForTailFrameToFinish.
        @remarks
            WARNING: calling waitForSpecificFrameToFinish( mFrameCount ); will perform
            a full stall!
        @par
            Avoid calling this function. Some implementations may decide to always full stall
            unless (mFrameCount - frameCount) >= mDynamicBufferMultiplier
        */
        virtual void waitForSpecificFrameToFinish( uint32 frameCount ) = 0;

        /// If this returns true, then waitForSpecificFrameToFinish is guaranteed to return immediately.
        virtual bool isFrameFinished( uint32 frameCount ) = 0;
    };
}

#endif
