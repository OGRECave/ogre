/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreException.h"
#include "OgreLogManager.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX // required to stop windows.h messing up std::min
#  include <windows.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   include "macUtils.h"
#   include <dlfcn.h>
#endif


namespace Ogre {

    //-----------------------------------------------------------------------
    DynLib::DynLib( const String& name )
    {
        mName = name;
        m_hInst = NULL;
    }

    //-----------------------------------------------------------------------
    DynLib::~DynLib()
    {
    }

    //-----------------------------------------------------------------------
    void DynLib::load()
    {
        // Log library load
        LogManager::getSingleton().logMessage("Loading library " + mName);

		String name = mName;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        // dlopen() does not add .so to the filename, like windows does for .dll
        if (name.substr(name.length() - 3, 3) != ".so")
           name += ".so";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // dlopen() does not add .dylib to the filename, like windows does for .dll
        if (name.substr(name.length() - 6, 6) != ".dylib")
			name += ".dylib";
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		// Although LoadLibraryEx will add .dll itself when you only specify the library name,
		// if you include a relative path then it does not. So, add it to be sure.
		if (name.substr(name.length() - 4, 4) != ".dll")
			name += ".dll";
#endif
        m_hInst = (DYNLIB_HANDLE)DYNLIB_LOAD( name.c_str() );

        if( !m_hInst )
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

        if( DYNLIB_UNLOAD( m_hInst ) )
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
        return (void*)DYNLIB_GETSYM( m_hInst, strName.c_str() );
    }
    //-----------------------------------------------------------------------
    String DynLib::dynlibError( void ) 
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        LPVOID lpMsgBuf; 
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS, 
            NULL, 
            GetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            (LPTSTR) &lpMsgBuf, 
            0, 
            NULL 
            ); 
        String ret = (char*)lpMsgBuf;
        // Free the buffer.
        LocalFree( lpMsgBuf );
        return ret;
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        return String(dlerror());
#else
        return String("");
#endif
    }

}
