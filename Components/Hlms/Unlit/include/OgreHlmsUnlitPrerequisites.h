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
#ifndef _OgreHlmsUnlitPrerequisites_H_
#define _OgreHlmsUnlitPrerequisites_H_

#include "OgrePrerequisites.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   if defined( OGRE_STATIC_LIB ) || defined( OGRE_UNLIT_STATIC_LIB )
#       define _OgreHlmsUnlitExport
#   else
#       if defined( OgreHlmsUnlit_EXPORTS )
#           define _OgreHlmsUnlitExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
#               define _OgreHlmsUnlitExport
#           else
#               define _OgreHlmsUnlitExport __declspec( dllimport )
#           endif
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreHlmsUnlitExport __attribute__ ((visibility("default")))
#else
#   define _OgreHlmsUnlitExport
#endif 

namespace Ogre
{
    enum UnlitTextureTypes
    {
        NUM_UNLIT_TEXTURE_TYPES = 16
    };

    enum UnlitBlendModes
    {
        /// Regular alpha blending
        UNLIT_BLEND_NORMAL_NON_PREMUL,
        /// Premultiplied alpha blending
        UNLIT_BLEND_NORMAL_PREMUL,
        UNLIT_BLEND_ADD,
        UNLIT_BLEND_SUBTRACT,
        UNLIT_BLEND_MULTIPLY,
        UNLIT_BLEND_MULTIPLY2X,
        UNLIT_BLEND_SCREEN,
        UNLIT_BLEND_OVERLAY,
        UNLIT_BLEND_LIGHTEN,
        UNLIT_BLEND_DARKEN,
        UNLIT_BLEND_GRAIN_EXTRACT,
        UNLIT_BLEND_GRAIN_MERGE,
        UNLIT_BLEND_DIFFERENCE,
        NUM_UNLIT_BLEND_MODES
    };

    class HlmsUnlit;
}

#endif
