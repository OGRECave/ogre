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
#include "OgreGLHardwareIndexBuffer.h"
#include "OgreGLHardwareBufferManager.h"
#include "OgreException.h"

namespace Ogre {

	//---------------------------------------------------------------------
    GLHardwareIndexBuffer::GLHardwareIndexBuffer(IndexType idxType,
        size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
        : HardwareIndexBuffer(idxType, numIndexes, usage, false, useShadowBuffer)
    {
        glGenBuffersARB( 1, &mBufferId );

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Cannot create GL index buffer", 
                "GLHardwareIndexBuffer::GLHardwareIndexBuffer");
        }

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId);

        // Initialise buffer and set usage
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mSizeInBytes, NULL, 
            GLHardwareBufferManager::getGLUsage(usage));

        //std::cerr << "creating index buffer " << mBufferId << std::endl;
    }
	//---------------------------------------------------------------------
    GLHardwareIndexBuffer::~GLHardwareIndexBuffer()
    {
        glDeleteBuffersARB(1, &mBufferId);
    }
	//---------------------------------------------------------------------
    void* GLHardwareIndexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        GLenum access = 0;

        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Invalid attempt to lock an index buffer that has already been locked",
                    "GLHardwareIndexBuffer::lock");
        }

        
		void* retPtr = 0;

		GLHardwareBufferManager* glBufManager = static_cast<GLHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());

		// Try to use scratch buffers for smaller buffers
		if( length < glBufManager->getGLMapBufferThreshold() )
		{
			retPtr = glBufManager->allocateScratch((uint32)length);
			if (retPtr)
			{
				mLockedToScratch = true;
				mScratchOffset = offset;
				mScratchSize = length;
				mScratchPtr = retPtr;
				mScratchUploadOnUnlock = (options != HBL_READ_ONLY);

				if (options != HBL_DISCARD)
				{
					// have to read back the data before returning the pointer
					readData(offset, length, retPtr);
				}
			}
		}

		if (!retPtr)
		{
			glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId );
			// Use glMapBuffer
			if(options == HBL_DISCARD)
			{
				// Discard the buffer
				glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mSizeInBytes, NULL, 
					GLHardwareBufferManager::getGLUsage(mUsage));
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_ARB;
			else if (options == HBL_READ_ONLY)
				access = GL_READ_ONLY_ARB;
			else
				access = GL_READ_WRITE_ARB;

			void* pBuffer = glMapBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, access );

			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Index Buffer: Out of memory", 
					"GLHardwareIndexBuffer::lock");
			}

			// return offsetted
			retPtr = static_cast<void*>(
				static_cast<unsigned char*>(pBuffer) + offset);

			mLockedToScratch = false;

		}
		mIsLocked = true;
		return retPtr;
    }
	//---------------------------------------------------------------------
	void GLHardwareIndexBuffer::unlockImpl(void)
    {
		if (mLockedToScratch)
		{
			if (mScratchUploadOnUnlock)
			{
				// have to write the data back to vertex buffer
				writeData(mScratchOffset, mScratchSize, mScratchPtr, 
					mScratchOffset == 0 && mScratchSize == getSizeInBytes());
			}

			// deallocate from scratch buffer
			static_cast<GLHardwareBufferManager*>(
				HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

			mLockedToScratch = false;
		}
		else
		{

			glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId );

			if(!glUnmapBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB ))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Buffer data corrupted, please reload", 
					"GLHardwareIndexBuffer::unlock");
			}
		}

        mIsLocked = false;
    }
	//---------------------------------------------------------------------
    void GLHardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        if(mUseShadowBuffer)
        {
            // get data from the shadow buffer
            void* srcData = mpShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mpShadowBuffer->unlock();
        }
        else
        {
            glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId );
            glGetBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, offset, length, pDest);
        }
    }
	//---------------------------------------------------------------------
    void GLHardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource, bool discardWholeBuffer)
    {
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId );

        // Update the shadow buffer
        if(mUseShadowBuffer)
        {
            void* destData = mpShadowBuffer->lock(offset, length, 
                discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
            memcpy(destData, pSource, length);
            mpShadowBuffer->unlock();
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mSizeInBytes, pSource,
                GLHardwareBufferManager::getGLUsage(mUsage));
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mSizeInBytes, NULL,
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }

            // Now update the real buffer
            glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, offset, length, pSource);
        }
    }
	//---------------------------------------------------------------------
    void GLHardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mpShadowBuffer->lock(
                mLockStart, mLockSize, HBL_READ_ONLY);

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId);

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mSizeInBytes, srcData,
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }
            else
            {
                glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mLockStart, mLockSize, srcData);
            }

            mpShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
