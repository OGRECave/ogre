/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "OgreVolumeDualCell.h"

#include "OgreVolumeMeshBuilder.h"

namespace Ogre {
namespace Volume {

    DualCell::DualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7)
    {
        mCorners[0] = c0;
        mCorners[1] = c1;
        mCorners[2] = c2;
        mCorners[3] = c3;
        mCorners[4] = c4;
        mCorners[5] = c5;
        mCorners[6] = c6;
        mCorners[7] = c7;
        mValues[0].x = (Real)0.0;
        mValues[0].y = (Real)0.0;
        mValues[0].z = (Real)0.0;
    }
    
    //-----------------------------------------------------------------------

    DualCell::DualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7,
                const Vector4 &v0, const Vector4 &v1, const Vector4 &v2, const Vector4 &v3, const Vector4 &v4, const Vector4 &v5, const Vector4 &v6, const Vector4 &v7)
    {
        mCorners[0] = c0;
        mCorners[1] = c1;
        mCorners[2] = c2;
        mCorners[3] = c3;
        mCorners[4] = c4;
        mCorners[5] = c5;
        mCorners[6] = c6;
        mCorners[7] = c7;
        mValues[0] = v0;
        mValues[1] = v1;
        mValues[2] = v2;
        mValues[3] = v3;
        mValues[4] = v4;
        mValues[5] = v5;
        mValues[6] = v6;
        mValues[7] = v7;
    }
    
    //-----------------------------------------------------------------------

    void DualCell::addToManualObject(ManualObject *manual, size_t &baseIndex) const
    {
        MeshBuilder::addCubeToManualObject(
            manual,
            mCorners[0],
            mCorners[1],
            mCorners[2],
            mCorners[3],
            mCorners[4],
            mCorners[5],
            mCorners[6],
            mCorners[7],
            baseIndex);
    }

}
}