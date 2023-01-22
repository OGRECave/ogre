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
#include "OgreD3D11Device.h"
#include "OgreD3D11HardwareBufferManager.h"
#include "OgreD3D11RenderToVertexBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreRenderable.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11HLSLProgram.h"

namespace Ogre {

    D3D11RenderToVertexBuffer::D3D11RenderToVertexBuffer(D3D11Device & device, 
                                                         D3D11HardwareBufferManagerBase * bufManager) 
        : mDevice(device)
        , mBufManager(bufManager)
        , mpGeometryShader(0)
    {
        mVertexBuffers[0].reset();
        mVertexBuffers[1].reset();
    }

    D3D11RenderToVertexBuffer::~D3D11RenderToVertexBuffer(void)
    {
    }

    void D3D11RenderToVertexBuffer::setupGeometryShaderLinkageToStreamOut(Pass* pass)
    {
        static bool done =  false;

        if (done || !pass->hasGeometryProgram())
            return;

        const GpuProgramPtr& program = pass->getGeometryProgram();

        D3D11HLSLProgram* dx11Program = static_cast<D3D11HLSLProgram*>(program->_getBindingDelegate());
        dx11Program->reinterpretGSForStreamOut();

        done = true;
        
    }
    void D3D11RenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        Ogre::Pass* r2vbPass = derivePass(sceneMgr);

        setupGeometryShaderLinkageToStreamOut(r2vbPass);

        size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        if (!mVertexBuffers[0] || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        {
            //Buffers don't match. Need to reallocate.
            mResetRequested = true;
        }

        RenderOperation renderOp;
        auto targetBufferIndex = mTargetBufferIndex;
        if (mResetRequested || mResetsEveryUpdate)
        {
            //Use source data to render to first buffer
            mSourceRenderable->getRenderOperation(renderOp);
        }
        else
        {
            getRenderOperation(renderOp);
        }

        if (!mVertexBuffers[targetBufferIndex] || 
            mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
        {
            reallocateBuffer(targetBufferIndex);
        }

        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();

        //Draw the object
        auto vertexBuffer = mVertexBuffers[targetBufferIndex]->_getImpl<D3D11HardwareBuffer>();
    
        UINT offset[1] = { 0 };
        ID3D11Buffer* iBuffer[1];
        iBuffer[0] = vertexBuffer->getD3DBuffer();
        mDevice.GetImmediateContext()->SOSetTargets( 1, iBuffer, offset );
        // Remove fragment program
        mDevice.GetImmediateContext()->PSSetShader(NULL, NULL, 0);

        targetRenderSystem->_render(renderOp);  

        //Switch the vertex binding
        mVertexData->vertexBufferBinding->unsetAllBindings();
        mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
        mTargetBufferIndex = mTargetBufferIndex == 0 ? 1 : 0;

        // Remove stream output buffer 
        iBuffer[0]=NULL;
        mDevice.GetImmediateContext()->SOSetTargets( 1, iBuffer, offset );
        //Clear the reset flag
        mResetRequested = false;

        // Enable DrawAuto
        mVertexData->vertexCount = -1;
    }
    //-----------------------------------------------------------------------------
    void D3D11RenderToVertexBuffer::reallocateBuffer(size_t index)
    {
        assert(index == 0 || index == 1);
        if (mVertexBuffers[index])
        {
            mVertexBuffers[index].reset();
        }

        mVertexBuffers[index] = mBufManager->createStreamOutputVertexBuffer(
            mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY
            );
    }
}
