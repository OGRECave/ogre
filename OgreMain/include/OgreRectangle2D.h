/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
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
#ifndef _Rectangle2D_H__
#define _Rectangle2D_H__

#include "OgrePrerequisites.h"

#include "OgreSimpleRenderable.h"

namespace Ogre {

    /** Allows the rendering of a simple 2D rectangle
    This class renders a simple 2D rectangle; this rectangle has no depth and
    therefore is best used with specific render queue and depth settings,
    like RENDER_QUEUE_BACKGROUND and 'depth_write off' for backdrops, and 
    RENDER_QUEUE_OVERLAY and 'depth_check off' for fullscreen quads.
    */
    class _OgreExport Rectangle2D : public SimpleRenderable
    {
    protected:
        /** Override this method to prevent parent transforms (rotation,translation,scale)
        */
        void getWorldTransforms( Matrix4* xform ) const;

    public:

        Rectangle2D(bool includeTextureCoordinates = false);
        ~Rectangle2D();

        /** Sets the corners of the rectangle, in relative coordinates.
        @param
        left Left position in screen relative coordinates, -1 = left edge, 1.0 = right edge
        top Top position in screen relative coordinates, 1 = top edge, -1 = bottom edge
        right Right position in screen relative coordinates
        bottom Bottom position in screen relative coordinates
        */
        void setCorners(Real left, Real top, Real right, Real bottom);

        Real getSquaredViewDepth(const Camera* cam) const { return 0; }

        Real getBoundingRadius(void) const { return 0; }

    };

}// namespace

#endif


