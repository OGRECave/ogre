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
#ifndef __D3D10HARDWAREBUFFER_H__
#define __D3D10HARDWAREBUFFER_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwareBuffer.h"

namespace Ogre { 


	/** Base implementation of a D3D10 buffer, dealing with all the common
	aspects.
	*/
	class D3D10HardwareBuffer : public HardwareBuffer
	{
	public:
		enum BufferType
		{
			VERTEX_BUFFER,
			INDEX_BUFFER
		};
	protected:
		ID3D10Buffer* mlpD3DBuffer;
		bool mUseTempStagingBuffer;
		D3D10HardwareBuffer* mpTempStagingBuffer;
		bool mStagingUploadNeeded;
		BufferType mBufferType;
		D3D10Device & mDevice;
		D3D10_BUFFER_DESC mDesc;



		/** See HardwareBuffer. */
		void* lockImpl(size_t offset, size_t length, LockOptions options);
		/** See HardwareBuffer. */
		void unlockImpl(void);

	public:
		D3D10HardwareBuffer(BufferType btype, size_t sizeBytes, HardwareBuffer::Usage usage, 
			D3D10Device & device, bool useSystemMem, bool useShadowBuffer);
		~D3D10HardwareBuffer();
		/** See HardwareBuffer. */
		void readData(size_t offset, size_t length, void* pDest);
		/** See HardwareBuffer. */
		void writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer = false);
		/** See HardwareBuffer. We perform a hardware copy here. */
		void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
			size_t dstOffset, size_t length, bool discardWholeBuffer = false);

		/// Get the D3D-specific buffer
		ID3D10Buffer* getD3DBuffer(void) { return mlpD3DBuffer; }
	};


}



#endif

