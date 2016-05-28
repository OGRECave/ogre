/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "OgreMetalPrerequisites.h"

#include "Vao/OgreMetalAsyncTicket.h"
#include "Vao/OgreMetalVaoManager.h"

#include "Vao/OgreStagingBuffer.h"

#include "OgreMetalDevice.h"

#import <Metal/MTLCommandBuffer.h>

namespace Ogre
{
    MetalAsyncTicket::MetalAsyncTicket( BufferPacked *creator,
                                        StagingBuffer *stagingBuffer,
                                        size_t elementStart,
                                        size_t elementCount,
                                        MetalDevice *device ) :
        AsyncTicket( creator, stagingBuffer, elementStart, elementCount ),
        mFenceName( 0 ),
        mDevice( device )
    {
        mFenceName = dispatch_semaphore_create( 0 );

        __block dispatch_semaphore_t blockSemaphore = mFenceName;
        [mDevice->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
        {
            dispatch_semaphore_signal( blockSemaphore );
        }];
        //Flush now for accuracy with downloads.
        mDevice->commitAndNextCommandBuffer();
    }
    //-----------------------------------------------------------------------------------
    MetalAsyncTicket::~MetalAsyncTicket()
    {
        mFenceName = 0;
    }
    //-----------------------------------------------------------------------------------
    const void* MetalAsyncTicket::mapImpl(void)
    {
        if( mFenceName )
            mFenceName = MetalVaoManager::waitFor( mFenceName, mDevice );

        return mStagingBuffer->_mapForRead( mStagingBufferMapOffset,
                                            mElementCount * mCreator->getBytesPerElement() );
    }
    //-----------------------------------------------------------------------------------
    bool MetalAsyncTicket::queryIsTransferDone(void)
    {
        bool retVal = false;

        if( mFenceName )
        {
            //Ask to return immediately and tell us about the fence
            const long result = dispatch_semaphore_wait( mFenceName, DISPATCH_TIME_NOW );
            if( result == 0 )
            {
                mFenceName = 0;
                retVal = true;
            }
        }
        else
        {
            retVal = true;
        }

        return retVal;
    }
}
