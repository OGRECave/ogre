/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreStableHeaders.h"
#include "Vao/OgreBufferInterface.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreStagingBuffer.h"

namespace Ogre
{
    BufferInterface::BufferInterface() :
        mBuffer( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void BufferInterface::upload( const void *data, size_t elementStart, size_t elementCount )
    {
        if( mBuffer->mBufferType >= BT_DYNAMIC_DEFAULT )
        {
            assert( mBuffer->mMappingState == MS_UNMAPPED );
            void *dstData = this->map( elementStart, elementCount, mBuffer->mMappingState );
            memcpy( dstData, data, elementCount * mBuffer->mBytesPerElement );
            this->unmap( UO_UNMAP_ALL );
        }
        else
        {
            size_t bytesPerElement = mBuffer->mBytesPerElement;

            //Get a staging buffer
            VaoManager *vaoManager = mBuffer->mVaoManager;
            StagingBuffer *stagingBuffer = vaoManager->getStagingBuffer( elementCount * bytesPerElement,
                                                                         true );

            //Map and memcpy the data (CPU -> GPU)
            void *dstData = stagingBuffer->map( elementCount * bytesPerElement );
            memcpy( dstData, data, elementCount * bytesPerElement );
            //Copy data from Staging to real buffer (GPU -> GPU)
            stagingBuffer->unmap( StagingBuffer::Destination( mBuffer,
                                                              elementStart * bytesPerElement,
                                                              0,
                                                              elementCount * bytesPerElement ) );
            stagingBuffer->removeReferenceCount();
        }
    }
    //-----------------------------------------------------------------------------------
}
