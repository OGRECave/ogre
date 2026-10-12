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

#include <algorithm>
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

static void checkMacroArgumentCondition(const String& condition, const String& defines,
                                        bool enabled, bool commonFirst, bool forward = false)
{
    SCOPED_TRACE(condition + defines);
    SCOPED_TRACE(commonFirst);
    SCOPED_TRACE(forward);
    CPreprocessor prep;
    String common = "uniform TYPE common;\n";
    String src = "#define TYPE vec4\n"
                 "#define OGRE_UNIFORMS(params) params\n"
                 "#define FORWARD(params) OGRE_UNIFORMS(params)\n" + defines +
                 (forward ? "FORWARD(\n" : "OGRE_UNIFORMS(\n") +
                 (commonFirst ? common : "") + condition + "\n"
                 "uniform TYPE enabled;\n"
                 "#else\n"
                 "uniform TYPE disabled;\n"
                 "#endif\n" + (commonFirst ? "" : common) + ")\nfloat after;\n";

    size_t olen = 0;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    ASSERT_NE(out, nullptr);
    String str(out, olen);
    free(out);

    String expected = (commonFirst ? "uniform vec4 common; " : "") +
                      String(enabled ? "uniform vec4 enabled; " : "uniform vec4 disabled; ") +
                      (commonFirst ? "" : "uniform vec4 common; ") + "float after;";
    EXPECT_EQ(StringUtil::split(str), StringUtil::split(expected));

    auto after = str.find("float after;");
    ASSERT_NE(after, String::npos);
    EXPECT_EQ(std::count(src.begin(), src.begin() + src.find("float after;"), '\n'),
              std::count(str.begin(), str.begin() + after, '\n'));
}

TEST(CPreprocessorTests, MacroArgumentIfDef)
{
    for (const String& defines : {String(), String("#define FOO\n"), String("#define FOO 0\n"),
                                 String("#define FOO 1\n"), String("#define FOO BAR\n#define BAR 0\n")})
        for (bool commonFirst : {false, true})
            checkMacroArgumentCondition("#ifdef FOO", defines, !defines.empty(), commonFirst);
}

TEST(CPreprocessorTests, MacroArgumentIfNDef)
{
    for (const String& defines : {String(), String("#define FOO\n"), String("#define FOO 0\n"),
                                 String("#define FOO 1\n"), String("#define FOO BAR\n#define BAR 0\n")})
        for (bool commonFirst : {false, true})
            for (bool forward : {false, true})
                checkMacroArgumentCondition("#ifndef FOO", defines, defines.empty(), commonFirst, forward);
}

TEST(CPreprocessorTests, MacroArgumentDefinedExpression)
{
    for (const String& defines : {String(), String("#define FOO\n"), String("#define FOO 0\n"),
                                 String("#define FOO 1\n"), String("#define FOO BAR\n#define BAR 0\n")})
        for (bool commonFirst : {false, true})
            for (bool forward : {false, true})
                checkMacroArgumentCondition("#if defined(FOO)", defines, !defines.empty(), commonFirst, forward);
}

TEST(CPreprocessorTests, MacroArgumentElifDefined)
{
    for (const String& defines : {String(), String("#define FOO\n"), String("#define FOO 0\n"),
                                 String("#define FOO 1\n"), String("#define FOO BAR\n#define BAR 0\n")})
        for (bool commonFirst : {false, true})
            for (bool forward : {false, true})
                checkMacroArgumentCondition("#if 0\nuniform TYPE rejected;\n#elif defined(FOO)",
                                            defines, !defines.empty(), commonFirst, forward);
}

TEST(CPreprocessorTests, MacroArgumentForwardedIfDef)
{
    for (const String& defines : {String(), String("#define FOO\n"), String("#define FOO 0\n"),
                                 String("#define FOO 1\n"), String("#define FOO BAR\n#define BAR 0\n")})
        for (bool commonFirst : {false, true})
            checkMacroArgumentCondition("#ifdef FOO", defines, !defines.empty(), commonFirst, true);
}

TEST(CPreprocessorTests, MacroRescanBorrowedTail)
{
    CPreprocessor prep;
    String src = "#define EMIT(hash, body) hash if 1 body hash endif\n"
                 "EMIT(#,\n"
                 "#if 1\n"
                 "float inside;\n"
                 "#endif\n"
                 ")\n"
                 "float after;\n";

    size_t olen = 0;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    ASSERT_NE(out, nullptr);
    String str(out, olen);
    free(out);
    EXPECT_EQ(StringUtil::split(str), StringUtil::split("float inside; float after;"));
    auto after = str.find("float after;");
    ASSERT_NE(after, String::npos);
    EXPECT_EQ(std::count(src.begin(), src.begin() + src.find("float after;"), '\n'),
              std::count(str.begin(), str.begin() + after, '\n'));
}

