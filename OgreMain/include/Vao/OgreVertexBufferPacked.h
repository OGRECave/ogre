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
#include "OgreHardwareVertexBuffer.h"

namespace Ogre
{
    struct VertexElement2
    {
        /// The type of element
        VertexElementType mType;
        /// The meaning of the element
        VertexElementSemantic mSemantic;

        VertexElement2( VertexElementType type, VertexElementSemantic semantic ) :
            mType( type ), mSemantic( semantic ) {}

        bool operator == ( const VertexElement2 _r ) const
        {
            return mType == _r.mType && mSemantic == _r.mSemantic;
        }
    };

    typedef vector<VertexElement2>::type VertexElement2Vec;
    typedef vector<VertexElement2Vec>::type VertexElement2VecVec;

    class VertexBufferPacked : public BufferPacked
    {
    protected:
        VertexElement2Vec mVertexElements;

        /// Multisource VertexArrayObjects is when VertexArrayObject::mVertexBuffers.size() is greater
        /// than 1 (i.e. have position in one vertex buffer, UVs in another.)
        /// A VertexBuffer created for multisource can be used/bound for rendering with just one buffer
        /// source (i.e. just bind the position buffer during the shadow map pass).
        /// But a VertexBuffer not created for multisource cannot be bound with together with other
        /// buffers.
        ///
        /// Before you're tempted into creating all your Vertex Buffers as multisource indiscriminately,
        /// the main issue is that multisource vertex buffers can heavily fragment GPU memory managed
        /// by the VaoManager (unless you know in advance the full number of vertices you need per
        /// vertex declaration and reserve this size) or waste a lot of GPU RAM, and/or increase
        /// the draw call count.
        bool mMultiSource;

    public:
        VertexBufferPacked( size_t internalBufferStart, size_t numElements, uint32 bytesPerElement,
                            BufferType bufferType, void *initialData, bool keepAsShadow,
                            VaoManager *vaoManager, BufferInterface *bufferInterface,
                            const VertexElement2Vec &vertexElements, bool multiSource );
        ~VertexBufferPacked();

        bool isMultiSource(void) const                          { return mMultiSource; }

        const VertexElement2Vec& getVertexElements(void) const  { return mVertexElements; }
    };

    typedef vector<VertexBufferPacked*>::type VertexBufferPackedVec;
}

#endif
