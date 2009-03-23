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
#ifndef __HardwareIndexBuffer__
#define __HardwareIndexBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"

namespace Ogre {

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
		    IndexType mIndexType;
		    size_t mNumIndexes;
            size_t mIndexSize;

	    public:
		    /// Should be called by HardwareBufferManager
		    HardwareIndexBuffer(IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage,
                bool useSystemMemory, bool useShadowBuffer);
            ~HardwareIndexBuffer();
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

