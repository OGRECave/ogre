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

#ifndef __DualQuaternionTests_H__
#define __DualQuaternionTests_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class DualQuaternionTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE(DualQuaternionTests);
	CPPUNIT_TEST(testConversion);
    CPPUNIT_TEST(testDefaultValue);
	CPPUNIT_TEST(testMatrix);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

	//Converts back and forth between a dual quaternion and a quaternion and vector
    void testConversion();
	//Tests if the default values of a dual quaternion are the correct identity values
    void testDefaultValue();
	//Converts back and forth between a dual quaternion and a transformation matrix
	void testMatrix();
};

#endif