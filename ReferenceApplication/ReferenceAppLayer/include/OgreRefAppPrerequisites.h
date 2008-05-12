/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#ifndef __REFAPP_PREREQUISITES_H__
#define __REFAPP_PREREQUISITES_H__

// Include ODE standard C header
#include <ode/ode.h>
// Include ODE C++ headers
#include <ode/odecpp.h>
#include <ode/odecpp_collision.h>

// Include main application-facing Ogre header
#include <Ogre.h>


// To save us some typing
using namespace Ogre;

namespace OgreRefApp {

    #if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
    // Export control
    #   if defined( REFERENCEAPPLAYER_EXPORTS )
    #       define _OgreRefAppExport __declspec( dllexport )
    #   else
    #       if defined( __MINGW32__ )
    #           define _OgreRefAppExport
    #       else
    #     	    define _OgreRefAppExport __declspec( dllimport )
    #       endif
    #   endif
    #else // Linux / Mac OSX etc
    #   define _OgreRefAppExport
    #endif

    // Quick conversions
    inline void OgreToOde(const Matrix3& ogre, dMatrix3& ode)
    {
        ode[0] = ogre[0][0];
        ode[1] = ogre[0][1];
        ode[2] = ogre[0][2];
        ode[3] = ogre[0][3];
        ode[4] = ogre[1][0];
        ode[5] = ogre[1][1];
        ode[6] = ogre[1][2];
        ode[7] = ogre[1][3];
        ode[8] = ogre[2][0];
        ode[9] = ogre[2][1];
        ode[10] = ogre[2][2];
        ode[11] = ogre[2][3];
    }


    // Forward definitions of classes to reduce dependencies
    class ApplicationObject;
    class OgreHead;
    class FinitePlane;
    class Ball;
    class Box;
    class CollideCamera;

}



#endif

