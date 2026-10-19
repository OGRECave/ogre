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
#include <cstdlib>
#include <cstring>
#include <vector>
#include <gtest/gtest.h>

using namespace Ogre;

namespace {
/// Parse @p iSource from a buffer owned by this object and remember whether the
/// result points into that buffer (the documented in-situ contract). The result
/// is free()d on destruction only when it does not point into the source, so the
/// free()-unless-inside-the-source rule is exercised on every test.
class InSituParse
{
public:
    InSituParse(CPreprocessor& iPreprocessor, const String& iSource)
        : Source(iSource), Buffer(iSource.begin(), iSource.end())
    {
        Buffer.push_back('\0');
        Output = iPreprocessor.Parse(Buffer.data(), Source.size(), Length);
        InSource = Output && Output >= Buffer.data() && Output <= Buffer.data() + Source.size();
    }

    ~InSituParse()
    {
        if (Output && !InSource)
            free(Output);
    }

    InSituParse(const InSituParse&) = delete;
    InSituParse& operator=(const InSituParse&) = delete;

    bool IsNull() const { return Output == nullptr; }
    String Text() const { return Output ? String(Output, Length) : String(); }

    /// The source (kept alive so that macros defined from it stay valid)
    String Source;
    /// The source buffer with an extra NUL byte beyond the passed length
    std::vector<char> Buffer;
    char* Output = nullptr;
    size_t Length = 0;
    bool InSource = false;
};
} // namespace

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

// ---------------------------------------------------------------------------
// In-situ contract: text is copied only when it changes, unchanged input is
// returned as a pointer *into* the source, and the caller frees the result only
// when it does not lie inside the source buffer.
// ---------------------------------------------------------------------------

TEST(CPreprocessorTests, ParseKeepsUnchangedInputInSitu)
{
    CPreprocessor prep;
    String src = "layout(location = 0) in vec3 position;\n"
                 "void main() { gl_Position = vec4(position, 1.0); }\n";
    InSituParse result(prep, src);
    ASSERT_FALSE(result.IsNull());
    EXPECT_TRUE(result.InSource);
    EXPECT_EQ(result.Text(), src);
}

TEST(CPreprocessorTests, ParseKeepsBlankInputInSitu)
{
    CPreprocessor prep;
    InSituParse result(prep, "\n\n\n");
    ASSERT_FALSE(result.IsNull());
    EXPECT_TRUE(result.InSource);
    EXPECT_EQ(result.Text(), "\n\n\n");
}

TEST(CPreprocessorTests, ParseCopiesWhenTextChanges)
{
    CPreprocessor prep;
    InSituParse result(prep, "/* comment */");
    ASSERT_FALSE(result.IsNull());
    EXPECT_FALSE(result.InSource);
    EXPECT_EQ(result.Text(), " ");
}

TEST(CPreprocessorTests, ParseReturnsNullForEmptyInput)
{
    CPreprocessor prep;
    InSituParse result(prep, "");
    EXPECT_TRUE(result.IsNull());
}

TEST(CPreprocessorTests, ParseWithoutTrailingNewlineIsFreeable)
{
    // Regression: the original returned a pointer into a static newline table
    // here, which the documented free()-unless-inside-the-source rule then freed.
    CPreprocessor prep;
    InSituParse result(prep, "#if 0\na\n#endif");
    ASSERT_FALSE(result.IsNull());
    EXPECT_FALSE(result.InSource);
    EXPECT_EQ(result.Text(), "\n\n");
}

// ---------------------------------------------------------------------------
// Object lifetime: switches and API state persist across Parse calls, and
// macros defined from a source keep views into it.
// ---------------------------------------------------------------------------

TEST(CPreprocessorTests, DefineUndefAndObjectReuse)
{
    CPreprocessor prep;
    const char* name = "ENABLE";
    const char* value = "3";
    const char* symbol = "SYM";
    prep.Define(name, std::strlen(name), value, std::strlen(value));
    prep.Define(symbol, std::strlen(symbol));

    // Declared first, so it outlives the later parses that still reference LATER
    InSituParse first(prep,
        "#if ENABLE == 3 && defined(SYM)\n"
        "yes ENABLE\n"
        "#endif\n"
        "#define LATER 9\n");
    ASSERT_FALSE(first.IsNull());
    EXPECT_EQ(first.Text(), "\nyes 3\n\n\n");

    InSituParse second(prep, "LATER ENABLE\n");
    ASSERT_FALSE(second.IsNull());
    EXPECT_EQ(second.Text(), "9 3\n");

    EXPECT_TRUE(prep.Undef(name, std::strlen(name)));
    EXPECT_FALSE(prep.Undef(name, std::strlen(name)));
    EXPECT_FALSE(prep.Undef("NOPE", 4));

    InSituParse third(prep, "LATER ENABLE\n");
    ASSERT_FALSE(third.IsNull());
    EXPECT_EQ(third.Text(), "9 ENABLE\n");
}

// ---------------------------------------------------------------------------
// Robustness limits and malformed input
// ---------------------------------------------------------------------------

