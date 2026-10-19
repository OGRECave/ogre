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

#include "OgreGLSLPreprocessor.h"
#include "OgreLogManager.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <utility>
#include <vector>

/*
 * Design notes
 * ------------
 * The preprocessor follows the "in-situ" philosophy known from pugixml:
 *
 *  - The Lexer never copies. A Token is a *view* (pointer + length) into the
 *    source buffer, so scanning and passing tokens around allocates nothing.
 *  - Text is only copied when it really changes (macro expansion, comment
 *    removal, ...). A Text is therefore either a borrowed span of the source
 *    or an owned malloc'ed buffer, and consecutive appends of adjacent source
 *    spans simply extend the span. Untouched input is returned untouched.
 *  - Small value types with explicit ownership (move-only Text/Token) replace
 *    the original "copy steals the buffer" trick.
 *  - Errors travel as a Token of type TK_ERROR plus one log message; there are
 *    no exceptions on the parsing path.
 *
 * Layers (bottom to top): character classes -> StringView -> Text -> Token ->
 * Lexer -> MacroTable -> Processor (directives, macro expansion, #if
 * expressions) -> CPreprocessor (public facade).
 */

namespace Ogre {

    // Limit max number of macro arguments to this
    static const size_t MAX_MACRO_ARGS = 16;

    /// Appended to macro parameter names to avoid clashes with argument text,
    /// see Processor::GetArgument().
    static const char ARG_SUFFIX[] = "__arg_";
    static const size_t ARG_SUFFIX_LEN = sizeof(ARG_SUFFIX) - 1;

    // -----------------------------------------------------------------------
    // Character classes (locale independent, "C" locale semantics)
    // -----------------------------------------------------------------------

