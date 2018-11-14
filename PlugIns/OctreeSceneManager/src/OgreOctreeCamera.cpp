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
/***************************************************************************
octreecamera.cpp  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

***************************************************************************/
#include "OgreAxisAlignedBox.h"
#include "OgreOctreeCamera.h"

namespace Ogre
{
OctreeCamera::OctreeCamera( const String& name, SceneManager* sm ) : Camera( name, sm )
{
                                                                      
}

OctreeCamera::~OctreeCamera()
{
}

OctreeCamera::Visibility OctreeCamera::getVisibility( const AxisAlignedBox &bound )
{

    // Null boxes always invisible
    if ( bound.isNull() )
        return NONE;

    // Get centre of the box
    Vector3 centre = bound.getCenter();
    // Get the half-size of the box
    Vector3 halfSize = bound.getHalfSize();

    bool all_inside = true;

    for ( uchar plane = 0; plane < 6; ++plane )
    {

        // Skip far plane if infinite view frustum
        if (plane == FRUSTUM_PLANE_FAR && mFarDist == 0)
            continue;

        // This updates frustum planes and deals with cull frustum
        Plane::Side side = getFrustumPlane(plane).getSide(centre, halfSize);
        if(side == Plane::NEGATIVE_SIDE) return NONE;
        // We can't return now as the box could be later on the negative side of a plane.
        if(side == Plane::BOTH_SIDE) 
                all_inside = false;
    }

    if ( all_inside )
        return FULL;
    else
        return PARTIAL;

}

}




