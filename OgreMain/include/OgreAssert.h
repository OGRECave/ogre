/*
* Copyright (c) 2008, Power of Two Games LLC
* Copyright (c) 2018, Matias N. Goldberg (small enhancements, ported to other compilers)
* All rights reserved.
*
* THIS FILE WAS AUTOMATICALLY GENERATED USING AssertLib
*
* See https://bitbucket.org/dark_sylinc/AssertLib
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Power of Two Games LLC nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY POWER OF TWO GAMES LLC ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL POWER OF TWO GAMES LLC BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _OgreAssert_H_
#define _OgreAssert_H_

#include "OgrePrerequisites.h"
#include "debugbreak/debugbreak.h"

namespace Ogre
{
namespace Assert
{
    enum FailBehavior
    {
        Halt,
        Continue,
    };

    typedef FailBehavior (*Handler)( const char* condition,
                                     const char* msg,
                                     const char* file,
                                     int line );

    _OgreExport Handler GetHandler();
    _OgreExport void SetHandler( Handler newHandler );

    _OgreExport FailBehavior ReportFailure( const char* condition,
                                            const char* file,
                                            int line,
                                            const char* msg, ... );
}
}

#define OGRE_HALT() debug_break()
#define OGRE_UNUSED(x) do { (void)sizeof(x); } while(0)

#ifdef OGRE_ASSERTS_ENABLED
    #define OGRE_ASSERT(cond) \
        do \
        { \
            if (!(cond)) \
            { \
                if (Ogre::Assert::ReportFailure(#cond, __FILE__, __LINE__, 0) == \
                    Ogre::Assert::Halt) \
                    OGRE_HALT(); \
            } \
        } while(0)

    #if _MSC_VER
        #define OGRE_ASSERT_MSG(cond, msg, ...) \
            do \
            { \
                if (!(cond)) \
                { \
                    if (Ogre::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
                        Ogre::Assert::Halt) \
                        OGRE_HALT(); \
                } \
            } while(0)

        #define OGRE_ASSERT_FAIL(msg, ...) \
            do \
            { \
                if (Ogre::Assert::ReportFailure(0, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
                    Ogre::Assert::Halt) \
                OGRE_HALT(); \
            } while(0)
        #define OGRE_VERIFY_MSG(cond, msg, ...) OGRE_ASSERT_MSG(cond, msg, __VA_ARGS__)
    #else
        #define OGRE_ASSERT_MSG(cond, msg, ...) \
            do \
            { \
                if (!(cond)) \
                { \
                    if (Ogre::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), ##__VA_ARGS__) == \
                        Ogre::Assert::Halt) \
                        OGRE_HALT(); \
                } \
            } while(0)

        #define OGRE_ASSERT_FAIL(msg, ...) \
            do \
            { \
                if (Ogre::Assert::ReportFailure(0, __FILE__, __LINE__, (msg), ##__VA_ARGS__) == \
                    Ogre::Assert::Halt) \
                OGRE_HALT(); \
            } while(0)
        #define OGRE_VERIFY_MSG(cond, msg, ...) OGRE_ASSERT_MSG(cond, msg, ##__VA_ARGS__)
    #endif

    #define OGRE_VERIFY(cond) OGRE_ASSERT(cond)
#else
    #define OGRE_ASSERT(condition) ((void)0)
    #define OGRE_ASSERT_MSG(condition, msg, ...) ((void)0)
    #define OGRE_ASSERT_FAIL(msg, ...) ((void)0)
    #define OGRE_VERIFY(cond) ((void)0)
    #define OGRE_VERIFY_MSG(cond, msg, ...) ((void)0)
#endif

#if __cplusplus >= 201103L
    #define OGRE_STATIC_ASSERT(x) static_assert( x, #x )
#else
    #define OGRE_STATIC_ASSERT(x) \
        typedef char OgreStaticAssert[(x) ? 1 : -1];OGRE_UNUSED(OgreStaticAssert);
#endif

#endif
