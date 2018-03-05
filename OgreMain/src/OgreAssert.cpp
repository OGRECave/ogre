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

#include "OgreStableHeaders.h"

#if _MSC_VER && !defined( _CONSOLE )
    #define _CRT_SECURE_NO_WARNINGS
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #include <Windows.h>
    #include <stdlib.h>
    #include <signal.h> //raise(SIGABRT)
#endif

#include "OgreAssert.h"

#include <stdio.h>
#include <stdarg.h>

namespace Ogre
{

namespace
{

Assert::FailBehavior DefaultHandler(const char* condition,
                                    const char* msg,
                                    const char* file,
                                    const int line)
{
#if _MSC_VER && _WIN32 && !defined( _CONSOLE )
    char fullmsg[1024];
    memset( fullmsg, 0, sizeof(fullmsg) );

    char path[MAX_PATH];
    GetModuleFileNameA( NULL, path, MAX_PATH );

    _snprintf( fullmsg, sizeof(fullmsg), "Assertion failed!\n\n"
               "Program: %s\n"
               "File: %s\n"
               "Line: %d\n", path, file, line );

    if( condition != NULL )
        _snprintf( fullmsg, sizeof(fullmsg), "%s\nExpresson: %s\n", fullmsg, condition );

    if( msg != NULL )
        _snprintf( fullmsg, sizeof(fullmsg), "%s\n%s\n", fullmsg, msg );

    strncat( fullmsg, "\n\n(Press Retry to debug application)", sizeof(fullmsg) );

    fullmsg[sizeof(fullmsg)-1u] = '\0';

    int msgboxId = MessageBoxA( NULL, fullmsg, "Assert Failure!",
                                MB_ICONERROR|MB_ABORTRETRYIGNORE );

    if( msgboxId == IDABORT )
    {
        raise(SIGABRT);
        return Assert::Halt;
    }
    else if( msgboxId == IDRETRY )
    {
        return Assert::Halt;
    }
    else //if( msgboxId == IDIGNORE )
    {
        return Assert::Continue;
    }
#else
    fprintf(stderr, "%s(%d): Assert Failure: ", file, line);

    if (condition != NULL)
        fprintf(stderr, "'%s' ", condition);

    if (msg != NULL)
        fprintf(stderr, "%s", msg);

    fprintf(stderr, "\n");

    fflush( stderr );

    return Assert::Halt;
#endif
}

Assert::Handler& GetAssertHandlerInstance()
{
    static Assert::Handler s_handler = &DefaultHandler;
    return s_handler;
}

}

Assert::Handler Assert::GetHandler()
{
    return GetAssertHandlerInstance();
}

void Assert::SetHandler(Assert::Handler newHandler)
{
    GetAssertHandlerInstance() = newHandler;
}

Assert::FailBehavior Assert::ReportFailure(const char* condition,
                                           const char* file,
                                           const int line,
                                           const char* msg, ...)
{
    const char* message = NULL;
    if (msg != NULL)
    {
        char messageBuffer[1024];
        {
            va_list args;
            va_start(args, msg);
            vsnprintf(messageBuffer, 1024, msg, args);
            va_end(args);
        }

        message = messageBuffer;
    }

    return GetAssertHandlerInstance()(condition, message, file, line);
}

}
