/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

