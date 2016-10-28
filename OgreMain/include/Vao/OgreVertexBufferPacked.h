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

#ifndef _Ogre_VertexBufferPacked_H_
#define _Ogre_VertexBufferPacked_H_

#include "Vao/OgreBufferPacked.h"
#include "Vao/OgreVertexElements.h"

namespace Ogre
{
    struct VertexElement2
    {
        /// The type of element
        VertexElementType mType;
        /// The meaning of the element
        VertexElementSemantic mSemantic;

        /// The number of instances to draw using the same per-instance data before
        /// advancing in the buffer by one element. This value must be 0 for an
        /// element that contains per-vertex data
        uint32 mInstancingStepRate;

        VertexElement2( VertexElementType type, VertexElementSemantic semantic ) :
            mType( type ), mSemantic( semantic ), mInstancingStepRate( 0 ) {}

        bool operator == ( const VertexElement2 _r ) const
        {
            return mType == _r.mType && mSemantic == _r.mSemantic &&
                    mInstancingStepRate == _r.mInstancingStepRate;
        }

        bool operator == ( VertexElementSemantic semantic ) const
        {
            return mSemantic == semantic;
        }

        /// Warning: Beware a VertexElement2Vec shouldn't be sorted.
        /// The order in which they're pushed into the vector defines the offsets
        /// in the vertex buffer. Altering the order would cause the GPU to
        /// to read garbage when trying to interpret the vertex buffer
        /// This operator exists because it's useful when implementing
        /// a '<' operator in other structs that contain VertexElement2Vecs
        /// (see HlmsPso)
        bool operator < ( const VertexElement2 &_r ) const
        {
            if( this->mType < _r.mType ) return true;
            if( this->mType > _r.mType ) return false;

            if( this->mSemantic < _r.mSemantic ) return true;
            if( this->mSemantic > _r.mSemantic ) return false;

            return this->mInstancingStepRate < _r.mInstancingStepRate;
        }
    };

    typedef vector<VertexElement2>::type VertexElement2Vec;
    typedef vector<VertexElement2Vec>::type VertexElement2VecVec;

    class _OgreExport VertexBufferPacked : public BufferPacked
    {
    protected:
        VertexElement2Vec mVertexElements;

        /// Multisource VertexArrayObjects is when VertexArrayObject::mVertexBuffers.size() is greater
        /// than 1 (i.e. have position in one vertex buffer, UVs in another.)
        /// A VertexBuffer created for multisource can be used/bound for rendering with just one buffer
        /// source (i.e. just bind the position buffer during the shadow map pass) or bound together
        /// with buffers that have the same mMultiSourceId and mMultiSourcePool.
        /// But a VertexBuffer not created for multisource cannot be bound together with other buffers.
        ///
        /// Before you're tempted into creating all your Vertex Buffers as multisource indiscriminately,
        /// the main issue is that multisource vertex buffers can heavily fragment GPU memory managed
        /// by the VaoManager (unless you know in advance the full number of vertices you need per
        /// vertex declaration and reserve this size) or waste a lot of GPU RAM, and/or increase
        /// the draw call count.
        size_t                      mMultiSourceId;
        MultiSourceVertexBufferPool *mMultiSourcePool;
        uint8                       mSourceIdx;

    public:
        VertexBufferPacked( size_t internalBufferStartBytes, size_t numElements, uint32 bytesPerElement,
                            uint32 numElementsPadding, BufferType bufferType,
                            void *initialData, bool keepAsShadow,
                            VaoManager *vaoManager, BufferInterface *bufferInterface,
                            const VertexElement2Vec &vertexElements, size_t multiSourceId,
                            MultiSourceVertexBufferPool *multiSourcePool, uint8 sourceIdx );
        ~VertexBufferPacked();

        virtual BufferPackedTypes getBufferPackedType(void) const   { return BP_TYPE_VERTEX; }

        const VertexElement2Vec& getVertexElements(void) const  { return mVertexElements; }

        size_t getMultiSourceId(void)                           { return mMultiSourceId; }

        /// Return value may be null
        MultiSourceVertexBufferPool* getMultiSourcePool(void)   { return mMultiSourcePool; }

        /// Source index reference assigned by the MultiSourceVertexBufferPool.
        /// This value does not restrict the fact that you can actually assign this buffer to
        /// another index (as long as it's with another buffer with the same multisource ID
        /// and pool). This value is for internal use.
        /// Always 0 for non-multisource vertex buffers.
        uint8 _getSourceIndex(void) const                       { return mSourceIdx; }
    };

    typedef vector<VertexBufferPacked*>::type VertexBufferPackedVec;
}

#endif
