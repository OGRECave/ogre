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

#ifndef _Ogre_VertexArrayObject_H_
#define _Ogre_VertexArrayObject_H_

#include "OgrePrerequisites.h"
#include "OgreVertexBufferPacked.h"
#include "OgreRenderOperation.h"

namespace Ogre
{
    typedef vector<VertexBufferPacked*>::type VertexBufferPackedVec;

    struct _OgreExport VertexArrayObject : public VertexArrayObjectAlloc
    {
        friend class RenderQueue;
        friend class RenderSystem;
        friend class GL3PlusRenderSystem;

    protected:
        /// ID of the internal vertex and index buffer layouts. If this ID
        /// doesn't change between two draw calls, then there is no need
        /// to reset the VAO (i.e. same vertex and index buffers are used)
        /// This ID may be shared by many VertexArrayObject instances.
        uint32 mVaoName;

        /// ID used for the RenderQueue to sort by VAOs. It's similar to
        /// mVaoName, but contains more information for sorting purposes.
        /// This ID may be shared by many VertexArrayObject instances.
        uint32 mRenderQueueId;

        uint32                  mFaceCount; /// For statistics
        VertexBufferPackedVec   mVertexBuffers;
        IndexBufferPacked       *mIndexBuffer;

        /// The type of operation to perform
        v1::RenderOperation::OperationType mOperationType;

    public:
        VertexArrayObject( uint32 vaoName, uint32 renderQueueId,
                           const VertexBufferPackedVec &vertexBuffers,
                           IndexBufferPacked *indexBuffer,
                           v1::RenderOperation::OperationType operationType );

        uint32 getRenderQueueId(void) const                             { return mRenderQueueId; }
        uint32 getVaoName(void) const                                   { return mVaoName; }

        const VertexBufferPackedVec& getVertexBuffers(void) const       { return mVertexBuffers; }
        IndexBufferPacked* getIndexBuffer(void) const                   { return mIndexBuffer; }
    };
}

#endif
