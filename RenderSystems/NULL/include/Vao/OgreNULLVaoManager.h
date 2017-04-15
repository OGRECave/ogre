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

#ifndef _Ogre_NULLVaoManager_H_
#define _Ogre_NULLVaoManager_H_

#include "OgreNULLPrerequisites.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    class _OgreNULLExport NULLVaoManager : public VaoManager
    {
    protected:
        enum VboFlag
        {
            CPU_INACCESSIBLE,
            CPU_ACCESSIBLE_DEFAULT,
            CPU_ACCESSIBLE_PERSISTENT,
            CPU_ACCESSIBLE_PERSISTENT_COHERENT,
            MAX_VBO_FLAG
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
            size_t sizeBytes;

            BlockVec            freeBlocks;
            StrideChangerVec    strideChangers;
        };

        struct Vao
        {
            struct VertexBinding
            {
                //GLuint              vertexBufferVbo;
                VertexElement2Vec   vertexElements;
                uint32              stride;
                size_t              offset;

                //OpenGL supports this parameter per attribute, but
                //we're a bit more conservative and do it per buffer
                uint32              instancingDivisor;

                bool operator == ( const VertexBinding &_r ) const
                {
                    return //vertexBufferVbo == _r.vertexBufferVbo &&
                            vertexElements == _r.vertexElements &&
                            stride == _r.stride &&
                            offset == _r.offset &&
                            instancingDivisor == _r.instancingDivisor;
                }
            };

            typedef vector<VertexBinding>::type VertexBindingVec;

            VertexBindingVec    vertexBuffers;
            uint32              indexBufferVbo;
            IndexBufferPacked::IndexType indexType;
            uint32              refCount;
        };

        typedef vector<Vbo>::type VboVec;
        typedef vector<Vao>::type VaoVec;
        typedef map<VertexElement2Vec, Vbo>::type VboMap;

        VboVec  mVbos[MAX_VBO_FLAG];

        VaoVec  mVaos;

        VertexBufferPacked  *mDrawId;

    protected:
        virtual VertexBufferPacked* createVertexBufferImpl( size_t numElements,
                                                            uint32 bytesPerElement,
                                                            BufferType bufferType,
                                                            void *initialData, bool keepAsShadow,
                                                            const VertexElement2Vec &vertexElements );

        virtual void destroyVertexBufferImpl( VertexBufferPacked *vertexBuffer );

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

        virtual VertexArrayObject* createVertexArrayObjectImpl(
                                                        const VertexBufferPackedVec &vertexBuffers,
                                                        IndexBufferPacked *indexBuffer,
                                                        OperationType opType );

        virtual void destroyVertexArrayObjectImpl( VertexArrayObject *vao );

        static VboFlag bufferTypeToVboFlag( BufferType bufferType );

    public:
        NULLVaoManager();
        virtual ~NULLVaoManager();

        bool supportsArbBufferStorage(void) const       { return false; }

        /** Creates a new staging buffer and adds it to the pool. @see getStagingBuffer.
        @remarks
            The returned buffer starts with a reference count of 1. You should decrease
            it when you're done using it.
        */
        virtual StagingBuffer* createStagingBuffer( size_t sizeBytes, bool forUpload );

        virtual AsyncTicketPtr createAsyncTicket( BufferPacked *creator, StagingBuffer *stagingBuffer,
                                                  size_t elementStart, size_t elementCount );

        virtual void _update(void);

        /// Returns the current frame # (which wraps to 0 every mDynamicBufferMultiplier
        /// times). But first stalls until that mDynamicBufferMultiplier-1 frame behind
        /// is finished.
        uint8 waitForTailFrameToFinish(void);
        virtual void waitForSpecificFrameToFinish( uint32 frameCount );
        virtual bool isFrameFinished( uint32 frameCount );
    };
}

#endif
