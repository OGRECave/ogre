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

#include "OgreDynLib.h"


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#  define WIN32_LEAN_AND_MEAN
#  if !defined(NOMINMAX) && defined(_MSC_VER)
#   define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#  include <windows.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
static std::wstring stringToWstring(const std::string& s)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), NULL, 0);
	std::wstring ws(L"", len);
	wchar_t* pWSBuf = const_cast<wchar_t*>(ws.c_str());
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, pWSBuf, len);
	return ws;
}
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   include "macUtils.h"
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   include <dlfcn.h>
#endif


namespace Ogre {

    //-----------------------------------------------------------------------
    DynLib::DynLib( const String& name )
    {
        mName = name;
        mInst = NULL;
    }

    //-----------------------------------------------------------------------
    DynLib::~DynLib()
    {
    }

    //-----------------------------------------------------------------------
    void DynLib::load()
    {
        String name = mName;
#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        if (name.find(".js") == String::npos)
            name += ".js";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        // dlopen() does not add .so to the filename, like windows does for .dll
        if (name.find(".so") == String::npos)
        {
            name += StringUtil::format(".so.%d.%d.%d", OGRE_VERSION_MAJOR, OGRE_VERSION_MINOR, OGRE_VERSION_PATCH);
        }
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // dlopen() does not add .dylib to the filename, like windows does for .dll
        if(name.substr(name.find_last_of(".") + 1) != "dylib")
            name += ".dylib";
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT

        // Although LoadLibraryEx will add .dll itself when you only specify the library name,
        // if you include a relative path then it does not. So, add it to be sure.
        if(name.substr(name.find_last_of(".") + 1) != "dll")
            name += OGRE_BUILD_SUFFIX ".dll";
#endif
        // Log library load
        LogManager::getSingleton().logMessage("Loading library " + name);

        mInst = (DYNLIB_HANDLE)DYNLIB_LOAD( name.c_str() );
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        if(!mInst)
        {
            name = mName;
            if(name.substr(name.find_last_of(".") + 1) != "framework")
                name += ".framework";
            // Try again as a framework
            mInst = (DYNLIB_HANDLE)FRAMEWORK_LOAD( name );
        }
#endif
        if( !mInst )
            OGRE_EXCEPT(
                Exception::ERR_INTERNAL_ERROR, 
                "Could not load dynamic library " + mName + 
                ".  System Error: " + dynlibError(),
                "DynLib::load" );
    }

    //-----------------------------------------------------------------------
    void DynLib::unload()
    {
        // Log library unload
        LogManager::getSingleton().logMessage("Unloading library " + mName);

        if( DYNLIB_UNLOAD( mInst ) )
        {
            OGRE_EXCEPT(
                Exception::ERR_INTERNAL_ERROR, 
                "Could not unload dynamic library " + mName +
                ".  System Error: " + dynlibError(),
                "DynLib::unload");
        }

    }

    //-----------------------------------------------------------------------
    void* DynLib::getSymbol( const String& strName ) const throw()
    {
        return (void*)DYNLIB_GETSYM( mInst, strName.c_str() );
    }
    //-----------------------------------------------------------------------
    String DynLib::dynlibError( void ) 
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        LPTSTR lpMsgBuf; 
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS, 
            NULL, 
            GetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            (LPTSTR)&lpMsgBuf, 
            0, 
            NULL 
            ); 
        String ret = lpMsgBuf;
        // Free the buffer.
        LocalFree( lpMsgBuf );
        return ret;
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        WCHAR wideMsgBuf[1024]; 
        if(0 == FormatMessageW( 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS, 
            NULL, 
            GetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            wideMsgBuf, 
            sizeof(wideMsgBuf) / sizeof(wideMsgBuf[0]), 
            NULL 
            ))
        {
            wideMsgBuf[0] = 0;
        }

        char narrowMsgBuf[2048] = "";
        if(0 == WideCharToMultiByte(
            CP_ACP, 0,
            wideMsgBuf, -1,
            narrowMsgBuf, sizeof(narrowMsgBuf) / sizeof(narrowMsgBuf[0]),
            NULL, NULL))
        {
            narrowMsgBuf[0] = 0;
        }
        String ret = narrowMsgBuf;

        return ret;
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        return String(dlerror());
#else
        return String("");
#endif
    }

}
