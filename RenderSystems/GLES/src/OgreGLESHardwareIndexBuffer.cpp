/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESHardwareIndexBuffer.h"
#include "OgreGLESHardwareBufferManager.h"
#include "OgreException.h"

namespace Ogre {
    GLESHardwareIndexBuffer::GLESHardwareIndexBuffer(HardwareBufferManagerBase* mgr, 
													 IndexType idxType,
                                                     size_t numIndexes,
                                                     HardwareBuffer::Usage usage,
                                                     bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, useShadowBuffer)
    {
		if (idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLESHardwareIndexBuffer");
		}

		if (!useShadowBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only support with shadowBuffer",
                        "GLESHardwareIndexBuffer");
        }

        glGenBuffers(1, &mBufferId);

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Cannot create GL index buffer",
                "GLESHardwareIndexBuffer::GLESHardwareIndexBuffer");
        }

        // Initialise buffer and set usage
        clearData();
    }

    GLESHardwareIndexBuffer::~GLESHardwareIndexBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
    }

    void Ogre::GLESHardwareIndexBuffer::unlockImpl(void)
    {
        if (mLockedToScratch)
        {
            if (mScratchUploadOnUnlock)
            {
                    // have to write the data back to vertex buffer
                    writeData(mScratchOffset, mScratchSize, mScratchPtr,
                              mScratchOffset == 0 && mScratchSize == getSizeInBytes());
            }

            static_cast<GLESHardwareBufferManager*>(
                    HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Oly support lock to Scratch",
                        "GLESHardwareIndexBuffer::unlockImpl");
        }
    }

    void* GLESHardwareIndexBuffer::lockImpl(size_t offset,
                                            size_t length,
                                            LockOptions options)
    {
        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GLESHardwareIndexBuffer::lock");
        }

        void* retPtr = 0;

        if(length < OGRE_GL_MAP_BUFFER_THRESHOLD)
        {
            retPtr = static_cast<GLESHardwareBufferManager*>(
                    HardwareBufferManager::getSingletonPtr())->allocateScratch((uint32)length);
            if (retPtr)
            {
                mLockedToScratch = true;
                mScratchOffset = offset;
                mScratchSize = length;
                mScratchPtr = retPtr;
                mScratchUploadOnUnlock = (options != HBL_READ_ONLY);

                if (options != HBL_DISCARD)
                {
                    readData(offset, length, retPtr);
                }
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid Buffer lockSize",
                        "GLESHardwareIndexBuffer::lock");
        }

        return retPtr;
    }

    void GLESHardwareIndexBuffer::readData(size_t offset,
                                           size_t length,
                                           void* pDest)
    {
        if(mUseShadowBuffer)
        {
            void* srcData = mpShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mpShadowBuffer->unlock();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Not support read hardware buffer",
                        "GLESHardwareIndexBuffer::readData");
        }
    }

    void GLESHardwareIndexBuffer::writeData(size_t offset, size_t length,
                                            const void* pSource,
                                            bool discardWholeBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

        // Update the shadow buffer
        if (mUseShadowBuffer)
        {
            void* destData = mpShadowBuffer->lock(offset, length,
                                                  discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
            memcpy(destData, pSource, length);
            mpShadowBuffer->unlock();
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, pSource,
                         GLESHardwareBufferManager::getGLUsage(mUsage));
        }
        else
        {
            if (discardWholeBuffer)
            {
                clearData();
            }

            // Now update the real buffer
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, length, pSource);
        }
    }

    void GLESHardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mpShadowBuffer->lock(mLockStart, mLockSize,
                                                       HBL_READ_ONLY);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, srcData,
                             GLESHardwareBufferManager::getGLUsage(mUsage));
            }
            else
            {
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                                mLockStart, mLockSize, srcData);
            }

            mpShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }

    void GLESHardwareIndexBuffer::clearData(void)
    {
        void *ptr;

        ptr = malloc(mSizeInBytes);
        memset(ptr, 0, mSizeInBytes);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, ptr,
                     GLESHardwareBufferManager::getGLUsage(mUsage));

        free(ptr);
    }
}
