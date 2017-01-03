/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreVolumeIsoSurface.h"

namespace Ogre {
namespace Volume {

    const Real IsoSurface::ISO_LEVEL = (Real)0.0;

    const size_t IsoSurface::MS_CORNERS_FRONT[4] = {7, 6, 2, 3};
    const size_t IsoSurface::MS_CORNERS_BACK[4] = {5, 4, 0, 1};
    const size_t IsoSurface::MS_CORNERS_LEFT[4] = {4, 7, 3, 0};
    const size_t IsoSurface::MS_CORNERS_RIGHT[4] = {6, 5, 1, 2};
    const size_t IsoSurface::MS_CORNERS_TOP[4] = {4, 5, 6, 7};
    const size_t IsoSurface::MS_CORNERS_BOTTOM[4] = {3, 2, 1, 0};
    
    //-----------------------------------------------------------------------

    IsoSurface::IsoSurface(const Source *src) : mSrc(src)
    {
    }
    
    //-----------------------------------------------------------------------

    IsoSurface::~IsoSurface(void)
    {
    }
}
}