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

#ifndef __StringTests_H__
#define __StringTests_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgreString.h"

class StringTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE(StringTests);
    CPPUNIT_TEST(testSplitFileNameNoPath);
    CPPUNIT_TEST(testSplitFileNameRelativePath);
    CPPUNIT_TEST(testSplitFileNameAbsolutePath);
    CPPUNIT_TEST(testMatchCaseSensitive);
    CPPUNIT_TEST(testMatchCaseInSensitive);
    CPPUNIT_TEST(testMatchGlobAll);
    CPPUNIT_TEST(testMatchGlobStart);
    CPPUNIT_TEST(testMatchGlobEnd);
    CPPUNIT_TEST(testMatchGlobStartAndEnd);
    CPPUNIT_TEST(testMatchGlobMiddle);
    CPPUNIT_TEST(testMatchSuperGlobtastic);
    CPPUNIT_TEST(testParseReal);
    CPPUNIT_TEST(testParseInt);
    CPPUNIT_TEST(testParseLong);
    CPPUNIT_TEST(testParseUnsignedLong);
    CPPUNIT_TEST(testParseVector3);
    CPPUNIT_TEST(testParseMatrix4);
    CPPUNIT_TEST(testParseQuaternion);
    CPPUNIT_TEST(testParseBool);
    CPPUNIT_TEST(testParseColourValue);
    CPPUNIT_TEST_SUITE_END();

protected:
    Ogre::String testFileNoPath;
    Ogre::String testFileRelativePathWindows;
    Ogre::String testFileRelativePathUnix;
    Ogre::String testFileAbsolutePathWindows;
    Ogre::String testFileAbsolutePathUnix;

public:
    void setUp();
    void tearDown();

    // StringUtil::splitFileName tests
    void testSplitFileNameNoPath();
    void testSplitFileNameRelativePath();
    void testSplitFileNameAbsolutePath();

    // StringUtil::match tests
    void testMatchCaseSensitive();
    void testMatchCaseInSensitive();
    void testMatchGlobAll();
    void testMatchGlobStart();
    void testMatchGlobEnd();
    void testMatchGlobStartAndEnd();
    void testMatchGlobMiddle();
    void testMatchSuperGlobtastic();

    // StringConverter tests
    void testParseReal();
    void testParseInt();
    void testParseLong();
    void testParseUnsignedLong();
    void testParseVector3();
    void testParseMatrix4();
    void testParseQuaternion();
    void testParseBool();
    void testParseColourValue();
};

#endif