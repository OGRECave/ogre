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
#include <gtest/gtest.h>
#include "OgreQuaternion.h"
#include "OgreVector.h"

using namespace Ogre;

TEST(QuaternionTests,Norm)
{
    EXPECT_EQ(Quaternion(0, 2, 2, 2).Norm(), Vector3(2, 2, 2).length());
}

TEST(QuaternionTests,FromVectors)
{
    // VectorBase<3, Real>::getRotationTo tests
    Vector3 list[][2] =
    {
        // Generate identity quaternions
        { Vector3(1, 0, 0), Vector3(1, 0, 0) },
        { Vector3(2, 0, 0), Vector3(3, 0, 0) },
        // Generate 90-degree rotations
        { Vector3(1, 0, 0), Vector3(0, 1, 0) },
        { Vector3(2, 0, 0), Vector3(0, 3, 0) },
        // Generate 180-degree rotations
        { Vector3(1, 0, 0), Vector3(0, -1, 0) },
        { Vector3(2, 0, 0), Vector3(0, -3, 0) },
        // Some random test values
        { Vector3(1, 2, 3), Vector3(-2, 1, 2) },
        { Vector3(2, -1, -3), Vector3(0, 1, -1) },
        { Vector3(100, 100, 0), Vector3(100, 100, 1) },
        { Vector3(10000, 10000, 0), Vector3(10000, 10000, 1) },
    };

    for (size_t index = 0; index < sizeof(list) / sizeof(Vector3[2]); ++index)
    {
        Vector3 from = list[index][0];
        Vector3 to = list[index][1];
        Vector3 result = from.getRotationTo(to) * from;
        EXPECT_TRUE(to.normalisedCopy().positionEquals(result.normalisedCopy())) << " index is " << index << std::endl;
        EXPECT_NEAR(from.length(), result.length(), 1e-6) << " index is " << index << std::endl;
    }
    Vector3 list2[][2] =
    {
        // Edge cases, return IDENTITY, because there is no correct rotation
        { Vector3(0, 0, 0), Vector3(1, 0, 0) },
        { Vector3(1, 0, 0), Vector3(0, 0, 0) },
    };
    for (size_t index = 0; index < sizeof(list2) / sizeof(Vector3[2]); ++index)
    {
        Vector3 from = list2[index][0];
        Vector3 to = list2[index][1];
        Quaternion quat = from.getRotationTo(to);
        EXPECT_EQ(quat, Quaternion::IDENTITY) << " index is " << index << std::endl;
    }
}

TEST(QuaternionTests,Exp)
{
    /** Comparison values got from the Octave quaternion package. */

    // Case a quaternion for which angle is 0 degrees.
    Quaternion quatA(1., 0., 0., 0.);
    Quaternion expQuatA = quatA.Exp();
    EXPECT_NEAR(expQuatA.w, 2.71828182845905, 1e-6);
    EXPECT_NEAR(expQuatA.x, 0., 1e-6);
    EXPECT_NEAR(expQuatA.y, 0., 1e-6);
    EXPECT_NEAR(expQuatA.z, 0., 1e-6);

    // Case of a common quaternion (no specific rotation).
    Quaternion quatB(0.2, 0.7, Ogre::Math::PI, 0.9);
    Quaternion expQuatB = quatB.Exp();
    EXPECT_NEAR(expQuatB.w, -1.19693377635754, 1e-6);
    EXPECT_NEAR(expQuatB.x, -0.05095014937169, 1e-6);
    EXPECT_NEAR(expQuatB.y, -0.22866373566485, 1e-6);
    EXPECT_NEAR(expQuatB.z, -0.06550733490645, 1e-6);

    // Normalised quaternion B.
    quatB.normalise() ;
    Quaternion expUnitQuatB = quatB.Exp();
    EXPECT_NEAR(expUnitQuatB.w, 0.575155457731263, 1e-6);
    EXPECT_NEAR(expUnitQuatB.x, 0.186879761760123, 1e-6);
    EXPECT_NEAR(expUnitQuatB.y, 0.838714409500305, 1e-6);
    EXPECT_NEAR(expUnitQuatB.z, 0.240273979405873, 1e-6);
}

TEST(QuaternionTests,Log)
{
    /** Comparison values got from the Octave quaternion package. */

    // Case of a common quaternion (no specific rotation).
    Quaternion quat(0.85, Ogre::Math::PI, 0.6, 0.2);
    quat.normalise() ;
    Quaternion logUnitQuat = quat.Log();
    EXPECT_NEAR(logUnitQuat.w, 0., 1e-6);
    EXPECT_NEAR(logUnitQuat.x, 1.28572906735070, 1e-6);
    EXPECT_NEAR(logUnitQuat.y, 0.24555616385496, 1e-6);
    EXPECT_NEAR(logUnitQuat.z, 0.08185205461832, 1e-6);
}
