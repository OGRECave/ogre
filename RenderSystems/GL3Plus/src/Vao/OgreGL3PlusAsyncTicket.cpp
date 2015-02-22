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

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreGL3PlusAsyncTicket.h"
#include "Vao/OgreGL3PlusVaoManager.h"

#include "Vao/OgreStagingBuffer.h"

namespace Ogre
{
    GL3PlusAsyncTicket::GL3PlusAsyncTicket( BufferPacked *creator,
                                            StagingBuffer *stagingBuffer,
                                            size_t elementStart,
                                            size_t elementCount ) :
        AsyncTicket( creator, stagingBuffer, elementStart, elementCount ),
        mFenceName( 0 )
    {
        //Base constructor has already called _asyncDownload. We should now place a fence.
        OCGE( mFenceName = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusAsyncTicket::~GL3PlusAsyncTicket()
    {
        if( mFenceName )
        {
            OCGE( glDeleteSync( mFenceName ) );
            mFenceName = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    const void* GL3PlusAsyncTicket::mapImpl(void)
    {
        if( mFenceName )
            mFenceName = GL3PlusVaoManager::waitFor( mFenceName );

        return mStagingBuffer->_mapForRead( mStagingBufferMapOffset,
                                            mElementCount * mCreator->getBytesPerElement() );
    }
    //-----------------------------------------------------------------------------------
    bool GL3PlusAsyncTicket::queryIsTransferDone(void)
    {
        bool retVal = false;

        if( mFenceName )
        {
            //Ask GL API to return immediately and tells us about the fence
            GLenum waitRet = glClientWaitSync( mFenceName, 0, 0 );
            if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED )
            {
                OCGE( glDeleteSync( mFenceName ) );
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
