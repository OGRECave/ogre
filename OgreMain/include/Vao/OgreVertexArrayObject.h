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

    /// When cloning Vaos, some vertex buffers are used multiple times for LOD'ing
    /// purposes (only the IndexBuffer changes). This maps an old VertexBuffer
    /// to a new VertexBuffer to know when to reuse an existing one while cloning.
    typedef map<VertexBufferPacked*, VertexBufferPacked*>::type SharedVertexBufferMap;

    /** Vertex array objects (Vaos) are immutable objects that describe a
        combination of vertex buffers and index buffer with a given operation
        type. Once created, they can't be modified. You have to destroy them
        and create a new one.
    @remarks
        If the VertexArrayObject contains one BT_IMMUTABLE buffer (i.e. one of
        the vertex buffers or the index buffer) then despite the immutability
        of this class, the internal values of mVaoName & mRenderQueueId may
        be changed automatically by the VaoManager as it performs maintenance
        and cleanups of these type of buffers (in practice only affects D3D11).
        Don't rely on the contents of these two variables if the Vao contains
    */
    struct _OgreExport VertexArrayObject : public VertexArrayObjectAlloc
    {
        friend class RenderQueue;
        friend class RenderSystem;
        friend class D3D11RenderSystem;
        friend class GL3PlusRenderSystem;
        friend class MetalRenderSystem;

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

        /// Used to generate the PSO (@see HlmsPso). RenderSystem-specific quirks:
        ///     In OpenGL mInputLayoutId is always 0. mVaoName changes when either the
        ///     layout or the internal vertex/index buffer changes
        ///
        ///     In the rest of the systems, same vertex layouts share the same ID,
        ///     mVaoName changes when the vertex/index buffer needs to change.
        uint16 mInputLayoutId;

        uint32                  mPrimStart;
        uint32                  mPrimCount;
        VertexBufferPackedVec   mVertexBuffers;
        IndexBufferPacked       *mIndexBuffer;

        VertexBufferPacked      *mBaseVertexBuffer;

        /// The type of operation to perform
        OperationType mOperationType;

    public:
        VertexArrayObject( uint32 vaoName, uint32 renderQueueId, uint16 inputLayoutId,
                           const VertexBufferPackedVec &vertexBuffers,
                           IndexBufferPacked *indexBuffer,
                           OperationType operationType );

        uint32 getRenderQueueId(void) const                             { return mRenderQueueId; }
        uint32 getVaoName(void) const                                   { return mVaoName; }
        uint16 getInputLayoutId(void) const                             { return mInputLayoutId; }

        const VertexBufferPackedVec& getVertexBuffers(void) const       { return mVertexBuffers; }
        IndexBufferPacked* getIndexBuffer(void) const                   { return mIndexBuffer; }

        OperationType getOperationType(void) const { return mOperationType; }

        uint32 getPrimitiveStart(void) const                            { return mPrimStart; }
        uint32 getPrimitiveCount(void) const                            { return mPrimCount; }

        /** Limits the range of triangle primitives that is rendered.
            For VAOs with index buffers, this controls the index start & count,
            akin to indexStart & indexCount from the v1 objects.
        @par
            For VAOs with no index buffers, this controls the vertex start & count,
            akin to vertexStart & vertexCount from the v1 objects.
        @remarks
            An exception is thrown if primStart, or primStart + primCount are
            out of the half open range:
            [0; mIndexBuffer->getNumElements()) or [0; mVertexBuffers[0]->getNumElements())
        @par
            Parameters are always in elements (indices or vertices)
        */
        void setPrimitiveRange( uint32 primStart, uint32 primCount );

        /** Returns the entire VertexElement2 descriptor in the vertex buffers.
        @param semantic
            Semantic to look for.
        @param outIndex
            The index to mVertexBuffers[index] if it's found.
            Otherwise the value is left untouched.
        @param outIndex
            The offset in bytes to retrieve the element in the vertex data.
            If the semantic isn't found, the value is left untouched.
        @param repeat
            The number of times to skip a semantic before returning the hit. Useful
            when you have more than one VES_TEXTURE_COORDINATES for example.
            Set repeat = 0 to retrieve the first set of VES_TEXTURE_COORDINATES
            Set repeat = 1 to retrieve the second set of VES_TEXTURE_COORDINATES
            etc.
        @return
            Null if not found. The returned pointer might be invalidated by future calls
            (e.g. something happens to the vertex buffer or the Vao) although this is
            strange since in general these objects are immutable once they've been
            constructed.
        */
        const VertexElement2* findBySemantic( VertexElementSemantic semantic, size_t &outIndex,
                                              size_t &outOffset, size_t repeat=0 ) const;

        /// Gets the combined vertex declaration of all the vertex buffers. Note that we
        /// iterate through all of them and allocate the vector. You should cache
        /// the result rather than calling this function frequently.
        VertexElement2VecVec getVertexDeclaration(void) const;

        static VertexElement2VecVec getVertexDeclaration( const VertexBufferPackedVec &vertexBuffers );

        /** Clones the vertex & index buffers and creates a new VertexArrayObject.
            The only exception is when one of the vertex buffers is already in sharedBuffers,
            in which case the buffer in sharedBuffers.find(vertexBuffer)->second will be
            used without cloning (useful for cloning LODs).
        @param vaoManager
            The VaoManager needed to create the structures
        @param sharedBuffers [in/out]
            Maps old vertex buffers to new vertex buffers so that we can reuse them.
            Optional. Use a null pointer to disable this feature.
        @param vertexBufferType
            See BufferType. Must be set to a valid BufferType. Pass a negative
            value to keep the same type of the original buffer being cloned.
        @param indexBufferType
            See BufferType. Must be set to a valid BufferType. Pass a negative
            value to keep the same type of the original buffer being cloned.
        @return
            New cloned Vao.
        */
        VertexArrayObject* clone( VaoManager *vaoManager, SharedVertexBufferMap *sharedBuffers,
                                  int vertexBufferType = -1, int indexBufferType = -1 ) const;

        struct ReadRequests
        {
            VertexElementSemantic semantic;
            VertexElementType type;
            AsyncTicketPtr asyncTicket;
            /// Data is already offseted. To get the vertex location, perform (data - offset);
            char const *data;
            size_t offset;
            VertexBufferPacked const *vertexBuffer;

            ReadRequests( VertexElementSemantic _semantic ) :
                semantic( _semantic ), type(VET_FLOAT1), data( 0 ), offset( 0 ), vertexBuffer( 0 ) {}
        };

        typedef FastArray<ReadRequests> ReadRequestsArray;

        /** Utility to get multiple pointers & read specific elements of the vertex,
            even if they're in separate buffers.
            When two elements share the same buffer, only one ticket is created.

            Example usage:
                VertexArrayObject::ReadRequestsArray requests;
                requests.push_back( VertexArrayObject::ReadRequests( VES_POSITION ) );
                requests.push_back( VertexArrayObject::ReadRequests( VES_NORMALS ) );
                vao->readRequests( requests );
                vao->mapAsyncTickets( requests );

                for( size_t i=0; i<numVertices; ++i )
                {
                    float const *position = reinterpret_cast<const float*>( requests[0].data );
                    float const *normals  = reinterpret_cast<const float*>( requests[1].data );

                    requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
                    requests[1].data += requests[1].vertexBuffer->getBytesPerElement();
                }

                vao->unmapAsyncTickets( requests );
        @remarks
            Throws if an element couldn't be found.
        @param requests [in/out]
            Array filled with the semantic.
        @param skipRequestIfBufferHasShadowCopy
            Avoid generating the AsyncTicket if the buffer has a shadow copy. Useful if you
            want to read directly from the shadow copy instead of downloading from the GPU.
            The 'data' variable will be filled immediately if there's a shadow copy available,
            and mapAsyncTickets can be safely called even if skipRequestIfBufferHasShadowCopy=true
        */
        void readRequests( ReadRequestsArray &requests, size_t elementStart=0, size_t elementCount=0,
                           bool skipRequestIfBufferHasShadowCopy=false );

        /// Maps the buffers requested via @see readRequests
        static void mapAsyncTickets( ReadRequestsArray &tickets );

        /// Unmaps the buffers mapped via @see mapAsyncTickets
        static void unmapAsyncTickets( ReadRequestsArray &tickets );

        /// When a Vao doesn't have a vertex buffer, a dummy one is assigned for performance
        /// reasons (avoid checking if pointer is null, avoid crashing inside Ogre)
        static VertexBufferPacked msDummyVertexBuffer;
    };
}

#endif
