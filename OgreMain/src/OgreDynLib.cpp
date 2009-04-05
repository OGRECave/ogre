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
#include "OgreStableHeaders.h"

#include "OgreDynLib.h"

#include "OgreException.h"
#include "OgreLogManager.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX // required to stop windows.h messing up std::min
#  include <windows.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include "macPlugins.h"
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
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // dlopen() does not add .dylib to the filename, like windows does for .dll
        if (name.substr(name.length() - 6, 6) != ".dylib")
			name += ".dylib";
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
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        return String(dlerror());
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        return String(mac_errorBundle());
#else
        return String("");
#endif
    }

}