    static inline bool isSpace(char c) { return c == ' ' || (c >= '\t' && c <= '\r'); }
    static inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
    static inline bool isXDigit(char c) { return isDigit(c) || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f'); }
    static inline bool isAlpha(char c) { return (c | 0x20) >= 'a' && (c | 0x20) <= 'z'; }
    static inline bool isKeywordChar(char c) { return c == '_' || isAlpha(c) || isDigit(c); }

    /// Return closest power of two not smaller than given number (0 stays 0)
    static size_t ClosestPow2(size_t x)
    {
        if (!(x & (x - 1)))
            return x;
        while (x & (x + 1))
            x |= (x + 1);
        return x + 1;
    }

    /// Growth policy of owned buffers
    static size_t CapacityFor(size_t iSize) { return std::max<size_t>(64, ClosestPow2(iSize)); }

    /// Sets a flag for the lifetime of the object
    class ScopedFlag
    {
    public:
        explicit ScopedFlag(bool& iFlag) : Flag(iFlag) { Flag = true; }
        ~ScopedFlag() { Flag = false; }
        ScopedFlag(const ScopedFlag&) = delete;
        ScopedFlag& operator=(const ScopedFlag&) = delete;

    private:
        bool& Flag;
    };

    // -----------------------------------------------------------------------
    // CPreprocessor::Text
    // -----------------------------------------------------------------------

    CPreprocessor::Text::Text(Text&& iOther) noexcept
        : String(iOther.String), Length(iOther.Length), Allocated(iOther.Allocated),
          DisabledMacros(std::move(iOther.DisabledMacros))
    {
        iOther.String = nullptr;
        iOther.Length = iOther.Allocated = 0;
        iOther.DisabledMacros.clear();
    }

    CPreprocessor::Text& CPreprocessor::Text::operator=(Text&& iOther) noexcept
    {
        if (this != &iOther)
        {
            FreeStorage();
            String = iOther.String;
            Length = iOther.Length;
            Allocated = iOther.Allocated;
            DisabledMacros = std::move(iOther.DisabledMacros);
            iOther.String = nullptr;
            iOther.Length = iOther.Allocated = 0;
            iOther.DisabledMacros.clear();
        }
        return *this;
    }

    CPreprocessor::Text CPreprocessor::Text::Borrow() const
    {
        Text t(String, Length);
        t.DisabledMacros = DisabledMacros;
        return t;
    }

    CPreprocessor::Text CPreprocessor::Text::Clone() const
    {
        Text t;
        t.DisabledMacros = DisabledMacros;
        if (String)
        {
            t.EnsureAllocated(Length);
            std::memcpy(t.Buffer(), String, Length);
            t.Length = Length;
        }
        return t;
    }

    void CPreprocessor::Text::Truncate(size_t iLength)
    {
        assert(iLength <= Length);
        Length = iLength;
    }

    void CPreprocessor::Text::ResizeBorrowed(size_t iLength)
    {
        assert(!IsAllocated());
        Length = iLength;
    }

    void CPreprocessor::Text::Assign(const char* iString, size_t iLength)
    {
        DisabledMacros.clear();
        if (!IsAllocated())
            String = nullptr;
        Length = 0;
        EnsureAllocated(iLength);
        std::memcpy(Buffer(), iString, iLength);
        Length = iLength;
    }

    void CPreprocessor::Text::AppendView(StringView iString)
    {
        if (!iString.String)
            return;
        if (!String)
        {
            String = iString.String;
            Length = iString.Length;
            return;
        }
        AppendChars(iString.String, iString.Length);
    }

    void CPreprocessor::Text::AppendCopy(StringView iString)
    {
        if (!iString.String)
            return;
        EnsureAllocated(iString.Length);
        std::memcpy(Buffer() + Length, iString.String, iString.Length);
        Length += iString.Length;
    }

    void CPreprocessor::Text::Append(const Text& iOther)
    {
        if (!iOther.String)
            return;
        if (!String)
        {
            if (iOther.IsAllocated())
                *this = iOther.Clone();
            else
            {
                String = iOther.String;
                Length = iOther.Length;
                DisabledMacros = iOther.DisabledMacros;
            }
            return;
        }
        AppendDisabled(iOther);
        AppendChars(iOther.String, iOther.Length);
    }

    void CPreprocessor::Text::Append(Text&& iOther)
    {
        if (!String && iOther.IsAllocated())
            *this = std::move(iOther);
        else
            Append(static_cast<const Text&>(iOther));
    }

    void CPreprocessor::Text::AppendNL(int iCount)
    {
        if (iCount <= 0)
            return;
        EnsureAllocated(size_t(iCount));
        std::memset(Buffer() + Length, '\n', size_t(iCount));
        Length += size_t(iCount);
    }

    char* CPreprocessor::Text::Release()
    {
        char* p = const_cast<char*>(String);
        String = nullptr;
        Length = Allocated = 0;
        return p;
    }

    bool CPreprocessor::Text::IsDisabled(StringView iMacroName) const
    {
        for (const DisabledMacro& disabled : DisabledMacros)
            if (disabled.Begin == 0 && disabled.End >= Length &&
                StringView(disabled.Name.data(), disabled.Name.size()) == iMacroName)
                return true;
        return false;
    }

    void CPreprocessor::Text::Disable(StringView iMacroName)
    {
        DisabledMacros.push_back({0, Length, std::string(iMacroName.String, iMacroName.Length)});
    }

    void CPreprocessor::Text::InheritDisabled(const Text& iSource, size_t iOffset, size_t iLength)
    {
        for (const DisabledMacro& disabled : iSource.DisabledMacros)
            if (disabled.Begin <= iOffset && disabled.End >= iOffset + iLength)
                DisabledMacros.push_back({0, iLength, disabled.Name});
    }

    void CPreprocessor::Text::FreeStorage()
    {
        if (Allocated)
            std::free(Buffer());
    }

    void CPreprocessor::Text::AppendDisabled(const Text& iOther)
    {
        for (const DisabledMacro& disabled : iOther.DisabledMacros)
            DisabledMacros.push_back({Length + disabled.Begin, Length + disabled.End, disabled.Name});
    }

    void CPreprocessor::Text::EnsureAllocated(size_t iExtra)
    {
        const size_t wanted = CapacityFor(Length + iExtra);
        if (Allocated)
        {
            if (wanted != Allocated)
            {
                void* p = std::realloc(Buffer(), wanted);
                if (!p)
                    throw std::bad_alloc();
                String = static_cast<char*>(p);
                Allocated = wanted;
            }
        }
        else
        {
            char* p = static_cast<char*>(std::malloc(wanted));
            if (!p)
                throw std::bad_alloc();
            if (Length)
                std::memcpy(p, String, Length);
            String = p;
            Allocated = wanted;
        }
    }

    void CPreprocessor::Text::AppendChars(const char* iString, size_t iLength)
    {
        if (!IsAllocated() && String + Length == iString)
        {
            Length += iLength; // in situ: the new span directly follows ours
            return;
        }
        EnsureAllocated(iLength);
        std::memcpy(Buffer() + Length, iString, iLength);
        Length += iLength;
    }

    // -----------------------------------------------------------------------
    // CPreprocessor::Token
    // -----------------------------------------------------------------------

    int CPreprocessor::Token::CountNL() const
    {
        if (Type == TK_EOS || Type == TK_ERROR || !String())
            return 0;
        return int(std::count(String(), String() + Length(), '\n'));
    }

    bool CPreprocessor::Token::GetValue(long& oValue) const
    {
        const char* s = String();
        const size_t n = Length();
        size_t i = 0;
        while (i < n && isSpace(s[i]))
            ++i;

        long base = 10;
        if (i < n && s[i] == '0')
        {
            if (n > i + 1 && s[i + 1] == 'x')
            {
                base = 16;
                i += 2;
            }
            else
                base = 8;
        }

        long val = 0;
        for (; i < n && !isSpace(s[i]); ++i)
        {
            const char c = s[i];
            long digit;
            if (isDigit(c))
                digit = c - '0';
            else if (isAlpha(c))
                digit = 10 + ((c | 0x20) - 'a');
            else
                return false;
            if (digit >= base)
                return false;
            val = val * base + digit;
        }

        // Check that all other characters are just spaces
        for (; i < n; ++i)
            if (!isSpace(s[i]))
                return false;

        oValue = val;
        return true;
    }

    void CPreprocessor::Token::SetValue(long iValue)
    {
        char tmp[24];
        const int len = std::snprintf(tmp, sizeof(tmp), "%ld", iValue);
        Str.Assign(tmp, size_t(len));
        Type = TK_NUMBER;
    }

    // -----------------------------------------------------------------------
    // CPreprocessor::MacroTable
    // -----------------------------------------------------------------------

    CPreprocessor::Macro* CPreprocessor::MacroTable::IsDefined(StringView iName)
    {
        for (Macro& cur : Macros)
            if (cur.Name == iName)
                return &cur;
        return nullptr;
    }

    CPreprocessor::Macro& CPreprocessor::MacroTable::Define(Macro&& iMacro)
    {
        Macros.push_front(std::move(iMacro));
        return Macros.front();
    }

    CPreprocessor::Macro& CPreprocessor::MacroTable::Define(StringView iName, Text&& iValue)
    {
        Macro m;
        m.Name = iName;
        m.Value = Token(Token::TK_TEXT, std::move(iValue));
        return Define(std::move(m));
    }

    void CPreprocessor::MacroTable::PopFront() { Macros.pop_front(); }

    bool CPreprocessor::MacroTable::Undef(StringView iName)
    {
        for (auto prev = Macros.before_begin(), it = std::next(prev); it != Macros.end(); prev = it++)
            if (it->Name == iName)
            {
                // Somebody up the call stack still holds a pointer to it (a
                // macro can #undef itself from within its own expansion or
                // arguments): keep it alive until the table dies.
                if (IsPinned(*it))
                    Retired.splice_after(Retired.before_begin(), Macros, prev);
                else
                    Macros.erase_after(prev);
                return true;
            }
        return false;
    }

    bool CPreprocessor::MacroTable::IsPinned(const Macro& iMacro) const
    {
        return std::find(Pinned.begin(), Pinned.end(), &iMacro) != Pinned.end();
    }

    // -----------------------------------------------------------------------
    // CPreprocessor::Lexer
    // -----------------------------------------------------------------------

    /// Stateless tokenizer: splits a Text into tokens, in situ. Does not
    /// expand macros.
    class CPreprocessor::Lexer
    {
    public:
        Lexer() = default;
        Lexer(const Text& iSource, int iLine) : SourceToken(&iSource), Line(iLine)
        {
            if (iSource.GetString())
            {
                Source = iSource.GetString();
                SourceEnd = Source + iSource.GetLength();
            }
        }

        bool AtEnd() const { return Source >= SourceEnd; }
        int GetLine() const { return Line; }
        const char* GetPosition() const { return Source; }
        const char* GetEnd() const { return SourceEnd; }

        /// Continue lexing at iPosition (used to un-read a token / a directive)
        void Rewind(const char* iPosition, int iLine)
        {
            Source = iPosition;
            Line = iLine;
        }

        Token GetToken()
        {
            if (AtEnd())
                return Token(Token::TK_EOS);

            const char* begin = Source;
            const char c = *Source++;

            if (c == '\n' || (c == '\r' && Peek() == '\n'))
                return ReadNewline(begin, c);
            if (isSpace(c))
                return ReadWhitespace(begin);
            if (isDigit(c))
                return ReadNumber(begin, c);
            if (isKeywordChar(c))
                return ReadKeyword(begin);
            if (c == '"' || c == '\'')
                return ReadString(begin, c);
            if (c == '/' && Peek() == '/')
                return ReadLineComment(begin);
            if (c == '/' && Peek() == '*')
                return ReadComment(begin);
            if (c == '#' && BOL)
                return ReadDirective(begin);
            if (c == '\\' && (Peek() == '\r' || Peek() == '\n'))
                return ReadLineCont(begin);
            return ReadPunctuation(begin, c);
        }

    private:
        /// Current character, or NUL past the end (inputs need not be terminated)
        char Peek(size_t iAhead = 0) const { return Source + iAhead < SourceEnd ? Source[iAhead] : '\0'; }

        Token Make(Token::Kind iType, const char* iBegin) const
        {
            return Token(iType, iBegin, size_t(Source - iBegin));
        }

        Token ReadNewline(const char* iBegin, char c)
        {
            Line++;
            BOL = true;
            if (c == '\r')
                Source++;
            return Make(Token::TK_NEWLINE, iBegin);
        }

        Token ReadWhitespace(const char* iBegin)
        {
            while (Source < SourceEnd && *Source != '\r' && *Source != '\n' && isSpace(*Source))
                Source++;
            return Make(Token::TK_WHITESPACE, iBegin);
        }

        Token ReadNumber(const char* iBegin, char iFirst)
        {
            BOL = false;
            if (iFirst == '0' && Peek() == 'x') // hex numbers
            {
                Source++;
                while (Source < SourceEnd && isXDigit(*Source))
                    Source++;
            }
            else
                while (Source < SourceEnd && isDigit(*Source))
                    Source++;
            return Make(Token::TK_NUMBER, iBegin);
        }

        Token ReadKeyword(const char* iBegin)
        {
            BOL = false;
            while (Source < SourceEnd && isKeywordChar(*Source))
                Source++;
            Token t = Make(Token::TK_KEYWORD, iBegin);
            // A keyword inside a disabled range of the source stays disabled
            if (SourceToken && SourceToken->HasDisabledMacros())
                t.Str.InheritDisabled(*SourceToken, size_t(iBegin - SourceToken->GetString()), t.Length());
            return t;
        }

        Token ReadString(const char* iBegin, char iQuote)
        {
            BOL = false;
            while (Source < SourceEnd && *Source != iQuote)
            {
                if (*Source == '\\' && ++Source >= SourceEnd)
                    break;
                if (*Source == '\n')
                    Line++;
                Source++;
            }
            if (Source < SourceEnd)
                Source++;
            return Make(Token::TK_STRING, iBegin);
        }

        Token ReadLineComment(const char* iBegin)
        {
            BOL = false;
            Source++; // second '/'
            while (Source < SourceEnd && *Source != '\r' && *Source != '\n')
                Source++;
            return Make(Token::TK_LINECOMMENT, iBegin);
        }

        Token ReadComment(const char* iBegin)
        {
            BOL = false;
            Source++; // '*'
            while (Source < SourceEnd && (Peek() != '*' || Peek(1) != '/'))
            {
                if (*Source == '\n')
                    Line++;
                Source++;
            }
            if (Source < SourceEnd && *Source == '*')
                Source++;
            if (Source < SourceEnd && *Source == '/')
                Source++;
            return Make(Token::TK_COMMENT, iBegin);
        }

        Token ReadDirective(const char* iBegin)
        {
            // '#', optional whitespace, then the directive name
            while (Source < SourceEnd && isSpace(*Source))
                Source++;
            while (Source < SourceEnd && !isSpace(*Source))
                Source++;
            return Make(Token::TK_DIRECTIVE, iBegin);
        }

        Token ReadLineCont(const char* iBegin)
        {
            if (Peek() == '\r')
                Source++;
            if (Peek() == '\n')
                Source++;
            Line++;
            BOL = true;
            return Make(Token::TK_LINECONT, iBegin);
        }

        Token ReadPunctuation(const char* iBegin, char c)
        {
            BOL = false;
            const char n = Peek();
            // Handle double-char operators here
            if (((c == '>' || c == '<') && (n == c || n == '=')) || ((c == '!' || c == '=') && n == '=') ||
                ((c == '|' || c == '&' || c == '^') && n == c))
                Source++;
            return Make(Token::TK_PUNCTUATION, iBegin);
        }

        /// The current source text input (for the disabled ranges; may be NULL)
        const Text* SourceToken = nullptr;
        const char* Source = nullptr;
        /// The end of the source text
        const char* SourceEnd = nullptr;
        /// Current line number
        int Line = 0;
        /// True if we are at beginning of line
        bool BOL = true;
    };

    // -----------------------------------------------------------------------
    // CPreprocessor::Processor
    // -----------------------------------------------------------------------

    /**
     * Preprocesses one Token: handles directives, expands macros and writes
     * the output. Instances are cheap; sub-processors (for directive bodies,
     * #if expressions and macro expansion) share the MacroTable.
     */
    class CPreprocessor::Processor
    {
    public:
        explicit Processor(MacroTable& iMacroList) : MacroList(iMacroList) {}
        Processor(MacroTable& iMacroList, const Token& iSource, int iLine)
            : MacroList(iMacroList), Lex(iSource.Str, iLine)
        {
        }

        /// Parse the input token and return a token containing the whole output,
        /// or a token of type TK_ERROR.
        Token Parse(const Token& iSource);

    private:
        /**
         * Nesting state of #if/#elif/#else/#endif, packed into two 32 bit words
         * (bit 0 is the innermost level).
         *  - EnableOutput: the level currently emits output. A sentinel bit below
         *    the outermost level makes an empty stack read as 1.
         *  - EnableElif: no branch of the level has been taken yet.
         */
        class ConditionalStack
        {
        public:
            bool IsEmpty() const { return EnableOutput == 1; }
            bool IsFull() const { return (EnableOutput & (1u << 31)) != 0; }
            /// True if all enclosing levels emit output
            bool IsOutputEnabled() const { return (EnableOutput & (EnableOutput + 1)) == 0; }

            void Push(bool iValue)
            {
                EnableElif <<= 1;
                EnableOutput <<= 1;
                if (iValue)
                    EnableOutput |= 1;
                else
                    EnableElif |= 1;
            }

            /// @return false on underflow (#endif without #if)
            bool Pop()
            {
                EnableElif >>= 1;
                EnableOutput >>= 1;
                return EnableOutput != 0;
            }

            void Elif(bool iValue)
            {
                if (iValue && (EnableElif & 1))
                {
                    EnableOutput |= 1;
                    EnableElif &= ~1u;
                }
                else
                    EnableOutput &= ~1u;
            }

            void Else()
            {
                if ((EnableElif & 1) || (EnableOutput & 1))
                    EnableOutput ^= 1;
            }

            /// Turns the innermost #ifdef into #ifndef
            void Negate()
            {
                EnableOutput ^= 1;
                EnableElif ^= 1;
            }

        private:
            /// A stack of 32 booleans packed into one value :)
            uint32_t EnableOutput = 1;
            uint32_t EnableElif = 0;
        };

        enum class BinaryOp
        {
            None, LogicalOr, LogicalAnd, BitOr, BitXor, BitAnd, Equal, NotEqual, Less, LessEqual,
            Greater, GreaterEqual, ShiftLeft, ShiftRight, Add, Subtract, Multiply, Divide, Modulo
        };

        enum class DirectiveKind { Define, UnDef, IfDef, IfNDef, If, Elif, Else, EndIf, Unknown };

        /// Priority of the operators of the whole expression and of unary operators
        static const int PRIO_PARENTHESIS = 1;
        static const int PRIO_UNARY = 12;

        static BinaryOp ParseBinaryOp(StringView iOp);
        static int Priority(BinaryOp iOp);
        /// @return false on division by zero
        static bool Apply(BinaryOp iOp, long vlop, long vrop, long& oValue);
        /// #define and #undef are only recognised in emitting regions; elsewhere
        /// they are passed through like any unknown directive.
        static DirectiveKind Classify(StringView iName, bool iOutputEnabled);

        // lexing and macro expansion
        Token GetToken(bool iExpand);
        Token ExpandMacro(Token iToken);
        Token Expand(Macro& iMacro, const std::vector<Token>& iArgs);
        Token ExpandDefined(const std::vector<Token>& iArgs);
        Token GetArgument(Token& oArg, bool iExpand, bool shouldAppendArg);
        Token GetArguments(std::vector<Token>& oArgs, bool iExpand, bool shouldAppendArg);

        // #if expressions
        Token GetExpression(Token& oResult, int iLine, int iOpPriority = 0);
        bool GetValue(const Token& iToken, long& oValue, int iLine);
        bool GetValueDef(const Token& iToken, long& oValue, int iLine);

        // directives
        Token HandleDirective(Token iToken, int iLine);
        Token GetDirectiveBody(Token& oLast);
        bool HandleDefine(Token iBody, int iLine);
        bool HandleUnDef(const Token& iBody, int iLine);
        bool HandleIfDef(const Token& iBody, int iLine);
        bool HandleIf(const Token& iBody, int iLine);
        bool HandleIf(bool iValue, int iLine);
        bool HandleElif(const Token& iBody, int iLine);
        bool HandleElse(const Token& iBody, int iLine);
        bool HandleEndIf(const Token& iBody, int iLine);
        bool WarnGarbage(Lexer& ioLex, int iLine);

        /// Call the error handler
        void Error(int iLine, const std::string& iError, const Token* iToken = nullptr) const;

        /// The list of macros defined so far (shared with sub-processors)
        MacroTable& MacroList;
        Lexer Lex;
        ConditionalStack Conditions;
        /// True when processing macro expansion text: it is temporary (macros
        /// defined in it must own their text) and a stray '#' is dropped.
        bool SupplimentaryExpand = false;
    };

    // --- operators --------------------------------------------------------

    CPreprocessor::Processor::BinaryOp CPreprocessor::Processor::ParseBinaryOp(StringView iOp)
    {
        if (iOp.Length == 1)
        {
            switch (iOp[0])
            {
            case '|': return BinaryOp::BitOr;
            case '^': return BinaryOp::BitXor;
            case '&': return BinaryOp::BitAnd;
            case '<': return BinaryOp::Less;
            case '>': return BinaryOp::Greater;
            case '+': return BinaryOp::Add;
            case '-': return BinaryOp::Subtract;
            case '*': return BinaryOp::Multiply;
            case '/': return BinaryOp::Divide;
            case '%': return BinaryOp::Modulo;
            }
        }
        else if (iOp.Length == 2)
        {
            const char a = iOp[0], b = iOp[1];
            if (a == '|' && b == '|') return BinaryOp::LogicalOr;
            if (a == '&' && b == '&') return BinaryOp::LogicalAnd;
            if (a == '=' && b == '=') return BinaryOp::Equal;
            if (a == '!' && b == '=') return BinaryOp::NotEqual;
            if (a == '<' && b == '=') return BinaryOp::LessEqual;
            if (a == '<' && b == '<') return BinaryOp::ShiftLeft;
            if (a == '>' && b == '=') return BinaryOp::GreaterEqual;
            if (a == '>' && b == '>') return BinaryOp::ShiftRight;
        }
        return BinaryOp::None;
    }

    int CPreprocessor::Processor::Priority(BinaryOp iOp)
    {
        switch (iOp)
        {
        case BinaryOp::LogicalOr: return 2;
        case BinaryOp::LogicalAnd: return 3;
        case BinaryOp::BitOr: return 4;
        case BinaryOp::BitXor: return 5;
        case BinaryOp::BitAnd: return 6;
        case BinaryOp::Equal:
        case BinaryOp::NotEqual: return 7;
        case BinaryOp::Less:
        case BinaryOp::LessEqual:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEqual: return 8;
        case BinaryOp::ShiftLeft:
        case BinaryOp::ShiftRight: return 9;
        case BinaryOp::Add:
        case BinaryOp::Subtract: return 10;
        case BinaryOp::Multiply:
        case BinaryOp::Divide:
        case BinaryOp::Modulo: return 11;
        case BinaryOp::None: break;
        }
        return 0;
    }

    bool CPreprocessor::Processor::Apply(BinaryOp iOp, long vlop, long vrop, long& oValue)
    {
        switch (iOp)
        {
        case BinaryOp::LogicalOr: oValue = vlop || vrop; break;
        case BinaryOp::LogicalAnd: oValue = vlop && vrop; break;
        case BinaryOp::BitOr: oValue = vlop | vrop; break;
        case BinaryOp::BitXor: oValue = vlop ^ vrop; break;
        case BinaryOp::BitAnd: oValue = vlop & vrop; break;
        case BinaryOp::Equal: oValue = vlop == vrop; break;
        case BinaryOp::NotEqual: oValue = vlop != vrop; break;
        case BinaryOp::Less: oValue = vlop < vrop; break;
        case BinaryOp::LessEqual: oValue = vlop <= vrop; break;
        case BinaryOp::Greater: oValue = vlop > vrop; break;
        case BinaryOp::GreaterEqual: oValue = vlop >= vrop; break;
        case BinaryOp::ShiftLeft: oValue = vlop << (vrop & 63); break;
        case BinaryOp::ShiftRight: oValue = vlop >> (vrop & 63); break;
        case BinaryOp::Add: oValue = vlop + vrop; break;
        case BinaryOp::Subtract: oValue = vlop - vrop; break;
        case BinaryOp::Multiply: oValue = vlop * vrop; break;
        case BinaryOp::Divide:
        case BinaryOp::Modulo:
            if (vrop == 0)
                return false;
            oValue = iOp == BinaryOp::Divide ? vlop / vrop : vlop % vrop;
            break;
        case BinaryOp::None: oValue = 0; break;
        }
        return true;
    }

    CPreprocessor::Processor::DirectiveKind
    CPreprocessor::Processor::Classify(StringView iName, bool iOutputEnabled)
    {
        if (iName == "define") return iOutputEnabled ? DirectiveKind::Define : DirectiveKind::Unknown;
        if (iName == "undef") return iOutputEnabled ? DirectiveKind::UnDef : DirectiveKind::Unknown;
        if (iName == "ifdef") return DirectiveKind::IfDef;
        if (iName == "ifndef") return DirectiveKind::IfNDef;
        if (iName == "if") return DirectiveKind::If;
        if (iName == "elif") return DirectiveKind::Elif;
        if (iName == "else") return DirectiveKind::Else;
        if (iName == "endif") return DirectiveKind::EndIf;
        return DirectiveKind::Unknown;
    }

    void CPreprocessor::Processor::Error(int iLine, const std::string& iError, const Token* iToken) const
    {
        String msg;
        if (iToken)
            msg = StringUtil::format("line %d: %s: `%.*s'\n", iLine, iError.c_str(), int(iToken->Length()),
                                     iToken->String() ? iToken->String() : "");
        else
            msg = StringUtil::format("line %d: %s\n", iLine, iError.c_str());
        LogManager::getSingleton().logMessage(msg, LML_CRITICAL);
    }

    CPreprocessor::Token CPreprocessor::Processor::GetToken(bool iExpand)
    {
        Token t = Lex.GetToken();
        if (iExpand && t.Type == Token::TK_KEYWORD)
            return ExpandMacro(std::move(t));
        return t;
    }

    // --- main loop --------------------------------------------------------

    CPreprocessor::Token CPreprocessor::Processor::Parse(const Token& iSource)
    {
        Lex = Lexer(iSource.Str, 1);
        Conditions = ConditionalStack();

        // Accumulate output into this token
        Token output(Token::TK_TEXT);
        int empty_lines = 0; // swallowed line continuations, re-added as newlines

        // Enable output only if all embedded #if's were true
        bool old_output_enabled = true;
        bool output_enabled = true;
        int output_disabled_line = 0;

        while (!Lex.AtEnd())
        {
            const int old_line = Lex.GetLine();
            Token t = GetToken(true);

            if (t.Type == Token::TK_DIRECTIVE)
            {
                t = HandleDirective(std::move(t), old_line);

                // Keep line numbers intact across suppressed regions
                output_enabled = Conditions.IsOutputEnabled();
                if (output_enabled != old_output_enabled)
                {
                    if (output_enabled)
                        output.Str.AppendNL(old_line - output_disabled_line);
                    else
                        output_disabled_line = old_line;
                    old_output_enabled = output_enabled;
                }
                if (output_enabled)
                    output.Str.AppendNL(Lex.GetLine() - old_line - t.CountNL());
            }

            switch (t.Type)
            {
            case Token::TK_ERROR:
                return t;

            case Token::TK_EOS:
                return output; // Force termination

            case Token::TK_COMMENT:
                // C comments are replaced with single spaces
                if (output_enabled)
                {
                    output.Str.AppendCopy(" ");
                    output.Str.AppendNL(Lex.GetLine() - old_line);
                }
                break;

            case Token::TK_LINECOMMENT:
                // C++ comments are ignored
                break;

            case Token::TK_LINECONT:
                // Backslash-Newline sequences are deleted, no matter where
                empty_lines++;
                break;

            case Token::TK_PUNCTUATION:
                if (output_enabled && !(SupplimentaryExpand && t.IsPunctuation('#')))
                    output.Str.Append(std::move(t.Str));
                break;

            default:
                if (t.Type == Token::TK_NEWLINE && empty_lines)
                {
                    // Compensate for the backslash-newline combinations
                    // we have encountered, otherwise line numeration is broken
                    if (output_enabled)
                        output.Str.AppendNL(empty_lines);
                    empty_lines = 0;
                }
                // Passthrough all other tokens
                if (output_enabled)
                    output.Str.Append(std::move(t.Str));
                break;
            }
        }

        if (!Conditions.IsEmpty())
        {
            Error(Lex.GetLine(), "Unclosed #if at end of source");
            return Token(Token::TK_ERROR);
        }
        return output;
    }

    // --- macro expansion --------------------------------------------------

    CPreprocessor::Token CPreprocessor::Processor::ExpandMacro(Token iToken)
    {
        Macro* cur = MacroList.IsDefined(iToken.View());
        if (!cur || cur->Expanding || iToken.Str.IsDisabled(cur->Name))
            return iToken;
        MacroTable::Pin pin(MacroList, *cur);

        std::vector<Token> args;
        const int old_line = Lex.GetLine();

        if (!cur->Args.empty())
        {
            Token t = GetArguments(args, !cur->IsDefinedFunc, false);
            if (t.Type == Token::TK_ERROR)
                return t;

            // Put the token back into the source pool; we'll handle it later
            if (t.String())
            {
                // Returned token should never be allocated on heap
                assert(!t.Str.IsAllocated());
                Lex.Rewind(t.String(), Lex.GetLine() - t.CountNL());
            }
            // If a macro is defined with arguments but gets not "called" it
            // should behave like normal text
            if (args.empty() && !t.IsPunctuation('('))
                return iToken;
        }

        if (args.size() > cur->Args.size())
        {
            Error(old_line, "Macro `" + std::string(cur->Name.String, cur->Name.Length) + "' passed " +
                                std::to_string(args.size()) + " arguments, but takes just " +
                                std::to_string(cur->Args.size()));
            return Token(Token::TK_ERROR);
        }

        Token t = cur->IsDefinedFunc ? ExpandDefined(args) : Expand(*cur, args);
        t.Str.AppendNL(Lex.GetLine() - old_line - t.CountNL());
        return t;
    }

    CPreprocessor::Token CPreprocessor::Processor::Expand(Macro& iMacro, const std::vector<Token>& iArgs)
    {
        ScopedFlag expanding(iMacro.Expanding);

        // Expand with a sub-processor in which every parameter is a macro
        Processor cpp(MacroList);
        cpp.SupplimentaryExpand = true;

        size_t i = 0;
        for (; i < iArgs.size(); i++)
            MacroList.Define(iMacro.Args[i].View(), iArgs[i].Str.Borrow());
        for (; i < iMacro.Args.size(); i++) // the rest arguments are empty
            MacroList.Define(iMacro.Args[i].View(), Text("", 0));

        // Repeat until there is no more change between parses (an unchanged pass
        // hands back its input untouched, so a pointer comparison detects that).
        Token xt(Token::TK_TEXT, iMacro.Value.Str.Borrow());
        for (bool first = true;; first = false)
        {
            Token new_xt = cpp.Parse(xt);

            // The result may alias an argument or a slice of xt's buffer, both
            // of which are about to go away: take a private copy.
            if (!new_xt.Str.IsAllocated() && new_xt.String() && new_xt.String() != xt.String())
                new_xt.Str = new_xt.Str.Clone();

            // Remove the extra macros we have defined, only needed once
            if (first)
                for (size_t j = iMacro.Args.size(); j-- > 0;)
                    MacroList.Undef(iMacro.Args[j].View());

            if (new_xt.String() == xt.String())
                break;
            xt = std::move(new_xt);
        }

        // Rescans must not expand this macro again inside its own output
        if (xt.Length())
            xt.Str.Disable(iMacro.Name);
        return xt;
    }

    CPreprocessor::Token CPreprocessor::Processor::ExpandDefined(const std::vector<Token>& iArgs)
    {
        if (iArgs.size() != 1)
        {
            Error(Lex.GetLine(), "The defined() function takes exactly one argument");
            return Token(Token::TK_ERROR);
        }
        return Token(Token::TK_NUMBER, MacroList.IsDefined(iArgs[0].View()) ? "1" : "0", 1);
    }

    CPreprocessor::Token CPreprocessor::Processor::GetArgument(Token& oArg, bool iExpand,
                                                              bool shouldAppendArg)
    {
        do
            oArg = GetToken(iExpand);
        while (Token::IsBlankOrNewline(oArg.Type));

        if (!iExpand)
        {
            // Parameter lists contain bare keywords only
            if (oArg.Type == Token::TK_EOS)
                return Token(Token::TK_EOS);
            if (oArg.IsPunctuation(',') || oArg.IsPunctuation(')'))
            {
                Token t = std::move(oArg);
                oArg = Token(Token::TK_TEXT, "", 0);
                return t;
            }
            if (oArg.Type != Token::TK_KEYWORD)
            {
                Error(Lex.GetLine(), "Unexpected token", &oArg);
                return Token(Token::TK_ERROR);
            }
        }

        size_t braceCount = 0;
        bool directive = oArg.Type == Token::TK_DIRECTIVE;
        if (oArg.IsPunctuation('('))
            ++braceCount;

        size_t len = oArg.Length(); // length without trailing whitespace
        for (;;)
        {
            // Rescan directives later, before expanding their operands
            Token t = GetToken(iExpand && !directive);
            switch (t.Type)
            {
            case Token::TK_EOS:
                Error(Lex.GetLine(), "Unfinished list of arguments");
                return Token(Token::TK_ERROR);

            case Token::TK_ERROR:
                return Token(Token::TK_ERROR);

            case Token::TK_PUNCTUATION:
            {
                const char c = t.View()[0];
                if (c == '(')
                    ++braceCount;
                else if (braceCount > 0)
                {
                    if (c == ')')
                        --braceCount;
                }
                else if (c == ',' || c == ')')
                {
                    // Trim whitespaces at the end
                    oArg.Str.Truncate(len);

                    // Append "__arg_" to all macro arguments, otherwise if user does:
                    //   #define mad( a, b, c ) fma( a, b, c )
                    //   mad( x.s, y, a );
                    // it will be translated to:
                    //   fma( x.s, y, x.s );
                    // instead of:
                    //   fma( x.s, y, a );
                    if (shouldAppendArg)
                        oArg.Str.AppendCopy(StringView(ARG_SUFFIX, ARG_SUFFIX_LEN));
                    return t;
                }
                break;
            }

            case Token::TK_DIRECTIVE:
                if (iExpand && oArg.Length() && oArg.String()[oArg.Length() - 1] != '\n')
                    oArg.Str.AppendCopy("\n");
                directive = true;
                break;

            case Token::TK_NEWLINE:
                if (iExpand && directive)
                    oArg.Str.Append(std::move(t.Str));
                directive = false;
                continue;

            case Token::TK_LINECONT:
            case Token::TK_COMMENT:
            case Token::TK_LINECOMMENT:
                // ignore these tokens
                continue;

            default:
                break;
            }

            if (!iExpand && t.Type != Token::TK_WHITESPACE)
            {
                Error(Lex.GetLine(), "Unexpected token", &oArg);
                return Token(Token::TK_ERROR);
            }

            oArg.Str.Append(std::move(t.Str));
            if (t.Type != Token::TK_WHITESPACE)
                len = oArg.Length();
        }
    }

    /**
     * Get all the arguments of a macro: '(' arg1 { ',' arg2 { ',' ... }} ')'.
     * Returns the first token after the list; if there is no list directly after
     * the name, oArgs stays empty and the first non-blank token is returned.
     *
     * The lookahead tokens are lexed without expansion: they are put back by the
     * caller and expanded when they are read for real.
     */
    CPreprocessor::Token CPreprocessor::Processor::GetArguments(std::vector<Token>& oArgs, bool iExpand,
                                                               bool shouldAppendArg)
    {
        oArgs.clear();

        Token t = GetToken(false);
        const bool isFirstTokenAnOpenBrace = t.IsPunctuation('(');
        while (Token::IsBlank(t.Type))
            t = GetToken(false);
        if (!isFirstTokenAnOpenBrace)
            return t;

        if (oArgs.capacity() == 0)
            oArgs.reserve(4);

        for (;;)
        {
            if (oArgs.size() == MAX_MACRO_ARGS)
            {
                Error(Lex.GetLine(), "Too many arguments to macro");
                oArgs.clear();
                return Token(Token::TK_ERROR);
            }

            oArgs.emplace_back();
            t = GetArgument(oArgs.back(), iExpand, shouldAppendArg);

            if (t.Type == Token::TK_EOS)
                Error(Lex.GetLine(), "Unfinished list of arguments");
            if (t.Type == Token::TK_EOS || t.Type == Token::TK_ERROR)
            {
                oArgs.clear();
                return Token(Token::TK_ERROR);
            }

            if (t.Type != Token::TK_PUNCTUATION)
                Error(Lex.GetLine(), "Unexpected token", &t);
            else if (t.IsPunctuation(')'))
                return GetToken(false);
            // otherwise we've got a ',' : next argument
        }
    }

    // --- #if expressions --------------------------------------------------

    /**
     * Parse an expression and return the first token that is not part of it.
     * The value of the (sub)expression is left in oResult, as a token: a number,
     * a keyword or macro text that GetValue() evaluates. Operators of priority
     * <= iOpPriority end the expression.
     *
     * Operator priority:
     *  0: Whole expression
     *  1: '(' ')'
     *  2: ||            3: &&          4: |          5: ^         6: &
     *  7: '==' '!='     8: '<' '<=' '>' '>='         9: '<<' '>>'
     * 10: '+' '-'      11: '*' '/' '%'              12: unary '+' '-' '!' '~'
     */
    CPreprocessor::Token CPreprocessor::Processor::GetExpression(Token& oResult, int iLine, int iOpPriority)
    {
        do
            oResult = GetToken(true);
        while (Token::IsBlankOrNewline(oResult.Type));

        Token op(Token::TK_WHITESPACE, "", 0);

        // Handle unary operators here
        if (oResult.Type == Token::TK_PUNCTUATION && oResult.Length() == 1)
        {
            const char uop = oResult.View()[0];
            if (uop == '+' || uop == '-' || uop == '!' || uop == '~')
            {
                op = GetExpression(oResult, iLine, PRIO_UNARY);
                long val;
                if (!GetValue(oResult, val, iLine))
                {
                    Error(iLine, std::string("Unary '") + uop + "' not applicable", &oResult);
                    return Token(Token::TK_ERROR);
                }
                if (uop == '-')
                    oResult.SetValue(-val);
                else if (uop == '!')
                    oResult.SetValue(!val);
                else if (uop == '~')
                    oResult.SetValue(~val);
            }
            else if (uop == '(')
            {
                op = GetExpression(oResult, iLine, PRIO_PARENTHESIS);
                if (op.Type == Token::TK_ERROR)
                    return op;
                if (op.Type == Token::TK_EOS)
                {
                    Error(iLine, "Unclosed parenthesis in #if expression");
                    return Token(Token::TK_ERROR);
                }
                op = GetToken(true); // skip ')'
            }
        }

        while (Token::IsBlankOrNewline(op.Type))
            op = GetToken(true);

        for (;;)
        {
            if (op.Type != Token::TK_PUNCTUATION || op.IsPunctuation(')'))
                return op;

            const BinaryOp binary = ParseBinaryOp(op.View());
            if (binary == BinaryOp::None)
            {
                Error(iLine, "Expecting operator, got", &op);
                return Token(Token::TK_ERROR);
            }

            const int prio = Priority(binary);
            if (iOpPriority >= prio)
                return op;

            Token rop;
            Token nextop = GetExpression(rop, iLine, prio);

            const std::string opText(op.String(), op.Length());
            long vlop, vrop, val;
            if (!GetValue(oResult, vlop, iLine))
            {
                Error(iLine, "Left operand of '" + opText + "' is not a number", &oResult);
                return Token(Token::TK_ERROR);
            }
            if (!GetValue(rop, vrop, iLine))
            {
                Error(iLine, "Right operand of '" + opText + "' is not a number", &rop);
                return Token(Token::TK_ERROR);
            }
            if (!Apply(binary, vlop, vrop, val))
            {
                Error(iLine, "Division by zero");
                return Token(Token::TK_ERROR);
            }
            oResult.SetValue(val);

            op = std::move(nextop);
        }
    }

    bool CPreprocessor::Processor::GetValue(const Token& iToken, long& oValue, int iLine)
    {
        const bool hasText = iToken.Type == Token::TK_KEYWORD || iToken.Type == Token::TK_TEXT ||
                             iToken.Type == Token::TK_NUMBER;
        if (hasText && !iToken.String())
        {
            Error(iLine, "Trying to evaluate an empty expression");
            return false;
        }

        switch (iToken.Type)
        {
        case Token::TK_TEXT:
        {
            // Macro expansion yields text that may be a whole expression
            Processor cpp(MacroList, iToken, iLine);
            Token r;
            Token t = cpp.GetExpression(r, iLine);
            if (t.Type == Token::TK_ERROR)
                return false;
            if (t.Type != Token::TK_EOS)
            {
                Error(iLine, "Garbage after expression", &t);
                return false;
            }
            // An expansion can still be text containing a suppressed keyword.
            // Evaluate it with its rescan metadata instead of treating it as a number.
            return GetValue(r, oValue, iLine);
        }

        case Token::TK_EOS:
        case Token::TK_ERROR:
            return false;

        case Token::TK_KEYWORD:
        {
            // Try to expand the macro
            Macro* m = MacroList.IsDefined(iToken.View());
            if (m && !m->Expanding && !iToken.Str.IsDisabled(m->Name))
            {
                MacroTable::Pin pin(MacroList, *m);
                Token x = ExpandMacro(iToken.Borrow());
                ScopedFlag expanding(m->Expanding);
                return GetValue(x, oValue, iLine);
            }
            // Undefined macro, "expand" to 0 (mimic cpp behaviour)
            oValue = 0;
            return true;
        }

        case Token::TK_NUMBER:
            if (!iToken.GetValue(oValue))
            {
                Error(iLine, "Not a numeric expression", &iToken);
                return false;
            }
            return true;

        default:
            Error(iLine, "Unexpected token", &iToken);
            return false;
        }
    }

    /// Same as GetValue(), but considers the defined() function
    bool CPreprocessor::Processor::GetValueDef(const Token& iToken, long& oValue, int iLine)
    {
        // Temporary add the defined() function to the macro list
        Macro defined;
        defined.Name = StringView("defined");
        defined.IsDefinedFunc = true;
        defined.Args.resize(1);
        MacroList.Define(std::move(defined));

        const bool rc = GetValue(iToken, oValue, iLine);

        // Restore the macro list
        MacroList.PopFront();
        return rc;
    }

    // --- directives -------------------------------------------------------

    /// Collect the remaining part of the directive until EOL (comments and line
    /// continuations removed). oLast receives the token that ended it.
    CPreprocessor::Token CPreprocessor::Processor::GetDirectiveBody(Token& oLast)
    {
        Token t;
        do
        {
            t = GetToken(false);
            if (t.Type == Token::TK_NEWLINE)
            {
                // No directive arguments
                oLast = t.Borrow();
                t.Str.Truncate(0);
                return t;
            }
        } while (Token::IsBlankOrLineCont(t.Type));

        for (;;)
        {
            oLast = GetToken(false);
            switch (oLast.Type)
            {
            case Token::TK_EOS: // can happen and is not an error
            case Token::TK_NEWLINE:
            case Token::TK_ERROR:
                return t;
            case Token::TK_LINECOMMENT:
            case Token::TK_COMMENT:
            case Token::TK_LINECONT:
                continue;
            default:
                break;
            }
            t.Str.Append(oLast.Str);
            t.Type = Token::TK_TEXT;
        }
    }

    CPreprocessor::Token CPreprocessor::Processor::HandleDirective(Token iToken, int iLine)
    {
        // Analyze preprocessor directive
        const char* directive = iToken.String() + 1;
        size_t dirlen = iToken.Length() - 1;
        while (dirlen && isSpace(*directive))
            dirlen--, directive++;

        const int old_line = Lex.GetLine();

        Token last;
        Token t = GetDirectiveBody(last);
        if (last.Type == Token::TK_ERROR)
            return last;

        bool rc = false;
        switch (Classify(StringView(directive, dirlen), Conditions.IsOutputEnabled()))
        {
        case DirectiveKind::Define: rc = HandleDefine(std::move(t), iLine); break;
        case DirectiveKind::UnDef: rc = HandleUnDef(t, iLine); break;
        case DirectiveKind::IfDef: rc = HandleIfDef(t, iLine); break;
        case DirectiveKind::IfNDef:
            rc = HandleIfDef(t, iLine);
            if (rc)
                Conditions.Negate();
            break;
        case DirectiveKind::If: rc = HandleIf(t, iLine); break;
        case DirectiveKind::Elif: rc = HandleElif(t, iLine); break;
        case DirectiveKind::Else: rc = HandleElse(t, iLine); break;
        case DirectiveKind::EndIf: rc = HandleEndIf(t, iLine); break;
        case DirectiveKind::Unknown:
            // Unknown preprocessor directive, roll back and pass through
            Lex.Rewind(iToken.String() + iToken.Length(), old_line);
            iToken.Type = Token::TK_TEXT;
            return iToken;
        }

        return rc ? std::move(last) : Token(Token::TK_ERROR);
    }

    bool CPreprocessor::Processor::WarnGarbage(Lexer& ioLex, int iLine)
    {
        Token t;
        do
            t = ioLex.GetToken();
        while (Token::IsBlank(t.Type));
        if (t.Type != Token::TK_EOS)
            Error(iLine, "Warning: Ignoring garbage after directive", &t);
        return true;
    }

    bool CPreprocessor::Processor::HandleDefine(Token iBody, int iLine)
    {
        // The text of an expansion is short-lived; the macro must not point into it
        if (SupplimentaryExpand && !iBody.Str.IsAllocated())
            iBody.Str = iBody.Str.Clone();

        // Create an additional preprocessor to process the macro body:
        // name, [arguments], value
        Processor cpp(MacroList, iBody, iLine);

        Token t = cpp.GetToken(false);
        if (t.Type != Token::TK_KEYWORD)
        {
            Error(iLine, "Macro name expected after #define");
            return false;
        }

        Macro m;
        m.Name = t.View();
        t = cpp.GetArguments(m.Args, false, true);
        while (t.Type == Token::TK_WHITESPACE)
            t = cpp.GetToken(false);

        switch (t.Type)
        {
        case Token::TK_NEWLINE:
        case Token::TK_EOS:
            t = Token(Token::TK_TEXT, "", 0); // assign "" to token
            break;
        case Token::TK_ERROR:
            return false;
        default:
            // The value is everything up to the end of the body
            t.Type = Token::TK_TEXT;
            assert(t.String() + t.Length() == cpp.Lex.GetPosition());
            t.Str.ResizeBorrowed(size_t(cpp.Lex.GetEnd() - t.String()));
            break;
        }

        if (!m.Args.empty())
        {
            // We need to convert:
            //   #define mad( a__arg_, b__arg_ ) fma( a, b )
            // into:
            //   #define mad( a__arg_, b__arg_ ) fma( a__arg_, b__arg_ )
            MacroTable renames;
            for (const Token& arg : m.Args)
                if (arg.Length() >= ARG_SUFFIX_LEN) // empty for "#define F()"
                    renames.Define(StringView(arg.String(), arg.Length() - ARG_SUFFIX_LEN),
                                   arg.Str.Borrow());

            // Now run the macro value through the supplimentary preprocessor
            Processor cpp2(renames);
            t = cpp2.Parse(t);
        }

        m.Value = std::move(t);
        m.Body = std::move(iBody); // keeps alive what Name/Args/Value point into
        MacroList.Define(std::move(m));
        return true;
    }

    bool CPreprocessor::Processor::HandleUnDef(const Token& iBody, int iLine)
    {
        Processor cpp(MacroList, iBody, iLine);

        Token t = cpp.GetToken(false);
        if (t.Type != Token::TK_KEYWORD)
        {
            Error(iLine, "Expecting a macro name after #undef, got", &t);
            return false;
        }

        // Don't barf if macro does not exist - standard C behaviour
        MacroList.Undef(t.View());
        return WarnGarbage(cpp.Lex, iLine);
    }

    bool CPreprocessor::Processor::HandleIf(bool iValue, int iLine)
    {
        if (Conditions.IsFull())
        {
            Error(iLine, "Too many embedded #if directives");
            return false;
        }
        Conditions.Push(iValue);
        return true;
    }

    bool CPreprocessor::Processor::HandleIfDef(const Token& iBody, int iLine)
    {
        Processor cpp(MacroList, iBody, iLine);

        Token t = cpp.GetToken(false);
        if (t.Type != Token::TK_KEYWORD)
        {
            Error(iLine, "Expecting a macro name after #ifdef, got", &t);
            return false;
        }

        if (!HandleIf(MacroList.IsDefined(t.View()) != nullptr, iLine))
            return false;
        return WarnGarbage(cpp.Lex, iLine);
    }

    bool CPreprocessor::Processor::HandleIf(const Token& iBody, int iLine)
    {
        long val;
        return GetValueDef(iBody, val, iLine) && HandleIf(val != 0, iLine);
    }

    bool CPreprocessor::Processor::HandleElif(const Token& iBody, int iLine)
    {
        if (Conditions.IsEmpty())
        {
            Error(iLine, "#elif without #if");
            return false;
        }

        long val;
        if (!GetValueDef(iBody, val, iLine))
            return false;
        Conditions.Elif(val != 0);
        return true;
    }

    bool CPreprocessor::Processor::HandleElse(const Token& iBody, int iLine)
    {
        if (Conditions.IsEmpty())
        {
            Error(iLine, "#else without #if");
            return false;
        }

        // Negate the result of last #if
        Conditions.Else();
        if (iBody.Length())
            Error(iLine, "Warning: Ignoring garbage after #else", &iBody);
        return true;
    }

    bool CPreprocessor::Processor::HandleEndIf(const Token& iBody, int iLine)
    {
        if (!Conditions.Pop())
        {
            Error(iLine, "#endif without #if");
            return false;
        }
        if (iBody.Length())
            Error(iLine, "Warning: Ignoring garbage after #endif", &iBody);
        return true;
    }

    // -----------------------------------------------------------------------
    // CPreprocessor
    // -----------------------------------------------------------------------

    void CPreprocessor::Define(const char* iMacroName, size_t iMacroNameLen, const char* iMacroValue,
                               size_t iMacroValueLen)
    {
        MacroList.Define(StringView(iMacroName, iMacroNameLen), Text(iMacroValue, iMacroValueLen));
    }

    void CPreprocessor::Define(const char* iMacroName, size_t iMacroNameLen)
    {
        MacroList.Define(StringView(iMacroName, iMacroNameLen), Text());
    }

    bool CPreprocessor::Undef(const char* iMacroName, size_t iMacroNameLen)
    {
        return MacroList.Undef(StringView(iMacroName, iMacroNameLen));
    }

    char* CPreprocessor::Parse(const char* iSource, size_t iLength, size_t& oLength)
    {
        Processor cpp(MacroList);
        Token retval = cpp.Parse(Token(Token::TK_TEXT, iSource, iLength));
        if (retval.Type == Token::TK_ERROR)
            return nullptr;

        oLength = retval.Length();
        return retval.Str.Release();
    }

} // namespace Ogre
