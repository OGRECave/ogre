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
#include "StringTests.h"
#include "OgreStringConverter.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreMatrix4.h"
#include "OgreColourValue.h"

#include "UnitTestSuite.h"

using namespace Ogre;

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(StringTests);

//--------------------------------------------------------------------------
void StringTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
    
    testFileNoPath = "testfile.txt";
    testFileRelativePathUnix = "this/is/relative/testfile.txt";
    testFileRelativePathWindows = "this\\is\\relative\\testfile.txt";
    testFileAbsolutePathUnix = "/this/is/absolute/testfile.txt";
    testFileAbsolutePathWindows = "c:\\this\\is\\absolute\\testfile.txt";
}
//--------------------------------------------------------------------------
void StringTests::tearDown()
{
}
//--------------------------------------------------------------------------
void StringTests::testSplitFileNameNoPath()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String basename, path;
    StringUtil::splitFilename(testFileNoPath, basename, path);

    CPPUNIT_ASSERT_EQUAL(testFileNoPath, basename);
    CPPUNIT_ASSERT(path.empty());
}
//--------------------------------------------------------------------------
void StringTests::testSplitFileNameRelativePath()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String basename, path;

    // Unix
    StringUtil::splitFilename(testFileRelativePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("this/is/relative/"), path);

    // Windows
    StringUtil::splitFilename(testFileRelativePathWindows, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("this/is/relative/"), path);
}
//--------------------------------------------------------------------------
void StringTests::testSplitFileNameAbsolutePath()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String basename, path;

    // Unix
    StringUtil::splitFilename(testFileAbsolutePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("/this/is/absolute/"), path);

    // Windows
    StringUtil::splitFilename(testFileAbsolutePathWindows, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("c:/this/is/absolute/"), path);
}
//--------------------------------------------------------------------------
void StringTests::testMatchCaseSensitive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    // Test positive
    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, true));

    // Test negative
    String upperCase = testFileNoPath;
    StringUtil::toUpperCase(upperCase);
    CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, upperCase, true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchCaseInSensitive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    // Test positive
    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, false));

    // Test positive
    String upperCase = testFileNoPath;
    StringUtil::toUpperCase(upperCase);
    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, upperCase, false));
}
//--------------------------------------------------------------------------
void StringTests::testMatchGlobAll()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*", true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchGlobStart()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.txt", true));
    CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "*astfile.txt", true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchGlobEnd()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "testfile.*", true));
    CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "testfile.d*", true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchGlobStartAndEnd()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.*", true));
    CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "*astfile.d*", true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchGlobMiddle()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "test*.txt", true));
    CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "last*.txt*", true));
}
//--------------------------------------------------------------------------
void StringTests::testMatchSuperGlobtastic()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*e*tf*e.t*t", true));
}
//--------------------------------------------------------------------------
void StringTests::testParseReal()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    Real r = 23.454;

    String s = StringConverter::toString(r);
    Real t = StringConverter::parseReal(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseInt()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    int r = 223546;

    String s = StringConverter::toString(r);
    int t = StringConverter::parseInt(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseLong()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    long r = -2147483647;

    String s = StringConverter::toString(r);
    long t = StringConverter::parseLong(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseUnsignedLong()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    unsigned long r = 4294967295UL;

    String s = StringConverter::toString(r);
    unsigned long t = StringConverter::parseUnsignedLong(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseVector3()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    Vector3 r(0.12, 3.22, -4.04);

    String s = StringConverter::toString(r);
    Vector3 t = StringConverter::parseVector3(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseMatrix4()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    Matrix4 r(1.12, 0, 0, 34, 0, 0.87, 0, 20, 0, 0, 0.56, 10, 0, 0, 0, 1);

    String s = StringConverter::toString(r);
    Matrix4 t = StringConverter::parseMatrix4(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseQuaternion()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    Quaternion r(1.12, 0.87, 0.67, 1);

    String s = StringConverter::toString(r);
    Quaternion t = StringConverter::parseQuaternion(s);

    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseBool()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    bool r = true;
    String s = StringConverter::toString(r);
    bool t = StringConverter::parseBool(s);
    CPPUNIT_ASSERT_EQUAL(r, t);

    r = false;
    s = StringConverter::toString(r);
    t = StringConverter::parseBool(s);
    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
void StringTests::testParseColourValue()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    ColourValue r(0.34, 0.44, 0.77, 1.0);

    String s = StringConverter::toString(r);
    ColourValue t = StringConverter::parseColourValue(s);
    CPPUNIT_ASSERT_EQUAL(r, t);
}
//--------------------------------------------------------------------------
