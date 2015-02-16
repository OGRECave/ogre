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

#ifndef __PixelFormatTests_H__
#define __PixelFormatTests_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgrePixelFormat.h"
#include "OgrePixelBox.h"

using namespace Ogre;

class PixelFormatTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE(PixelFormatTests);
    CPPUNIT_TEST(testIntegerPackUnpack);
    CPPUNIT_TEST(testFloatPackUnpack);
    CPPUNIT_TEST(testBulkConversion);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testIntegerPackUnpack();
    void testFloatPackUnpack();
    void testBulkConversion();

    // Utils
    void setupBoxes(PixelFormat srcFormat, PixelFormat dstFormat);
    void testCase(PixelFormat srcFormat, PixelFormat dstFormat);

private:
    int mSize;
    uint8 *mRandomData;
    uint8 *mTemp, *mTemp2;
    PixelBox mSrc, mDst1, mDst2;
};

#endif