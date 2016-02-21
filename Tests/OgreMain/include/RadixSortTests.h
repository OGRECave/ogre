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

#ifndef __RadixSortTests_H__
#define __RadixSortTests_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class RadixSortTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE(RadixSortTests);
    CPPUNIT_TEST(testFloatVector);
    CPPUNIT_TEST(testFloatList);
    CPPUNIT_TEST(testUnsignedIntList);
    CPPUNIT_TEST(testIntList);
    CPPUNIT_TEST(testUnsignedIntVector);
    CPPUNIT_TEST(testIntVector);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp();
    void tearDown();

    void testFloatVector();
    void testFloatList();
    void testUnsignedIntList();
    void testIntList();
    void testUnsignedIntVector();
    void testIntVector();
};

#endif