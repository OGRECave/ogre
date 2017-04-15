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

#ifndef _Ogre_D3D11VaoManager_H_
#define _Ogre_D3D11VaoManager_H_

#include "OgreD3D11Prerequisites.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    class _OgreD3D11Export D3D11VaoManager : public VaoManager
    {
        enum InternalBufferType
        {
            VERTEX_BUFFER,
            INDEX_BUFFER,
            SHADER_BUFFER,
            NumInternalBufferTypes
        };
    public:
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
            bool operator () ( const StrideChanger &left, const StrideChanger &right ) const
            {
                return left.offsetAfterPadding < right.offsetAfterPadding;
            }
        };

        typedef vector<Block>::type BlockVec;
        typedef vector<StrideChanger>::type StrideChangerVec;

    protected:
        struct Vbo
        {
            ID3D11Buffer        *vboName;
            size_t              sizeBytes;
            D3D11DynamicBuffer  *dynamicBuffer; //Null for non BT_DYNAMIC_* BOs.

            BlockVec            freeBlocks;
            StrideChangerVec    strideChangers;
        };

        struct Vao
        {
            uint32 vaoName;
            D3D11VertexArrayObjectShared *sharedData;

            struct VertexBinding
            {
                ID3D11Buffer        *vertexBufferVbo;
                VertexElement2Vec   vertexElements;
                uint32              stride;
                size_t              offset;

                //OpenGL supports this parameter per attribute, but
                //we're a bit more conservative and do it per buffer
                uint32              instancingDivisor;

                bool operator == ( const VertexBinding &_r ) const
                {
                    return vertexBufferVbo == _r.vertexBufferVbo &&
                            vertexElements == _r.vertexElements &&
                            stride == _r.stride &&
                            offset == _r.offset &&
                            instancingDivisor == _r.instancingDivisor;
                }
            };

            typedef vector<VertexBinding>::type VertexBindingVec;

            /// Not used anymore, however it's useful for sorting
            /// purposes in the RenderQueue (using the Vao's ID).
            OperationType operationType;
            VertexBindingVec    vertexBuffers;
            ID3D11Buffer        *indexBufferVbo;
            IndexBufferPacked::IndexType indexType;
            uint32              refCount;
        };

        typedef vector<Vbo>::type VboVec;
        typedef vector<Vao>::type VaoVec;
        typedef map<VertexElement2Vec, Vbo>::type VboMap;
        typedef vector<ID3D11Query*>::type D3D11SyncVec;

        VboVec  mVbos[NumInternalBufferTypes][BT_DYNAMIC_DEFAULT+1];
        /// MultiSource VBOs request a block from mVbo (i.e. they call allocateVbo) and thus do not
        /// own the vboName. For the rest, the way they manage free blocks is almost the same as
        /// with regular mVbos.
        VboMap  mMultiSourceVbos;
        size_t  mDefaultPoolSize[NumInternalBufferTypes][BT_DYNAMIC_DEFAULT+1];

        BufferPackedVec mDelayedBuffers[NumInternalBufferTypes];

        VaoVec  mVaos;
        uint32  mVaoNames;

        D3D11Device &mDevice;

        D3D11SyncVec mFrameSyncVec;

        VertexBufferPacked  *mDrawId;

        D3D11RenderSystem   *mD3D11RenderSystem;

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
        void allocateVbo( size_t sizeBytes, size_t alignment, BufferType bufferType,
                          InternalBufferType internalType,
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
                            BufferType bufferType, InternalBufferType internalType );

        void removeBufferFromDelayedQueue( BufferPackedVec &container, BufferPacked *buffer );

        void createImmutableBuffer( InternalBufferType internalType,
                                    size_t sizeBytes, void *initialData,
                                    Vbo &inOutVbo );

    public:
        /// @see StagingBuffer::mergeContiguousBlocks
        static void mergeContiguousBlocks( BlockVec::iterator blockToMerge,
                                           BlockVec &blocks );

    protected:
        virtual VertexBufferPacked* createVertexBufferImpl( size_t numElements,
                                                            uint32 bytesPerElement,
                                                            BufferType bufferType,
                                                            void *initialData, bool keepAsShadow,
                                                            const VertexElement2Vec &vertexElements );

        virtual void destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer );

        void createDelayedImmutableBuffers(void);
        void reorganizeImmutableVaos(void);

        virtual MultiSourceVertexBufferPool* createMultiSourceVertexBufferPoolImpl(
                                            const VertexElement2VecVec &vertexElementsBySource,
                                            size_t maxNumVertices, size_t totalBytesPerVertex,
                                            BufferType bufferType );

        virtual IndexBufferPacked* createIndexBufferImpl( size_t numElements,
                                                          uint32 bytesPerElement,
                                                          BufferType bufferType,
                                                          void *initialData, bool keepAsShadow );

        virtual void destroyIndexBufferImpl( IndexBufferPacked *indexBuffer );

        virtual ConstBufferPacked* createConstBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                          void *initialData, bool keepAsShadow );
        virtual void destroyConstBufferImpl( ConstBufferPacked *constBuffer );

        virtual TexBufferPacked* createTexBufferImpl( PixelFormat pixelFormat, size_t sizeBytes,
                                                      BufferType bufferType,
                                                      void *initialData, bool keepAsShadow );
        virtual void destroyTexBufferImpl( TexBufferPacked *texBuffer );

        virtual UavBufferPacked* createUavBufferImpl( size_t numElements, uint32 bytesPerElement,
                                                      uint32 bindFlags,
                                                      void *initialData, bool keepAsShadow );
        virtual void destroyUavBufferImpl( UavBufferPacked *uavBuffer );

        virtual IndirectBufferPacked* createIndirectBufferImpl( size_t sizeBytes, BufferType bufferType,
                                                                void *initialData, bool keepAsShadow );
        virtual void destroyIndirectBufferImpl( IndirectBufferPacked *indirectBuffer );

        /// Finds the Vao. Calls createVao automatically if not found.
        /// Increases refCount before returning the iterator.
        VaoVec::iterator findVao( const VertexBufferPackedVec &vertexBuffers,
                                  IndexBufferPacked *indexBuffer,
                                  OperationType opType );
        uint32 createVao( const Vao &vaoRef );
        void releaseVao( VertexArrayObject *vao );

        static uint32 generateRenderQueueId( uint32 vaoName, uint32 uniqueVaoId );
        static uint32 extractUniqueVaoIdFromRenderQueueId( uint32 rqId );

        virtual VertexArrayObject* createVertexArrayObjectImpl(
                                                        const VertexBufferPackedVec &vertexBuffers,
                                                        IndexBufferPacked *indexBuffer,
                                                        OperationType opType );

        virtual void destroyVertexArrayObjectImpl( VertexArrayObject *vao );

        D3D11CompatBufferInterface* createShaderBufferInterface( uint32 bindFlags,
                                                                 size_t sizeBytes,
                                                                 BufferType bufferType,
                                                                 void *initialData,
                                                                 uint32 structureByteStride = 0 );

    public:
        D3D11VaoManager( bool supportsIndirectBuffers, D3D11Device &device,
                         D3D11RenderSystem *renderSystem );
        virtual ~D3D11VaoManager();

        D3D11RenderSystem* getD3D11RenderSystem(void) const              { return mD3D11RenderSystem; }

        /// Binds the Draw ID to the currently bound vertex array object.
        void bindDrawId( uint32 bindSlotId );

        /** Creates a new staging buffer and adds it to the pool. @see getStagingBuffer.
        @remarks
            The returned buffer starts with a reference count of 1. You should decrease
            it when you're done using it.
        */
        virtual StagingBuffer* createStagingBuffer( size_t sizeBytes, bool forUpload );

        virtual AsyncTicketPtr createAsyncTicket( BufferPacked *creator, StagingBuffer *stagingBuffer,
                                                  size_t elementStart, size_t elementCount );

        virtual void _beginFrame(void);
        virtual void _update(void);

        /// @see VaoManager::waitForTailFrameToFinish
        virtual uint8 waitForTailFrameToFinish(void);

        /// See VaoManager::waitForSpecificFrameToFinish
        virtual void waitForSpecificFrameToFinish( uint32 frameCount );

        /// See VaoManager::isFrameFinished
        virtual bool isFrameFinished( uint32 frameCount );

        static ID3D11Query* createFence( D3D11Device &device );
        ID3D11Query* createFence(void);

        /** Will stall undefinitely until GPU finishes (signals the sync object).
        @param fenceName
            Sync object to wait for. Will be deleted on success. On failure,
            throws an exception and fenceName will not be deleted.
        @returns
            Null ptr on success. Should throw on failure, but if this function for some
            strange reason doesn't throw, it is programmed to return 'fenceName'
        */
        static ID3D11Query* waitFor( ID3D11Query *fenceName, ID3D11DeviceContextN *deviceContext );
        ID3D11Query* waitFor( ID3D11Query *fenceName );
    };
}

#endif
