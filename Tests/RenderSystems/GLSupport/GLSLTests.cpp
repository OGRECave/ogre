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

#include "GLSL/OgreGLSLPreprocessor.h"
#include "OgreString.h"

#include <gtest/gtest.h>

using namespace Ogre;

TEST(CPreprocessorTests, MacroBraces)
{
    CPreprocessor prep;
    String src = "#define MY_MACRO(x) print( x )\n"
                 "MY_MACRO( (myValue * 3) * 2)";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "print( (myValue * 3) * 2 )");
    free(out);
}

TEST(CPreprocessorTests, MacroExpansion)
{
    CPreprocessor prep;
    String src = "#define mad( a, b, c ) fma( a, b, c )\n"
                 "mad( x.s, y, a )";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "fma( x.s, y, a )");
    free(out);
}

TEST(CPreprocessorTests, IfDef)
{
    CPreprocessor prep;
    String src = "#define A\n"
                 "#ifndef A\n"
                 "undefined\n"
                 "#else\n"
                 "defined\n"
                 "#endif";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "defined");
    free(out);
}

TEST(CPreprocessorTests, ElseIf)
{
    CPreprocessor prep;
    String src = "#define A 0\n"
                 "#if A == 1\n"
                 "value is 1\n"
                 "#elif A == 0\n"
                 "value is 0\n"
                 "#elif A == 2\n"
                 "value is 2\n"
                 "#else\n"
                 "value is 3\n"
                 "#endif";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "value is 0");
    free(out);
}

TEST(CPreprocessorTests, MacroMacroArgument)
{
    CPreprocessor prep;
    String src = "#define LEFT(left, right) (left)\n"
                 "#define USE(a, b, F) F(a, b)\n"
                 "USE(A, B, LEFT)";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "(A)");
    free(out);
}

TEST(CPreprocessorTests, MacroMacroArgumentAndExpansion)
{
    CPreprocessor prep;
    String src = "#define RIGHT(left, right) (right)\n"
                 "#define LEFT(left, right) (left)\n"
                 "#define USE(a, b, F) F(a, b)\n"
                 "USE(RIGHT(A, C), B, LEFT)";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "((C))");
    free(out);
}

TEST(CPreprocessorTests, MacroRecursion1)
{
    CPreprocessor prep;
    String src = "#define U(b) (b,U(b),b)\n"
                 "U(U)";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "(U,U(U),U)");
    free(out);
}


TEST(CPreprocessorTests, MacroRecursion2)
{
    CPreprocessor prep;
    String src = "#define Z(b) (b,b,b)\n"
                 "Z(Z+Z(1))";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "(Z+(1,1,1),Z+(1,1,1),Z+(1,1,1))");
    free(out);
}

TEST(CPreprocessorTests, MacroRecursion3)
{
    CPreprocessor prep;
    String src = "#define Z(b) (b,b,b)\n"
                 "Z(Z(Z(1)+Z(2)))";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "(((1,1,1)+(2,2,2),(1,1,1)+(2,2,2),(1,1,1)+(2,2,2)),((1,1,1)+(2,2,2),(1,1,1)+(2,2,2),(1,1,1)+(2,2,2)),((1,1,1)+(2,2,2),(1,1,1)+(2,2,2),(1,1,1)+(2,2,2)))");
    free(out);
}

TEST(CPreprocessorTests, MacroConcat)
{
    CPreprocessor prep;
    String src = "#define concat( a, b ) a##b\n"
                 "concat( Hello , World )";

    size_t olen;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    String str(out, olen);
    StringUtil::trim(str);
    EXPECT_EQ(str, "HelloWorld");
    free(out);
}