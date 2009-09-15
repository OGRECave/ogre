/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreRotationalSpline.h"



namespace Ogre {

    //---------------------------------------------------------------------
    RotationalSpline::RotationalSpline()
        : mAutoCalc(true)
    {
    }
    //---------------------------------------------------------------------
    RotationalSpline::~RotationalSpline()
    {
    }
    //---------------------------------------------------------------------
    void RotationalSpline::addPoint(const Quaternion& p)
    {
        mPoints.push_back(p);
        if (mAutoCalc)
        {
            recalcTangents();
        }
    }
    //---------------------------------------------------------------------
    Quaternion RotationalSpline::interpolate(Real t, bool useShortestPath)
    {
        // Work out which segment this is in
        Real fSeg = t * (mPoints.size() - 1);
        unsigned int segIdx = (unsigned int)fSeg;
        // Apportion t 
        t = fSeg - segIdx;

        return interpolate(segIdx, t, useShortestPath);

    }
    //---------------------------------------------------------------------
    Quaternion RotationalSpline::interpolate(unsigned int fromIndex, Real t,
		bool useShortestPath)
    {
        // Bounds check
        assert (fromIndex >= 0 && fromIndex < mPoints.size() &&
            "fromIndex out of bounds");

        if ((fromIndex + 1) == mPoints.size())
        {
            // Duff request, cannot blend to nothing
            // Just return source
            return mPoints[fromIndex];

        }
        // Fast special cases
        if (t == 0.0f)
        {
            return mPoints[fromIndex];
        }
        else if(t == 1.0f)
        {
            return mPoints[fromIndex + 1];
        }

        // Real interpolation
        // Use squad using tangents we've already set up
        Quaternion &p = mPoints[fromIndex];
        Quaternion &q = mPoints[fromIndex+1];
        Quaternion &a = mTangents[fromIndex];
        Quaternion &b = mTangents[fromIndex+1];

        // NB interpolate to nearest rotation
        return Quaternion::Squad(t, p, a, b, q, useShortestPath);

    }
    //---------------------------------------------------------------------
    void RotationalSpline::recalcTangents(void)
    {
        // ShoeMake (1987) approach
        // Just like Catmull-Rom really, just more gnarly
        // And no, I don't understand how to derive this!
        //
        // let p = point[i], pInv = p.Inverse
        // tangent[i] = p * exp( -0.25 * ( log(pInv * point[i+1]) + log(pInv * point[i-1]) ) )
        //
        // Assume endpoint tangents are parallel with line with neighbour

        unsigned int i, numPoints;
        bool isClosed;

        numPoints = (unsigned int)mPoints.size();

        if (numPoints < 2)
        {
            // Can't do anything yet
            return;
        }

        mTangents.resize(numPoints);

        if (mPoints[0] == mPoints[numPoints-1])
        {
            isClosed = true;
        }
        else
        {
            isClosed = false;
        }

        Quaternion invp, part1, part2, preExp;
        for(i = 0; i < numPoints; ++i)
        {
            Quaternion &p = mPoints[i];
            invp = p.Inverse();

            if (i ==0)
            {
                // special case start
                part1 = (invp * mPoints[i+1]).Log();
                if (isClosed)
                {
                    // Use numPoints-2 since numPoints-1 == end == start == this one
                    part2 = (invp * mPoints[numPoints-2]).Log();
                }
                else
                {
                    part2 = (invp * p).Log();
                }
            }
            else if (i == numPoints-1)
            {
                // special case end
                if (isClosed)
                {
                    // Wrap to [1] (not [0], this is the same as end == this one)
                    part1 = (invp * mPoints[1]).Log();
                }
                else
                {
                    part1 = (invp * p).Log();
                }
                part2 = (invp * mPoints[i-1]).Log();
            }
            else
            {
                part1 = (invp * mPoints[i+1]).Log();
                part2 = (invp * mPoints[i-1]).Log();
            }

            preExp = -0.25 * (part1 + part2);
            mTangents[i] = p * preExp.Exp();
            
        }



    }
    //---------------------------------------------------------------------
    const Quaternion& RotationalSpline::getPoint(unsigned short index) const
    {
        assert (index < mPoints.size() && "Point index is out of bounds!!");

        return mPoints[index];
    }
    //---------------------------------------------------------------------
    unsigned short RotationalSpline::getNumPoints(void) const
    {
        return (unsigned short)mPoints.size();
    }
    //---------------------------------------------------------------------
    void RotationalSpline::clear(void)
    {
        mPoints.clear();
        mTangents.clear();
    }
    //---------------------------------------------------------------------
    void RotationalSpline::updatePoint(unsigned short index, const Quaternion& value)
    {
        assert (index < mPoints.size() && "Point index is out of bounds!!");

        mPoints[index] = value;
        if (mAutoCalc)
        {
            recalcTangents();
        }
    }
    //---------------------------------------------------------------------
    void RotationalSpline::setAutoCalculate(bool autoCalc)
    {
        mAutoCalc = autoCalc;
    }
    //---------------------------------------------------------------------



}




