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
#ifndef __RotationalSpline_H__
#define __RotationalSpline_H__

#include "OgrePrerequisites.h"
#include "OgreQuaternion.h"

namespace Ogre {

    /** This class interpolates orientations (rotations) along a spline using 
        derivatives of quaternions.
    @remarks
        Like the SimpleSpline class, this class is about interpolating values 
        smoothly over a spline. Whilst SimpleSpline deals with positions (the normal
        sense we think about splines), this class interpolates orientations. The
        theory is identical, except we're now in 4-dimensional space instead of 3.
    @par
        In positional splines, we use the points and tangents on those points to generate
        control points for the spline. In this case, we use quaternions and derivatives
        of the quaternions (i.e. the rate and direction of change at each point). This is the
        same as SimpleSpline since a tangent is a derivative of a position. We effectively 
        generate an extra quaternion in between each actual quaternion which when take with 
        the original quaternion forms the 'tangent' of that quaternion.
    */
	class _OgreExport RotationalSpline : public SplineAlloc
    {
    public:
        RotationalSpline();
        ~RotationalSpline();

        /** Adds a control point to the end of the spline. */
        void addPoint(const Quaternion& p);

        /** Gets the detail of one of the control points of the spline. */
        const Quaternion& getPoint(unsigned short index) const;

        /** Gets the number of control points in the spline. */
        unsigned short getNumPoints(void) const;

        /** Clears all the points in the spline. */
        void clear(void);

        /** Updates a single point in the spline. 
        @remarks
            This point must already exist in the spline.
        */
        void updatePoint(unsigned short index, const Quaternion& value);

        /** Returns an interpolated point based on a parametric value over the whole series.
        @remarks
            Given a t value between 0 and 1 representing the parametric distance along the
            whole length of the spline, this method returns an interpolated point.
        @param t Parametric value.
		@param useShortestPath Defines if rotation should take the shortest possible path
        */
        Quaternion interpolate(Real t, bool useShortestPath=true);

        /** Interpolates a single segment of the spline given a parametric value.
        @param fromIndex The point index to treat as t=0. fromIndex + 1 is deemed to be t=1
        @param t Parametric value
		@param useShortestPath Defines if rotation should take the shortest possible path
        */
        Quaternion interpolate(unsigned int fromIndex, Real t, bool useShortestPath=true);

        /** Tells the spline whether it should automatically calculate tangents on demand
            as points are added.
        @remarks
            The spline calculates tangents at each point automatically based on the input points.
            Normally it does this every time a point changes. However, if you have a lot of points
            to add in one go, you probably don't want to incur this overhead and would prefer to 
            defer the calculation until you are finished setting all the points. You can do this
            by calling this method with a parameter of 'false'. Just remember to manually call 
            the recalcTangents method when you are done.
        @param autoCalc If true, tangents are calculated for you whenever a point changes. If false, 
            you must call reclacTangents to recalculate them when it best suits.
        */
        void setAutoCalculate(bool autoCalc);

        /** Recalculates the tangents associated with this spline. 
        @remarks
            If you tell the spline not to update on demand by calling setAutoCalculate(false)
            then you must call this after completing your updates to the spline points.
        */
        void recalcTangents(void);

    protected:

        bool mAutoCalc;



        std::vector<Quaternion> mPoints;
        std::vector<Quaternion> mTangents;

    };


}


#endif

