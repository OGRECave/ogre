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
#include "DualQuaternionTests.h"
#include "OgreDualQuaternion.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

#include "UnitTestSuite.h"

using namespace Ogre;

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(DualQuaternionTests);

//--------------------------------------------------------------------------
void DualQuaternionTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
}
//--------------------------------------------------------------------------
void DualQuaternionTests::tearDown()
{
}
//--------------------------------------------------------------------------
void DualQuaternionTests::testConversion()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

	DualQuaternion dQuat;
	Quaternion quat(Radian(Degree(60)), Vector3::UNIT_Y);
	Vector3 translation(0, 0, 10);
	dQuat.fromRotationTranslation(quat, translation);
		
	Quaternion result;
	Vector3 resTrans;
	dQuat.toRotationTranslation(result, resTrans);

	CPPUNIT_ASSERT_EQUAL(result, quat);
	CPPUNIT_ASSERT(resTrans.positionEquals(translation));
}
//--------------------------------------------------------------------------
void DualQuaternionTests::testDefaultValue()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

	DualQuaternion dQuatDefault;
	
	Quaternion quatDefault;
	Vector3 transDefault;
	
	dQuatDefault.toRotationTranslation(quatDefault, transDefault);

	CPPUNIT_ASSERT_EQUAL(quatDefault, Quaternion::IDENTITY); 
	CPPUNIT_ASSERT(transDefault.positionEquals(Vector3::ZERO));
}
//--------------------------------------------------------------------------
void DualQuaternionTests::testMatrix()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

	Matrix4 transform;
	Vector3 translation(10, 4, 0);
	Vector3 scale = Vector3::UNIT_SCALE;
	Quaternion rotation;
	rotation.FromAngleAxis(Radian(Math::PI), Vector3::UNIT_Z);
	transform.makeTransform(translation, scale, rotation);

	DualQuaternion dQuat;
	dQuat.fromTransformationMatrix(transform);
	Matrix4 transformResult;
	dQuat.toTransformationMatrix(transformResult);

	Vector3 translationResult;
	Vector3 scaleResult;
	Quaternion rotationResult;
	transformResult.decomposition(translationResult, scaleResult, rotationResult);

	CPPUNIT_ASSERT(translationResult.positionEquals(translation));
	CPPUNIT_ASSERT(scaleResult.positionEquals(scale));
	CPPUNIT_ASSERT(rotationResult.equals(rotation, Radian(0.001)));
}
//--------------------------------------------------------------------------