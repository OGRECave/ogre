/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __D3D11DepthBuffer_H__
#define __D3D11DepthBuffer_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreDepthBuffer.h"

namespace Ogre 
{
    class _OgreD3D11Export D3D11DepthBuffer : public DepthBuffer
    {
    public:
        D3D11DepthBuffer( uint16 poolId, D3D11RenderSystem *renderSystem,
                            ID3D11DepthStencilView *depthBufferView,
                            uint32 width, uint32 height,
                            uint32 fsaa, uint32 multiSampleQuality, bool isManual );
        ~D3D11DepthBuffer();

        /// @copydoc DepthBuffer::isCompatible
        virtual bool isCompatible( RenderTarget *renderTarget ) const;

        ID3D11DepthStencilView* getDepthStencilView() const;
        /// internal method, gets called when the renderwindow was resized
        void _resized(ID3D11DepthStencilView *depthBufferView, uint32 width, uint32 height);

    protected:
        ID3D11DepthStencilView      *mDepthStencilView; //aka. actual "DepthBuffer"
        uint32                      mMultiSampleQuality;
        D3D11RenderSystem           *mRenderSystem;
    };
}
#endif
