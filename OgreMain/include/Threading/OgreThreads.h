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

#ifndef __Threads_H__
#define __Threads_H__

#include "OgreSharedPtr.h"

#if defined(__i386) || defined(_M_IX86)
    // Calling conventions are needed for x86 (32-bit ONLY) CPUs
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        #define OGRE_THREAD_CALL_CONVENTION _OGRE_SIMD_ALIGN_ATTRIBUTE __stdcall
    #elif OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG
        #define __cdecl __attribute__((__cdecl__))
        #define OGRE_THREAD_CALL_CONVENTION __cdecl
    #endif
#else
    #define OGRE_THREAD_CALL_CONVENTION
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    /// @See Threads::CreateThread for an example on how to use
    #define THREAD_DECLARE( threadFunction ) \
    unsigned long OGRE_THREAD_CALL_CONVENTION threadFunction##_internal( void *argName )\
    {\
        unsigned long retVal = 0;\
        Ogre::ThreadHandle *threadHandle( reinterpret_cast<Ogre::ThreadHandle*>( argName ) );\
        try {\
            retVal = threadFunction( threadHandle );\
        }\
        catch( ... )\
        {\
        }\
        delete threadHandle;\
        return retVal;\
    }
#else
    /// @See Threads::CreateThread for an example on how to use
    #define THREAD_DECLARE( threadFunction ) \
    void* OGRE_THREAD_CALL_CONVENTION threadFunction##_internal( void *argName )\
    {\
        unsigned long retVal = 0;\
        Ogre::ThreadHandle *threadHandle( reinterpret_cast<Ogre::ThreadHandle*>( argName ) );\
        try {\
            retVal = threadFunction( threadHandle );\
        }\
        catch( ... )\
        {\
        }\
        delete threadHandle;\
        \
        return (void*)retVal;\
    }
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    //No need to include the heavy windows.h header for something like this!
    typedef void* HANDLE;
#else
    #include <pthread.h>
#endif

namespace Ogre
{
    class _OgreExport ThreadHandle
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        HANDLE  mThread;
#else
        pthread_t mThread;
#endif
        size_t  mThreadIdx;
        void    *mUserParam;

    public:
        ThreadHandle( size_t threadIdx, void *userParam );
        ~ThreadHandle();

        size_t getThreadIdx() const         { return mThreadIdx; }
        void* getUserParam() const          { return mUserParam; }

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        /// Internal use
        void _setOsHandle( HANDLE handle )  { mThread = handle; }
        /// Internal use
        HANDLE _getOsHandle() const         { return mThread; }
#else
        /// Internal use
        void _setOsHandle( pthread_t &handle )  { mThread = handle; }
        /// Internal use
        pthread_t _getOsHandle() const          { return mThread; }
#endif
    };

    typedef SharedPtr<ThreadHandle> ThreadHandlePtr;
    typedef vector<ThreadHandlePtr>::type ThreadHandleVec;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    typedef unsigned long (OGRE_THREAD_CALL_CONVENTION *THREAD_ENTRY_POINT)( void *lpThreadParameter );
#else
    typedef void* (OGRE_THREAD_CALL_CONVENTION *THREAD_ENTRY_POINT)( void *lpThreadParameter );
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    typedef uint32 TlsHandle;
#else
    typedef pthread_key_t TlsHandle;
#endif

    #define OGRE_TLS_INVALID_HANDLE 0xFFFFFFFF

    class _OgreExport Threads
    {
    public:
        #define THREAD_GET( threadFunction ) threadFunction##_internal
        /** Creates and executes a new thread
        @remarks
            The function to execute must be declared via THREAD_DECLARE, and the first argument
            must be provided through THREAD_GET. The following example shows how to use:

            unsigned long myOwnThread( ThreadHandle *ownThreadHandle )
            {
                //Should print "Hello from thread 50"
                printf( "Hello from thread %i", ownThreadHandle.getThreadIdx() );
                return 0;
            }
            THREAD_DECLARE( myOwnThread );

            int main()
            {
                ThreadHandle handle = CreateThread( THREAD_GET( myOwnThread ), 50, 0 );
                WaitForThreads( 1, handle );
                return 0;
            }

            It is not possible to retrieve the return value of the function because it's not
            portable (and trying to emulate it induces to easy-to-cause memory leaks; as
            we're dealing with C functions + potential race conditions, not C++)
        @param entryPoint
            Function name of the entry point. VERY IMPORTANT: The entry point must be provided
            by THREAD_GET. Do not use a function directly, otherwise there will be memory leaks!
        @param threadIdx
            Optional index for this thread (ie. when you have many worker threads to work on a
            section of data.
        @param param
            Optional argument to be passed.
        @return
            Handle to created thread.
        */
        static ThreadHandlePtr CreateThread( THREAD_ENTRY_POINT entryPoint,
                                             size_t threadIdx, void *param );

        /** Waits until all threads are finished
        @param numThreadInfos
            Number of ThreadHandle passed in the array as 'threadHandles'
        @param threadHandles
            Array of numThreadHandles or more ThreadHandle
        @remarks
            Don't pass more than 128 handles per call
        */
        static void WaitForThreads( size_t numThreadHandles, const ThreadHandlePtr *threadHandles );
        static void WaitForThreads( const ThreadHandleVec &threadHandles );

        /// Sleeps for a **minimum** of the specified time of milliseconds. Actual time spent
        /// sleeping may vary widely depending on OS and other variables. Do not feed 0.
        static void Sleep( uint32 milliseconds );

        /** Allocates a Thread Local Storage handle to use
        @param outTls [out]
            Handle to TLS.
            On failure this handle is set to OGRE_TLS_INVALID_HANDLE
        @return
            True on success, false on failure.
            TLS allocation can fail if the system ran out of handles,
            or it ran out of memory.
        */
        static bool CreateTls( TlsHandle *outTls );

        /** Destroys a Thread Local Storage handle created with CreateTls
        @param tlsHandle
            Handle to destroy
        */
        static void DestroyTls( TlsHandle tlsHandle );

        static void SetTls( TlsHandle tlsHandle, void *value );
        static void* GetTls( TlsHandle tlsHandle );
    };
}

#endif