TEST(CPreprocessorTests, MacroArgumentLimit)
{
    CPreprocessor prep;
    InSituParse sixteen(prep,
        "#define F(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) a0+a15\n"
        "F(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)\n");
    ASSERT_FALSE(sixteen.IsNull());
    EXPECT_EQ(StringUtil::split(sixteen.Text()), StringUtil::split("1+16"));

    CPreprocessor tooMany;
    InSituParse seventeen(tooMany,
        "#define F(a) a\n"
        "F(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17)\n");
    EXPECT_TRUE(seventeen.IsNull());
}

TEST(CPreprocessorTests, NestedIfLimit)
{
    String open, close;
    for (int i = 0; i < 31; ++i)
    {
        open += "#if 1\n";
        close += "#endif\n";
    }

    CPreprocessor prep;
    InSituParse ok(prep, open + "x\n" + close);
    ASSERT_FALSE(ok.IsNull());
    EXPECT_NE(ok.Text().find('x'), String::npos);

    String open32, close32;
    for (int i = 0; i < 32; ++i)
    {
        open32 += "#if 1\n";
        close32 += "#endif\n";
    }

    CPreprocessor tooDeep;
    InSituParse overflow(tooDeep, open32 + "x\n" + close32);
    EXPECT_TRUE(overflow.IsNull());
}

TEST(CPreprocessorTests, EmbeddedNulBytes)
{
    CPreprocessor prep;
    String src("a\0b\n#define X 1\nX", 16);
    src.append("\0X\n", 3);

    InSituParse result(prep, src);
    ASSERT_FALSE(result.IsNull());
    EXPECT_FALSE(result.InSource);
    // Embedded NULs are ordinary text; only the macro name after them expands.
    const char expected[] = {'a', '\0', 'b', '\n', '\n', '\0', '1', '\n'};
    EXPECT_EQ(result.Text(), String(expected, sizeof(expected)));
}

TEST(CPreprocessorTests, CrLfWithLineContinuation)
{
    CPreprocessor prep;
    InSituParse result(prep, "#define A 1 \\\r\n + 2\r\nA\r\nx \\\r\ny\r\n");
    ASSERT_FALSE(result.IsNull());
    EXPECT_NE(result.Text().find("1  + 2"), String::npos);
    EXPECT_NE(result.Text().find("x y"), String::npos);
}

TEST(CPreprocessorTests, UnterminatedStringIsPassedThrough)
{
    CPreprocessor prep;
    InSituParse result(prep, "x \"abc\ny");
    ASSERT_FALSE(result.IsNull());
    EXPECT_TRUE(result.InSource);
    EXPECT_EQ(result.Text(), "x \"abc\ny");
}

TEST(CPreprocessorTests, UnterminatedBlockCommentIsClosed)
{
    CPreprocessor prep;
    InSituParse result(prep, "x /* abc\ny");
    ASSERT_FALSE(result.IsNull());
    EXPECT_EQ(result.Text(), "x  \n");
}

TEST(CPreprocessorTests, DirectiveAtEndWithoutNewline)
{
    CPreprocessor prep;
    InSituParse result(prep, "#define A 1\nA\n#if 1");
    ASSERT_FALSE(result.IsNull());
    EXPECT_EQ(StringUtil::split(result.Text()), StringUtil::split("1"));
}

TEST(CPreprocessorTests, UnclosedIfAtEndOfSource)
{
    CPreprocessor prep;
    InSituParse result(prep, "#if 1\nx\n");
    EXPECT_TRUE(result.IsNull());
}

TEST(CPreprocessorTests, ShiftCountIsMasked)
{
    // Regression for the UB fix: shifting by at least the operand width is
    // masked instead of invoking undefined behaviour.
    checkMacroExpansion("#if 1 << 910\nyes\n#endif\n", "yes");
}

TEST(CPreprocessorTests, NegativeIntermediateIsRejected)
{
    // Preserved quirk: a negative intermediate #if result is not a valid number.
    CPreprocessor prep;
    InSituParse result(prep, "#if 1 - 2\nyes\n#else\nno\n#endif\n");
    EXPECT_TRUE(result.IsNull());
}

TEST(CPreprocessorTests, DivisionByZeroIsRejected)
{
    CPreprocessor prep;
    InSituParse result(prep, "#if 1/0\nyes\n#endif\n");
    EXPECT_TRUE(result.IsNull());
}

TEST(CPreprocessorTests, IfExpressionHexOctalDefinedElif)
{
    checkMacroExpansion("#define V 0x10\n"
                        "#if V == 16 && defined(V) && !defined(W)\n"
                        "a\n"
                        "#elif 010 == 8\n"
                        "b\n"
                        "#else\n"
                        "c\n"
                        "#endif\n",
                        "a");
}

TEST(CPreprocessorTests, MutuallyRecursiveObjectMacros)
{
    checkMacroExpansion("#define A B\n#define B A\nA B\n", "A B");
}

TEST(CPreprocessorTests, SelfReferentialMacro)
{
    checkMacroExpansion("#define X X+1\nX\n", "X+1");
}
