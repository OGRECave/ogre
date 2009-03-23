/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
