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

#ifndef __GLDefaultHardwareBufferManager_H__
#define __GLDefaultHardwareBufferManager_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

    /// Specialisation of HardwareVertexBuffer for emulation
    class _OgreGLExport GLDefaultHardwareVertexBuffer : public HardwareVertexBuffer 
    {
	protected:
		unsigned char* mpData;
        /// @copydoc HardwareBuffer::lock
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /// @copydoc HardwareBuffer::unlock
        void unlockImpl(void);

    public:
		GLDefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
            HardwareBuffer::Usage usage);
        ~GLDefaultHardwareVertexBuffer();
        /// @copydoc HardwareBuffer::readData
        void readData(size_t offset, size_t length, void* pDest);
        /// @copydoc HardwareBuffer::writeData

        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
        /** Override HardwareBuffer to turn off all shadowing. */
        void* lock(size_t offset, size_t length, LockOptions options);
        /** Override HardwareBuffer to turn off all shadowing. */
		void unlock(void);

        //void* getDataPtr(void) const { return (void*)mpData; }
        void* getDataPtr(size_t offset) const { return (void*)(mpData + offset); }
    };

	/// Specialisation of HardwareIndexBuffer for emulation
    class _OgreGLExport GLDefaultHardwareIndexBuffer : public HardwareIndexBuffer
    {
	protected:
		unsigned char* mpData;
        /// @copydoc HardwareBuffer::lock
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /// @copydoc HardwareBuffer::unlock
        void unlockImpl(void);
    public:
		GLDefaultHardwareIndexBuffer(IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage);
        ~GLDefaultHardwareIndexBuffer();
        /// @copydoc HardwareBuffer::readData
        void readData(size_t offset, size_t length, void* pDest);
        /// @copydoc HardwareBuffer::writeData
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
        /** Override HardwareBuffer to turn off all shadowing. */
        void* lock(size_t offset, size_t length, LockOptions options);
        /** Override HardwareBuffer to turn off all shadowing. */
        void unlock(void);

        void* getDataPtr(size_t offset) const { return (void*)(mpData + offset); }
    };

	/** Specialisation of HardwareBufferManager to emulate hardware buffers.
	@remarks
		You might want to instantiate this class if you want to utilise
		classes like MeshSerializer without having initialised the 
		rendering system (which is required to create a 'real' hardware
		buffer manager.
	*/
	class _OgreGLExport GLDefaultHardwareBufferManagerBase : public HardwareBufferManagerBase
	{
    public:
        GLDefaultHardwareBufferManagerBase();
        ~GLDefaultHardwareBufferManagerBase();
        /// Creates a vertex buffer
		HardwareVertexBufferSharedPtr 
            createVertexBuffer(size_t vertexSize, size_t numVerts, 
				HardwareBuffer::Usage usage, bool useShadowBuffer = false);
		/// Create a hardware vertex buffer
		HardwareIndexBufferSharedPtr 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
				HardwareBuffer::Usage usage, bool useShadowBuffer = false);
		/// Create a render to vertex buffer
		RenderToVertexBufferSharedPtr createRenderToVertexBuffer();

    };

	/// GLDefaultHardwareBufferManagerBase as a Singleton
	class _OgreGLExport GLDefaultHardwareBufferManager : public HardwareBufferManager
	{
	public:
		GLDefaultHardwareBufferManager()
			: HardwareBufferManager(OGRE_NEW GLDefaultHardwareBufferManagerBase()) 
		{

		}
		~GLDefaultHardwareBufferManager()
		{
			OGRE_DELETE mImpl;
		}
	};
}

#endif
