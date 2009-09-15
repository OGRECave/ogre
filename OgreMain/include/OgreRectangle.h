/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _Rectangle_H__
#define _Rectangle_H__

#include "OgrePrerequisites.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

    struct _OgreExport Rectangle
    {
        Real left;
        Real top;
        Real right;
        Real bottom;

        inline bool inside(Real x, Real y) const { return x >= left && x <= right && y >= top && y <= bottom; }
    };

    /** Geometric intersection of two rectanglar regions.
     *
     * @remarks Calculates the geometric intersection of two rectangular
     * regions.  Rectangle coordinates must be ([0-N], [0-N]), such that
     * (0,0) is in the upper left hand corner.
     *
     * If the two input rectangles do not intersect, then the result will be
     * a degenerate rectangle, i.e. left >= right or top >= bottom, or both.
     */
    inline Rectangle intersect(const Rectangle& lhs, const Rectangle& rhs)
    {
        Rectangle r;

        r.left   = lhs.left   > rhs.left   ? lhs.left   : rhs.left;
        r.top    = lhs.top    > rhs.top    ? lhs.top    : rhs.top;
        r.right  = lhs.right  < rhs.right  ? lhs.right  : rhs.right;
        r.bottom = lhs.bottom < rhs.bottom ? lhs.bottom : rhs.bottom;

        return r;
    }
	/** @} */
	/** @} */

}

#endif // _Rectangle_H__
