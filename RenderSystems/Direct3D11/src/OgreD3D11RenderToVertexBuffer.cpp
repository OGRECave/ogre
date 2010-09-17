/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreD3D11HardwareVertexBuffer.h"
#include "OgreRenderable.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreStringConverter.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11HLSLProgram.h"

namespace Ogre {

	D3D11RenderToVertexBuffer::D3D11RenderToVertexBuffer(D3D11Device & device, 
														 D3D11HardwareBufferManagerBase * bufManager) 
		: mDevice(device),  mFrontBufferIndex(-1), mBufManager(bufManager)
	{
		mVertexBuffers[0].setNull();
		mVertexBuffers[1].setNull();
	}

	D3D11RenderToVertexBuffer::~D3D11RenderToVertexBuffer(void)
	{
	}
	void D3D11RenderToVertexBuffer::getRenderOperation(RenderOperation& op)
	{
		op.operationType = mOperationType;
		op.useIndexes = false;
		op.vertexData = mVertexData;
	}
	void D3D11RenderToVertexBuffer::update(SceneManager* sceneMgr)
	{
		size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
		if (mVertexBuffers[0].isNull() || mVertexBuffers[0]->getSizeInBytes() != bufSize)
		{
			//Buffers don't match. Need to reallocate.
			mResetRequested = true;
		}
	
		//Single pass only for now
		Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);
		//Set pass before binding buffers to activate the GPU programs
		sceneMgr->_setPass(r2vbPass);
		

		RenderOperation renderOp;
		size_t targetBufferIndex;
		if (mResetRequested || mResetsEveryUpdate)
		{
			//Use source data to render to first buffer
			mSourceRenderable->getRenderOperation(renderOp);
			targetBufferIndex = 0;
		}
		else
		{
			//Use current front buffer to render to back buffer
			this->getRenderOperation(renderOp);
			targetBufferIndex = 1 - mFrontBufferIndex;
		}

		if (mVertexBuffers[targetBufferIndex].isNull() || 
			mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
		{
			reallocateBuffer(targetBufferIndex);
		}



		const D3D11HardwareVertexBuffer* d3d11buf = 
			static_cast<const D3D11HardwareVertexBuffer*>(mVertexBuffers[targetBufferIndex].getPointer());

		ID3D11Buffer * pVertexBuffers = d3d11buf->getD3DVertexBuffer();

		mDevice.GetImmediateContext()->SOSetTargets(1, &pVertexBuffers, 0);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set vertex buffer\nError Description:" + errorDescription,
				"D3D11RenderToVertexBuffer::getRenderOperation");
		}

		D3D11RenderSystem* targetRenderSystem = (D3D11RenderSystem*)Root::getSingleton().getRenderSystem();

		D3D11HLSLProgram* originalVertexProgram = targetRenderSystem->_getBoundVertexProgram();
		targetRenderSystem->bindGpuProgram(r2vbPass->getVertexProgram().getPointer());
		targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
			r2vbPass->getVertexProgramParameters(), GPV_ALL);

		D3D11HLSLProgram* originalGeometryProgram = targetRenderSystem->_getBoundGeometryProgram();
		targetRenderSystem->bindGpuProgram(r2vbPass->getGeometryProgram().getPointer());
		targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
			r2vbPass->getGeometryProgramParameters(), GPV_ALL);

		// no fragment program!
		mDevice.GetImmediateContext()->PSSetShader(NULL, NULL, 0);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set vertex buffer to NULL\nError Description:" + errorDescription,
				"D3D11RenderToVertexBuffer::getRenderOperation");
		}
	
		// save depth buffer check value 
		bool depthBufferCheckValue = targetRenderSystem->_getDepthBufferCheckEnabled();

		// disable depth test
		targetRenderSystem->_setDepthBufferCheckEnabled(false);

		targetRenderSystem->_render(renderOp);


		mVertexData->vertexCount = -1; // -1 means we will use DrawAuto later when rendering...

		//Switch the vertex binding if necessary
		if (targetBufferIndex != mFrontBufferIndex)
		{
			mVertexData->vertexBufferBinding->unsetAllBindings();
			mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
			mFrontBufferIndex = targetBufferIndex;
		}

		// get back to the original value
		targetRenderSystem->_setDepthBufferCheckEnabled(depthBufferCheckValue);

		// go back to the original shaders
		targetRenderSystem->bindGpuProgram(originalVertexProgram);
		targetRenderSystem->bindGpuProgram(originalGeometryProgram);

		// get back to normal render
		ID3D11Buffer* dummyBuffer = NULL;
		mDevice.GetImmediateContext()->SOSetTargets(1, &dummyBuffer, 0);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set vertex buffer to NULL\nError Description:" + errorDescription,
				"D3D11RenderToVertexBuffer::getRenderOperation");
		}

		//Clear the reset flag
		mResetRequested = false;

	}
	//-----------------------------------------------------------------------------
	void D3D11RenderToVertexBuffer::reallocateBuffer(size_t index)
	{
		assert(index == 0 || index == 1);
		if (!mVertexBuffers[index].isNull())
		{
			mVertexBuffers[index].setNull();
		}
		
		mVertexBuffers[index] = mBufManager->createStreamOutputVertexBuffer(
			mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY
			);
	}
}
