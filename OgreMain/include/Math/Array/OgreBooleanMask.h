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
#ifndef __BooleanMask_H__
#define __BooleanMask_H__

//This file is a proxy, it redirects to the proper file depending on platform
#include "OgreArrayConfig.h"

#if OGRE_CPU == OGRE_CPU_X86 && OGRE_USE_SIMD == 1
    #if OGRE_DOUBLE_PRECISION == 1
        #include "SSE2/Double/OgreBooleanMask.h"
    #else
        #include "SSE2/Single/OgreBooleanMask.h"
    #endif
#elif OGRE_CPU == OGRE_CPU_ARM && OGRE_USE_SIMD == 1
    #if OGRE_DOUBLE_PRECISION == 1
        #error Double precision with SIMD on ARM is not supported
    #else
        #include "NEON/Single/OgreBooleanMask.h"
    #endif
#else
    #include "C/OgreBooleanMask.h"
#endif

#endif
