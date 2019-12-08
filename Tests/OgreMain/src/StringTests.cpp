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
#include "OgreVector.h"
#include "OgreQuaternion.h"
#include "OgreMatrix4.h"
#include "OgreColourValue.h"


using namespace Ogre;

// Register the test suite

//--------------------------------------------------------------------------
void StringTests::SetUp()
{    
    testFileNoPath = "testfile.txt";
    testFileRelativePathUnix = "this/is/relative/testfile.txt";
    testFileRelativePathWindows = "this\\is\\relative\\testfile.txt";
    testFileAbsolutePathUnix = "/this/is/absolute/testfile.txt";
    testFileAbsolutePathWindows = "c:\\this\\is\\absolute\\testfile.txt";
    setlocale(LC_NUMERIC, "");
}
//--------------------------------------------------------------------------
void StringTests::TearDown()
{
}
//--------------------------------------------------------------------------
TEST_F(StringTests,SplitBaseFilename)
{
    String base, ext;

    StringUtil::splitBaseFilename("basename.ext", base, ext);
    EXPECT_EQ("basename", base);
    EXPECT_EQ("ext", ext);

    StringUtil::splitBaseFilename("basename.", base, ext);
    EXPECT_EQ("basename", base);
    EXPECT_EQ("", ext);
}
TEST_F(StringTests,SplitFileNameNoPath)
{
    String basename, path;
    StringUtil::splitFilename(testFileNoPath, basename, path);

    EXPECT_EQ(testFileNoPath, basename);
    EXPECT_TRUE(path.empty());
}
//--------------------------------------------------------------------------
TEST_F(StringTests,SplitFileNameRelativePath)
{
    String basename, path;

    // Unix
    StringUtil::splitFilename(testFileRelativePathUnix, basename, path);
    EXPECT_EQ(String("testfile.txt"), basename);
    EXPECT_EQ(String("this/is/relative/"), path);

    // Windows
    StringUtil::splitFilename(testFileRelativePathWindows, basename, path);
    EXPECT_EQ(String("testfile.txt"), basename);
    EXPECT_EQ(String("this/is/relative/"), path);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,SplitFileNameAbsolutePath)
{
    String basename, path;

    // Unix
    StringUtil::splitFilename(testFileAbsolutePathUnix, basename, path);
    EXPECT_EQ(String("testfile.txt"), basename);
    EXPECT_EQ(String("/this/is/absolute/"), path);

    // Windows
    StringUtil::splitFilename(testFileAbsolutePathWindows, basename, path);
    EXPECT_EQ(String("testfile.txt"), basename);
    EXPECT_EQ(String("c:/this/is/absolute/"), path);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchCaseSensitive)
{
    // Test positive
    EXPECT_TRUE(StringUtil::match(testFileNoPath, testFileNoPath, true));

    // Test negative
    String upperCase = testFileNoPath;
    StringUtil::toUpperCase(upperCase);
    EXPECT_TRUE(!StringUtil::match(testFileNoPath, upperCase, true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchCaseInSensitive)
{
    // Test positive
    EXPECT_TRUE(StringUtil::match(testFileNoPath, testFileNoPath, false));

    // Test positive
    String upperCase = testFileNoPath;
    StringUtil::toUpperCase(upperCase);
    EXPECT_TRUE(StringUtil::match(testFileNoPath, upperCase, false));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchGlobAll)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "*", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchGlobStart)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "*stfile.txt", true));
    EXPECT_TRUE(!StringUtil::match(testFileNoPath, "*astfile.txt", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchGlobEnd)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "testfile.*", true));
    EXPECT_TRUE(!StringUtil::match(testFileNoPath, "testfile.d*", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchGlobStartAndEnd)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "*stfile.*", true));
    EXPECT_TRUE(!StringUtil::match(testFileNoPath, "*astfile.d*", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchGlobMiddle)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "test*.txt", true));
    EXPECT_TRUE(!StringUtil::match(testFileNoPath, "last*.txt*", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchSuperGlobtastic)
{
    EXPECT_TRUE(StringUtil::match(testFileNoPath, "*e*tf*e.t*t", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,MatchSuperGlobEnd)
{
    EXPECT_TRUE(StringUtil::match("normal", "*normal*", true));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseReal)
{
    Real r = 23.454;

    String s = StringConverter::toString(r);

    EXPECT_EQ(r, StringConverter::parseReal(s));
    EXPECT_EQ(r, StringConverter::parseReal("23.454"));
    EXPECT_NE(r, StringConverter::parseReal("23,454"));
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseInt32)
{
    int32 r = -223546;

    String s = StringConverter::toString(r);
    int32 t = StringConverter::parseInt(s);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseInt64)
{
    long r = -2147483647;

    String s = StringConverter::toString(r);
    int64 t;
    StringConverter::parse(s, t);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseUInt64)
{
    uint64 r = ~0;

    String s = StringConverter::toString(r);
    uint64 t;
    StringConverter::parse(s, t);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseSizeT)
{
    size_t r = ~0;

    String s = StringConverter::toString(r);
    size_t t = StringConverter::parseSizeT(s);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseVector3)
{
    Vector3 r(0.12, 3.22, -4.04);

    String s = StringConverter::toString(r);
    Vector3 t = StringConverter::parseVector3(s);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseMatrix4)
{
    Matrix4 r(1.12, 0, 0, 34, 0, 0.87, 0, 20, 0, 0, 0.56, 10, 0, 0, 0, 1);

    String s = StringConverter::toString(r);
    Matrix4 t = StringConverter::parseMatrix4(s);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseQuaternion)
{
    Quaternion r(1.12, 0.87, 0.67, 1);

    String s = StringConverter::toString(r);
    Quaternion t = StringConverter::parseQuaternion(s);

    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseBool)
{
    bool r = true;
    String s = StringConverter::toString(r);
    bool t = StringConverter::parseBool(s);
    EXPECT_EQ(r, t);

    r = false;
    s = StringConverter::toString(r);
    t = StringConverter::parseBool(s);
    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,ParseColourValue)
{
    ColourValue r(0.34, 0.44, 0.77, 1.0);

    String s = StringConverter::toString(r);
    ColourValue t = StringConverter::parseColourValue(s);
    EXPECT_EQ(r, t);
}
//--------------------------------------------------------------------------
TEST_F(StringTests,EndsWith)
{
    String s = "Hello World!";

    EXPECT_TRUE(StringUtil::endsWith(s, "world!"));
    EXPECT_FALSE(StringUtil::endsWith(s, "hello"));
    EXPECT_FALSE(StringUtil::endsWith(s, "world!", false));
    EXPECT_FALSE(StringUtil::endsWith(s, "", false));
}

TEST_F(StringTests,StartsWith)
{
    String s = "Hello World!";

    EXPECT_TRUE(StringUtil::startsWith(s, "hello"));
    EXPECT_FALSE(StringUtil::startsWith(s, "world"));
    EXPECT_FALSE(StringUtil::startsWith(s, "hello", false));
    EXPECT_FALSE(StringUtil::startsWith(s, "", false));
}
