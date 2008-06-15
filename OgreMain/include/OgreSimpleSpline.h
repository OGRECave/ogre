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

#ifndef __SimpleSpline_H__
#define __SimpleSpline_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre {


    /** A very simple spline class which implements the Catmull-Rom class of splines.
    @remarks
        Splines are bendy lines. You define a series of points, and the spline forms
        a smoother line between the points to eliminate the sharp angles.
    @par
        Catmull-Rom splines are a specialisation of the general Hermite spline. With
        a Hermite spline, you define the start and end point of the line, and 2 tangents,
        one at the start of the line and one at the end. The Catmull-Rom spline simplifies
        this by just asking you to define a series of points, and the tangents are 
        created for you. 
    */
	class _OgreExport SimpleSpline
    {
    public:
        SimpleSpline();
        ~SimpleSpline();

        /** Adds a control point to the end of the spline. */
        void addPoint(const Vector3& p);

        /** Gets the detail of one of the control points of the spline. */
        const Vector3& getPoint(unsigned short index) const;

        /** Gets the number of control points in the spline. */
        unsigned short getNumPoints(void) const;

        /** Clears all the points in the spline. */
        void clear(void);

        /** Updates a single point in the spline. 
        @remarks
            This point must already exist in the spline.
        */
        void updatePoint(unsigned short index, const Vector3& value);

        /** Returns an interpolated point based on a parametric value over the whole series.
        @remarks
            Given a t value between 0 and 1 representing the parametric distance along the
            whole length of the spline, this method returns an interpolated point.
        @param t Parametric value.
        */
        Vector3 interpolate(Real t);

        /** Interpolates a single segment of the spline given a parametric value.
        @param fromIndex The point index to treat as t=0. fromIndex + 1 is deemed to be t=1
        @param t Parametric value
        */
        Vector3 interpolate(unsigned int fromIndex, Real t);


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

        std::vector<Vector3> mPoints;
        std::vector<Vector3> mTangents;

        /// Matrix of coefficients 
        Matrix4 mCoeffs;



    };


}


#endif

