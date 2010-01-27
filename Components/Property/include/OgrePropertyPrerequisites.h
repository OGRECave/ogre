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

#ifndef __Ogre_Property_Prereq_H__
#define __Ogre_Property_Prereq_H__

#include "OgrePrerequisites.h"

namespace Ogre
{

	typedef GeneralAllocatedObject PropertyAlloc;

}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#	if defined( OGRE_STATIC_LIB )
#   	define _OgrePropertyExport
#   else
#   	if defined( OGRE_PROPERTY_EXPORTS )
#       	define _OgrePropertyExport __declspec( dllexport )
#   	else
#           if defined( __MINGW32__ )
#               define _OgrePropertyExport
#           else
#       	    define _OgrePropertyExport __declspec( dllimport )
#           endif
#   	endif
#	endif
#else
#	define _OgrePropertyExport
#endif 


#endif 
