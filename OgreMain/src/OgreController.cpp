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

#include "OgreStableHeaders.h"

#include "OgreController.h"
#include "OgreVector3.h"

namespace Ogre
{
    template <>
    float ControllerFunction<float>::getAdjustedInput(float input)
    {
        if (mDeltaInput)
        {
            mDeltaCount += input;
            // Wrap to range [0; 1)
            //  i.e.
            //      0       => 0
            //      0.99    => 0.99
            //      1       => 0
            //      1.1     => 0.1
            //     -0.1     => 0.9
            mDeltaCount = mDeltaCount - floorf( mDeltaCount );

            return mDeltaCount;
        }
        else
        {
            return input;
        }
    }

    template <>
    double ControllerFunction<double>::getAdjustedInput(double input)
    {
        if (mDeltaInput)
        {
            mDeltaCount += input;
            mDeltaCount = mDeltaCount - floor( mDeltaCount );

            return mDeltaCount;
        }
        else
        {
            return input;
        }
    }

    template <>
    Vector3 ControllerFunction<Vector3>::getAdjustedInput(Vector3 input)
    {
        if (mDeltaInput)
        {
            mDeltaCount += input;
            mDeltaCount.x = mDeltaCount.x - Math::Floor( mDeltaCount.x );
            mDeltaCount.y = mDeltaCount.y - Math::Floor( mDeltaCount.y );
            mDeltaCount.z = mDeltaCount.z - Math::Floor( mDeltaCount.z );

            return mDeltaCount;
        }
        else
        {
            return input;
        }
    }
}
