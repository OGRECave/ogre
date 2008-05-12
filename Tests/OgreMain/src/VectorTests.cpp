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
#include "VectorTests.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"

// Register the suite
CPPUNIT_TEST_SUITE_REGISTRATION( VectorTests );

using namespace Ogre;

void VectorTests::setUp()
{
}

void VectorTests::tearDown()
{
}


void VectorTests::testVector2Scaler()
{
    CPPUNIT_ASSERT_EQUAL(Vector2(1, 1) + Vector2(2, 2), Vector2(3, 3));
    CPPUNIT_ASSERT_EQUAL(1 + Vector2(2), Vector2(3, 3));
    Vector2 v1;
    v1 = 1;
    CPPUNIT_ASSERT_EQUAL(v1, Vector2(1, 1));
    v1 = 0.0;
    CPPUNIT_ASSERT_EQUAL(v1, Vector2::ZERO);
    v1 += 3;
    CPPUNIT_ASSERT_EQUAL(v1, Vector2(3, 3));

    v1 = 3 - Vector2(2);
    CPPUNIT_ASSERT_EQUAL(v1, Vector2(1));
    v1 = Vector2(5) - 7;
    CPPUNIT_ASSERT_EQUAL(v1, Vector2(-2));
    v1 -= 4;
    CPPUNIT_ASSERT_EQUAL(v1, Vector2(-6));
}

void VectorTests::testVector3Scaler()
{
    CPPUNIT_ASSERT_EQUAL(Vector3(1, 1, 1) + Vector3(2, 2, 2), Vector3(3, 3, 3));
    CPPUNIT_ASSERT_EQUAL(1 + Vector3(2), Vector3(3, 3, 3));
    Vector3 v1;
    v1 = 1;
    CPPUNIT_ASSERT_EQUAL(v1, Vector3(1));
    v1 = 0.0;
    CPPUNIT_ASSERT_EQUAL(v1, Vector3::ZERO);
    v1 += 3;
    CPPUNIT_ASSERT_EQUAL(v1, Vector3(3));

    v1 = 3 - Vector3(2);
    CPPUNIT_ASSERT_EQUAL(v1, Vector3(1));
    v1 = Vector3(5) - 7;
    CPPUNIT_ASSERT_EQUAL(v1, Vector3(-2));
    v1 -= 4;
    CPPUNIT_ASSERT_EQUAL(v1, Vector3(-6));
}

void VectorTests::testVector4Scaler()
{
    CPPUNIT_ASSERT_EQUAL(Vector4(1, 1, 1, 1) + Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
    CPPUNIT_ASSERT_EQUAL(1 + Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
    Vector4 v1;
    v1 = 1;
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(1, 1, 1, 1));
    v1 = 0.0;
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(0,0,0,0));
    v1 += 3;
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(3,3,3,3));

    v1 = 3 - Vector4(2,2,2,2);
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(1,1,1,1));
    v1 = Vector4(5,5,5,5) - 7;
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(-2,-2,-2,-2));
    v1 -= 4;
    CPPUNIT_ASSERT_EQUAL(v1, Vector4(-6,-6,-6,-6));
}
