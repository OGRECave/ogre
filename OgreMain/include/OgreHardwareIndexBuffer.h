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
#ifndef __HardwareIndexBuffer__
#define __HardwareIndexBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"

namespace Ogre {
	class HardwareBufferManagerBase;

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Specialisation of HardwareBuffer for vertex index buffers, still abstract. */
    class _OgreExport HardwareIndexBuffer : public HardwareBuffer
    {
	    public:
		    enum IndexType {
			    IT_16BIT,
			    IT_32BIT
		    };

	    protected:
			HardwareBufferManagerBase* mMgr;
		    IndexType mIndexType;
		    size_t mNumIndexes;
            size_t mIndexSize;

	    public:
		    /// Should be called by HardwareBufferManager
		    HardwareIndexBuffer(HardwareBufferManagerBase* mgr, IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage,
                bool useSystemMemory, bool useShadowBuffer);
            ~HardwareIndexBuffer();
			/// Return the manager of this buffer, if any
			HardwareBufferManagerBase* getManager() const { return mMgr; }
    		/// Get the type of indexes used in this buffer
            IndexType getType(void) const { return mIndexType; }
            /// Get the number of indexes in this buffer
            size_t getNumIndexes(void) const { return mNumIndexes; }
            /// Get the size in bytes of each index
            size_t getIndexSize(void) const { return mIndexSize; }

		    // NB subclasses should override lock, unlock, readData, writeData
    };


    /** Shared pointer implementation used to share index buffers. */
    class _OgreExport HardwareIndexBufferSharedPtr : public SharedPtr<HardwareIndexBuffer>
    {
    public:
        HardwareIndexBufferSharedPtr() : SharedPtr<HardwareIndexBuffer>() {}
        explicit HardwareIndexBufferSharedPtr(HardwareIndexBuffer* buf);
    };
	/** @} */
	/** @} */
}
#endif

