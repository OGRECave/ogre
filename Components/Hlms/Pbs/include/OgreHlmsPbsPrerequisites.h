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
#ifndef _OgreHlmsPbsPrerequisites_H_
#define _OgreHlmsPbsPrerequisites_H_

#include "OgrePrerequisites.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   if defined( OGRE_STATIC_LIB ) || defined( OGRE_PBS__STATIC_LIB )
#       define _OgreHlmsPbsExport
#   else
#       if defined( OgreHlmsPbs_EXPORTS )
#           define _OgreHlmsPbsExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
#               define _OgreHlmsPbsExport
#           else
#               define _OgreHlmsPbsExport __declspec( dllimport )
#           endif
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreHlmsPbsExport __attribute__ ((visibility("default")))
#else
#   define _OgreHlmsPbsExport
#endif 

namespace Ogre
{
    enum PbsTextureTypes
    {
        PBSM_DIFFUSE,
        PBSM_NORMAL,
        PBSM_SPECULAR,
        PBSM_METALLIC = PBSM_SPECULAR,
        PBSM_ROUGHNESS,
        PBSM_DETAIL_WEIGHT,
        PBSM_DETAIL0,
        PBSM_DETAIL1,
        PBSM_DETAIL2,
        PBSM_DETAIL3,
        PBSM_DETAIL0_NM,
        PBSM_DETAIL1_NM,
        PBSM_DETAIL2_NM,
        PBSM_DETAIL3_NM,
        PBSM_EMISSIVE,
        PBSM_REFLECTION,
        NUM_PBSM_SOURCES = PBSM_REFLECTION,
        NUM_PBSM_TEXTURE_TYPES
    };

    enum PbsBlendModes
    {
        /// Regular alpha blending
        PBSM_BLEND_NORMAL_NON_PREMUL,
        /// Premultiplied alpha blending
        PBSM_BLEND_NORMAL_PREMUL,
        PBSM_BLEND_ADD,
        PBSM_BLEND_SUBTRACT,
        PBSM_BLEND_MULTIPLY,
        PBSM_BLEND_MULTIPLY2X,
        PBSM_BLEND_SCREEN,
        PBSM_BLEND_OVERLAY,
        PBSM_BLEND_LIGHTEN,
        PBSM_BLEND_DARKEN,
        PBSM_BLEND_GRAIN_EXTRACT,
        PBSM_BLEND_GRAIN_MERGE,
        PBSM_BLEND_DIFFERENCE,
        NUM_PBSM_BLEND_MODES
    };

    class CubemapProbe;
    class HlmsPbs;
    class IrradianceVolume;
    class ParallaxCorrectedCubemap;
}

#endif
