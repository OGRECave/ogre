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
OgrePCZPrerequisites.h  -  Portal Connected Zone Scene Manager prerequisites

-----------------------------------------------------------------------------
begin                : Mon Feb 19 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#ifndef PCZ_PREREQUISITES_H
#define PCZ_PREREQUISITES_H

#include "OgrePrerequisites.h"

//-----------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Windows Settings
//-----------------------------------------------------------------------

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_PCZPLUGIN_EXPORTS
#       define _OgrePCZPluginExport __declspec(dllexport)
#   else
#       define _OgrePCZPluginExport __declspec(dllimport)
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgrePCZPluginExport  __attribute__ ((visibility("default")))
#else
#   define _OgrePCZPluginExport
#endif

#endif