TEST(CPreprocessorTests, MacroArgumentRepeatedMultiline)
{
    CPreprocessor prep;
    String src = "#define DUP(value) ((value)+(value))\n"
                 "float sum = DUP(1.0\n"
                 "+2.0);\n"
                 "float after;\n";

    size_t olen = 0;
    char* out = prep.Parse(src.c_str(), src.size(), olen);
    ASSERT_NE(out, nullptr);
    String str(out, olen);
    free(out);

    String compact = str;
    compact.erase(std::remove_if(compact.begin(), compact.end(), [](char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }), compact.end());
    EXPECT_EQ(compact, "floatsum=((1.0+2.0)+(1.0+2.0));floatafter;");
    auto after = str.find("float after;");
    ASSERT_NE(after, String::npos);
    EXPECT_EQ(std::count(src.begin(), src.begin() + src.find("float after;"), '\n'),
              std::count(str.begin(), str.begin() + after, '\n'));
}

static void checkMacroExpansion(const String& source, const String& expected)
{
    CPreprocessor prep;
    size_t length = 0;
    char* output = prep.Parse(source.c_str(), source.size(), length);
    ASSERT_NE(output, nullptr);
    String result(output, length);
    free(output);
    EXPECT_EQ(StringUtil::split(result), StringUtil::split(expected));
}

TEST(CPreprocessorTests, MacroRecursiveAlias)
{
    checkMacroExpansion("#define foo bar foo\n#define test foo\ntest\n",
                        "bar foo");
}

TEST(CPreprocessorTests, MacroRecursiveAliasRepeated)
{
    checkMacroExpansion("#define foo bar foo\n#define test foo foo\ntest foo test\n",
                        "bar foo bar foo bar foo bar foo bar foo");
}

TEST(CPreprocessorTests, MacroMutualRecursionThroughAlias)
{
    checkMacroExpansion("#define A B + 1\n#define B A + 2\n#define USE A\nUSE\n",
                        "A + 2 + 1");
}

TEST(CPreprocessorTests, MacroRecursiveFunctionThroughWrapper)
{
    checkMacroExpansion("#define SELF(x) (SELF(x))\n#define FORWARD(x) SELF(x)\nFORWARD(1)\n",
                        "(SELF(1))");
}

TEST(CPreprocessorTests, MacroArgumentsDoNotExpandEachOther)
{
    checkMacroExpansion("#define PAIR(a, b) a + b\nPAIR(b, a)\n",
                        "b + a");
}

TEST(CPreprocessorTests, MacroPastedNameRescan)
{
    checkMacroExpansion("#define HelloWorld 42\n#define CONCAT(a, b) a##b\nCONCAT(Hello, World)\n",
                        "42");
}

TEST(CPreprocessorTests, MacroRepeatedArgumentStorage)
{
    String argument(256, 'x');
    checkMacroExpansion("#define REPEAT(x) x + x + x\nREPEAT(" + argument + ")\n",
                        argument + " + " + argument + " + " + argument);
}

TEST(CPreprocessorTests, MacroArgumentsKeepStringsAndComments)
{
    checkMacroExpansion("#define TEXT(x) print(\"x\", x) /* x */\nTEXT(value)\n",
                        "print(\"x\", value)");
}

TEST(CPreprocessorTests, MacroRecursiveArgumentStaysSuppressed)
{
    checkMacroExpansion("#define foo bar foo\n#define ID(x) x\nID(foo)\n",
                        "bar foo");
}

TEST(CPreprocessorTests, MacroRecursiveAndHigherOrderArguments)
{
    checkMacroExpansion("#define foo bar foo\n#define LEFT(a, b) a\n"
                        "#define USE(a, b, F) foo F(a, b)\nUSE(1, 2, LEFT)\n",
                        "bar foo 1");
}

TEST(CPreprocessorTests, MacroRecursiveConditionalExpression)
{
    checkMacroExpansion("#define A A\n#define ID(x) x\n#if ID(A)\nbad\n#else\ngood\n#endif\n",
                        "good");
    checkMacroExpansion("#define A A\n#define ID(x) x\n#if ID(A + 1)\ngood\n#else\nbad\n#endif\n",
                        "good");
}

TEST(CPreprocessorTests, MacroNestedRecursiveArguments)
{
    checkMacroExpansion("#define foo bar foo\n#define ID(x) x\n#define DUP(x) x + x\n"
                        "ID(ID(foo)) DUP(foo) foo\n",
                        "bar foo bar foo + bar foo bar foo");
    checkMacroExpansion("#define A B + 1\n#define B A + 2\n#define ID(x) x\nID(A) A\n",
                        "A + 2 + 1 A + 2 + 1");
}

TEST(CPreprocessorTests, MacroMutualFunctionsStaySuppressed)
{
    checkMacroExpansion("#define A(x) B(x)\n#define B(x) A(x)\n#define ID(x) x\nID(A(1)) A(2)\n",
                        "A(1) A(2)");
}

TEST(CPreprocessorTests, MacroRecursiveFreshInvocations)
{
    checkMacroExpansion("#define F(x) G(x)\n#define G(x) F(x)\n#define ALIAS F\n"
                        "#define WRAP(x) F(x) ALIAS(x)\nWRAP(3)\n",
                        "F(3) F(3)");
}
