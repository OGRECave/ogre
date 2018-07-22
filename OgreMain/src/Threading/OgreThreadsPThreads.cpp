/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "Threading/OgreThreads.h"

namespace Ogre
{
    ThreadHandle::ThreadHandle( size_t threadIdx, void *userParam ) :
        mThreadIdx( threadIdx ),
        mUserParam( userParam )
    {
    }
    //-----------------------------------------------------------------------------------
    ThreadHandle::~ThreadHandle()
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------

    ThreadHandlePtr Threads::CreateThread( THREAD_ENTRY_POINT entryPoint, size_t threadIdx, void *param )
    {
        ThreadHandle *threadArg( new ThreadHandle( threadIdx, param ) );
        ThreadHandlePtr retVal( new ThreadHandle( threadIdx, param ) );
        pthread_t threadId;
        pthread_create( &threadId, NULL, entryPoint, threadArg );
        retVal->_setOsHandle( threadId );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Threads::WaitForThreads( size_t numThreadHandles, const ThreadHandlePtr *threadHandles )
    {
        assert( numThreadHandles < 128 );

        for( size_t i=0; i<numThreadHandles; ++i )
            pthread_join( threadHandles[i]->_getOsHandle(), NULL );
    }
    //-----------------------------------------------------------------------------------
    void Threads::WaitForThreads( const ThreadHandleVec &threadHandles )
    {
        if( !threadHandles.empty() )
            Threads::WaitForThreads( threadHandles.size(), &threadHandles[0] );
    }
    //-----------------------------------------------------------------------------------
    void Threads::Sleep( uint32 milliseconds )
    {
        timespec timeToSleep;
        timeToSleep.tv_nsec = (milliseconds % 1000) * 1000000;
        timeToSleep.tv_sec  = milliseconds / 1000;
        nanosleep( &timeToSleep, 0 );
    }
    //-----------------------------------------------------------------------------------
    bool Threads::CreateTls( TlsHandle *outTls )
    {
        int result = pthread_key_create( outTls, NULL );
        if( result )
            *outTls = OGRE_TLS_INVALID_HANDLE;

        return result == 0;
    }
    //-----------------------------------------------------------------------------------
    void Threads::DestroyTls( TlsHandle tlsHandle )
    {
        pthread_key_delete( tlsHandle );
    }
    //-----------------------------------------------------------------------------------
    void Threads::SetTls( TlsHandle tlsHandle, void *value )
    {
        pthread_setspecific( tlsHandle, value );
    }
    //-----------------------------------------------------------------------------------
    void* Threads::GetTls( TlsHandle tlsHandle )
    {
        return pthread_getspecific( tlsHandle );
    }
}
