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
#include "QuaternionTests.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"

// Register the suite
CPPUNIT_TEST_SUITE_REGISTRATION( QuaternionTests );

using namespace Ogre;

void QuaternionTests::setUp()
{
}

void QuaternionTests::tearDown()
{
}

void QuaternionTests::testFromVectors()
{
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

    Quaternion quat;
    Vector3 from, to;

    from = list[0][0];
    to = list[0][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[1][0];
    to = list[1][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[2][0];
    to = list[2][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[3][0];
    to = list[3][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[4][0];
    to = list[4][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[5][0];
    to = list[5][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[6][0];
    to = list[6][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[7][0];
    to = list[7][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[8][0];
    to = list[8][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));

    from = list[9][0];
    to = list[9][1];
    CPPUNIT_ASSERT(to.normalisedCopy().positionEquals(from.getRotationTo(to) * from.normalisedCopy()));
}

