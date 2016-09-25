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

#ifndef _Ogre_UavBufferPacked_H_
#define _Ogre_UavBufferPacked_H_

#include "Vao/OgreBufferPacked.h"
#include "OgrePixelFormat.h"

namespace Ogre
{
    /** Represents UAV buffers (also known as SSBOs in OpenGL)
        Uav buffers are supported in DX10/DX10.1 hardware (with up to date drivers).
    */
    class _OgreExport UavBufferPacked : public BufferPacked
    {
    protected:
        uint32  mBindFlags; /// @see BufferBindFlags
        vector<TexBufferPacked*>::type  mTexBufferViews;

        virtual TexBufferPacked* getAsTexBufferImpl( PixelFormat pixelFormat ) = 0;

    public:
        UavBufferPacked( size_t internalBufferStartBytes, size_t numElements, uint32 bytesPerElement,
                         uint32 bindFlags, void *initialData, bool keepAsShadow,
                         VaoManager *vaoManager, BufferInterface *bufferInterface );
        virtual ~UavBufferPacked();

        virtual BufferPackedTypes getBufferPackedType(void) const   { return BP_TYPE_UAV; }

        //ConstBufferPacked* getAsConstBuffer(void);

        /** Returns a TexBufferPacked for binding to the GPU as a texture w/ read-only access.
            Buffer must've been created with BB_FLAG_TEX.
        @remarks
            Don't try to use readRequest or map the returned pointer. All major operations
            except binding to the shader should be done through the UavBufferPacked.
        @par
            If this function was already called, it will return the same pointer as the
            last time (though performs an O(N) search), otherwise returns a new pointer
        @return
            A TexBufferPacked to be used to bind to the different stages. Do not destroy
            this buffer via VaoManager::destroyTexBuffer. @see destroyTexBufferView
        */
        TexBufferPacked* getAsTexBufferView( PixelFormat pixelFormat );

        /// Frees memory from a view created by getAsTexBufferView
        /// Does nothing if a view of the given pixel format did not exist.
        void destroyTexBufferView( PixelFormat pixelFormat );

        /// @see destroyTexBufferView, this one does it on all the creates views.
        void destroyAllTexBufferViews(void);

        /** Binds the texture buffer to the given slot in the
            Vertex/Pixel/Geometry/Hull/Domain/Compute Shader
        @remarks
            Not all RS API separate by shader stage. For best compatibility,
            don't assign two different buffers at the same slot for different
            stages (just leave the slot empty on the stages you don't use).
        @param slot
            The slot to asign this constant buffer. In D3D11 it's called 'slot'.
            In GLSL it's called it's called 'binding'
        @param offset
            0-based offset. It is possible to bind a region of the buffer.
            Offset needs to be aligned. You can query the RS capabilities for
            the alignment, however 256 bytes is the maximum allowed alignment
            per the OpenGL specification, making it a safe bet to hardcode.
        @param sizeBytes
            Size in bytes to bind the tex buffer. When zero,
            binds from offset until the end of the buffer.
        */
//        virtual void bindBufferVS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
//        virtual void bindBufferPS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
//        virtual void bindBufferGS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
//        virtual void bindBufferDS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
//        virtual void bindBufferHS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
        virtual void bindBufferCS( uint16 slot, size_t offset=0, size_t sizeBytes=0 ) = 0;
    };
}

#endif
