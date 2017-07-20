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

#ifndef __GLStateCacheManagerCommon_H__
#define __GLStateCacheManagerCommon_H__

#include "OgreGLSupportPrerequisites.h"
#include "OgreMemoryAllocatorConfig.h"

namespace Ogre
{
    typedef GeneralAllocatedObject StateCacheAlloc;

    /** An in memory cache of the OpenGL state.
     @remarks
     State changes can be particularly expensive time wise. This is because
     a change requires OpenGL to re-evaluate and update the state machine.
     Because of the general purpose nature of OGRE we often set the state for
     a specific texture, material, buffer, etc. But this may be the same as the
     current status of the state machine and is therefore redundant and causes
     unnecessary work to be performed by OpenGL.
     @par
     Instead we are caching the state so that we can check whether it actually
     does need to be updated. This leads to improved performance all around and 
     can be somewhat dramatic in some cases.
     */
    class _OgreGLExport GLStateCacheManagerCommon : public StateCacheAlloc
    {
    public:
        GLStateCacheManagerCommon(void) {}
        virtual ~GLStateCacheManagerCommon() {}
    };
}

#endif
