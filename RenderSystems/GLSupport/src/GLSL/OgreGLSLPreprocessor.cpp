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

#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <iterator>
#include <cstring>

namespace Ogre {

    // Limit max number of macro arguments to this
#define MAX_MACRO_ARGS 16

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined( __MINGW32__ )
#define snprintf _snprintf
#endif

    // Character classes with "C" locale semantics. The <ctype.h> functions
    // depend on the global locale and are undefined for negative chars, which
    // is what non-ASCII bytes are on most platforms.
    static inline bool isSpace (char c) { return c == ' ' || (c >= '\t' && c <= '\r'); }
    static inline bool isDigit (char c) { return c >= '0' && c <= '9'; }
    static inline bool isXDigit (char c) { return isDigit (c) || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f'); }
    static inline bool isAlpha (char c) { return (c | 0x20) >= 'a' && (c | 0x20) <= 'z'; }
    /// Can this character be part of a keyword (identifier)?
    static inline bool isKeywordChar (char c) { return c == '_' || isAlpha (c) || isDigit (c); }

    /// Return closest power of two not smaller than given number
    static size_t ClosestPow2 (size_t x)
    {
        if (!(x & (x - 1)))
            return x;
        while (x & (x + 1))
            x |= (x + 1);
        return x + 1;
    }

    /// Growth policy of owned buffers
    static size_t CapacityFor (size_t iSize) { return std::max<size_t> (64, ClosestPow2 (iSize)); }

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

    CPreprocessor::Token CPreprocessor::Macro::Expand(const std::vector<Token>& iArgs,
                                                      MacroTable& iMacros)
    {
        Expanding = true;

        CPreprocessor cpp;
        cpp.SupplimentaryExpand = true;
        std::swap(cpp.MacroList, iMacros);

        // Define a new macro for every argument
        size_t i;
        for (i = 0; i < iArgs.size(); i++)
            cpp.MacroList.Define (Args [i].View (), iArgs [i].Str.Borrow ());
        // The rest arguments are empty
        for (; i < Args.size(); i++)
            cpp.MacroList.Define (Args [i].View (), Text ("", 0));

        Token xt;
        Token new_xt = Token (Token::TK_TEXT, Value.Str.Borrow ());
        bool first = true;
        do {
            xt = std::move (new_xt);
            // Now run the macro expansion through the supplimentary preprocessor
            new_xt = cpp.Parse (xt);

            // The result can borrow an argument or a slice of xt's buffer.
            if (!new_xt.Str.IsAllocated () && new_xt.String () && new_xt.String () != xt.String ())
                new_xt.Str = new_xt.Str.Clone ();

            // Remove the extra macros we have defined, only needed once.
            if (first) {
                first = false;
                for (int j = Args.size() - 1; j >= 0; j--)
                    cpp.MacroList.Undef (Args [j].View ());
            }
            // Repeat until there is no more change between parses
        } while (xt.String () != new_xt.String ());

        Expanding = false;
        std::swap(cpp.MacroList, iMacros);

        if (xt.Length ())
            xt.Str.Disable (Name);
        return xt;
    }

    void CPreprocessor::Error(int iLine, const char *iError, const Token *iToken)
    {
        String msg;
        if (iToken)
            msg = StringUtil::format("line %d: %s: `%.*s'\n", iLine, iError, int(iToken->Length ()), iToken->String ());
        else
            msg = StringUtil::format("line %d: %s\n", iLine, iError);
        LogManager::getSingleton().logMessage(msg, LML_CRITICAL);
    }

    CPreprocessor::CPreprocessor (const Token &iToken, int iLine)
    {
        SourceToken = &iToken;
        Source = iToken.String ();
        SourceEnd = iToken.String () + iToken.Length ();
        Line = iLine;
        BOL = true;
        SupplimentaryExpand = false;
    }

    CPreprocessor::CPreprocessor()
    {
        SourceToken = nullptr;
        Source = 0;
        SourceEnd = 0;
        Line = 0;
        BOL = true;
        SupplimentaryExpand = false;
    }

    CPreprocessor::~CPreprocessor() {}

    CPreprocessor::Token CPreprocessor::GetToken (bool iExpand)
    {
        if (Source >= SourceEnd)
            return Token (Token::TK_EOS);

        const char *begin = Source;
        char c = *Source++;


        if (c == '\n' || (c == '\r' && Peek () == '\n'))
        {
            Line++;
            BOL = true;
            if (c == '\r')
                Source++;
            return Token (Token::TK_NEWLINE, begin, Source - begin);
        }
        else if (isSpace (c))
        {
            while (Source < SourceEnd &&
                   *Source != '\r' &&
                   *Source != '\n' &&
                   isSpace (*Source))
                Source++;

            return Token (Token::TK_WHITESPACE, begin, Source - begin);
        }
        else if (isDigit (c))
        {
            BOL = false;
            if (c == '0' && Peek () == 'x') // hex numbers
            {
                Source++;
                while (Source < SourceEnd && isXDigit (*Source))
                    Source++;
            }
            else
                while (Source < SourceEnd && isDigit (*Source))
                    Source++;
            return Token (Token::TK_NUMBER, begin, Source - begin);
        }
        else if (isKeywordChar (c))
        {
            BOL = false;
            while (Source < SourceEnd && isKeywordChar (*Source))
                Source++;
            Token t (Token::TK_KEYWORD, begin, Source - begin);
            if (SourceToken)
                t.Str.InheritDisabled (SourceToken->Str, begin - SourceToken->String (), t.Length ());
            if (iExpand)
                return ExpandMacro (std::move (t));
            return t;
        }
        else if (c == '"' || c == '\'')
        {
            BOL = false;
            while (Source < SourceEnd && *Source != c)
            {
                if (*Source == '\\')
                {
                    Source++;
                    if (Source >= SourceEnd)
                        break;
                }
                if (*Source == '\n')
                    Line++;
                Source++;
            }
            if (Source < SourceEnd)
                Source++;
            return Token (Token::TK_STRING, begin, Source - begin);
        }
        else if (c == '/' && Peek () == '/')
        {
            BOL = false;
            Source++;
            while (Source < SourceEnd && *Source != '\r' && *Source != '\n')
                Source++;
            return Token (Token::TK_LINECOMMENT, begin, Source - begin);
        }
        else if (c == '/' && Peek () == '*')
        {
            BOL = false;
            Source++;
            while (Source < SourceEnd && (Peek () != '*' || Peek (1) != '/'))
            {
                if (*Source == '\n')
                    Line++;
                Source++;
            }
            if (Source < SourceEnd && *Source == '*')
                Source++;
            if (Source < SourceEnd && *Source == '/')
                Source++;
            return Token (Token::TK_COMMENT, begin, Source - begin);
        }
        else if (c == '#' && BOL)
        {
            // Skip all whitespaces after '#'
            while (Source < SourceEnd && isSpace (*Source))
                Source++;
            while (Source < SourceEnd && !isSpace (*Source))
                Source++;
            return Token (Token::TK_DIRECTIVE, begin, Source - begin);
        }
        else if (c == '\\' && (Peek () == '\r' || Peek () == '\n'))
        {
            // Treat backslash-newline as a whole token
            if (Peek () == '\r')
                Source++;
            if (Peek () == '\n')
                Source++;
            Line++;
            BOL = true;
            return Token (Token::TK_LINECONT, begin, Source - begin);
        }
        else
        {
            BOL = false;
            // Handle double-char operators here
            if (c == '>' && (Peek () == '>' || Peek () == '='))
                Source++;
            else if (c == '<' && (Peek () == '<' || Peek () == '='))
                Source++;
            else if (c == '!' && Peek () == '=')
                Source++;
            else if (c == '=' && Peek () == '=')
                Source++;
            else if ((c == '|' || c == '&' || c == '^') && Peek () == c)
                Source++;
            return Token (Token::TK_PUNCTUATION, begin, Source - begin);
        }
    }


    CPreprocessor::Token CPreprocessor::ExpandMacro (Token iToken)
    {
        Macro *cur = MacroList.IsDefined (iToken.View ());
        if (cur && !cur->Expanding && !iToken.Str.IsDisabled (cur->Name))
        {
            // #undef of this very macro can be run by its arguments or expansion
            MacroTable::Pin pin (MacroList, *cur);

            std::vector<Token> args;
            int old_line = Line;

            if (!cur->Args.empty())
            {
                Token t = GetArguments (args, cur->ExpandFunc ? false : true, false);
                if (t.Type == Token::TK_ERROR)
                {
                    return t;
                }

                // Put the token back into the source pool; we'll handle it later
                if (t.String ())
                {
                    // Returned token should never be allocated on heap
                    assert (!t.Str.IsAllocated ());
                    Source = t.String ();
                    Line -= t.CountNL ();
                }
                // If a macro is defined with arguments but gets not "called" it should behave like normal text
                if (args.size() == 0 && (t.Type != Token::TK_PUNCTUATION || t.String () [0] != '('))
                {
                    return iToken;
                }
            }

            if (args.size() > cur->Args.size())
            {
                std::string msg = "Macro `" + std::string (cur->Name.String, cur->Name.Length) +
                    "' passed " + std::to_string (args.size ()) +
                    " arguments, but takes just " + std::to_string (cur->Args.size ());
                Error (old_line, msg.c_str ());
                return Token (Token::TK_ERROR);
            }

            Token t = cur->ExpandFunc ?
                cur->ExpandFunc (this, args) :
                cur->Expand (args, MacroList);
            t.Str.AppendNL (Line - old_line - t.CountNL ());

            return t;
        }

        return iToken;
    }


    /**
     * Operator priority:
     *  0: Whole expression
     *  1: '(' ')'
     *  2: ||
     *  3: &&
     *  4: |
     *  5: ^
     *  6: &
     *  7: '==' '!='
     *  8: '<' '<=' '>' '>='
     *  9: '<<' '>>'
     * 10: '+' '-'
     * 11: '*' '/' '%'
     * 12: unary '+' '-' '!' '~'
     */
    CPreprocessor::Token CPreprocessor::GetExpression (
        Token &oResult, int iLine, int iOpPriority)
    {
        do
        {
            oResult = GetToken (true);
        } while (Token::IsBlankOrNewline (oResult.Type));

        Token op (Token::TK_WHITESPACE, "", 0);

        // Handle unary operators here
        if (oResult.Type == Token::TK_PUNCTUATION && oResult.Length () == 1)
        {
            // (strchr() would also match the terminating zero)
            if (oResult.String () [0] == '+' || oResult.String () [0] == '-' ||
                oResult.String () [0] == '!' || oResult.String () [0] == '~')
            {
                char uop = oResult.String () [0];
                op = GetExpression (oResult, iLine, 12);
                long val;
                if (!GetValue (oResult, val, iLine))
                {
                    std::string msg = std::string ("Unary '") + uop + "' not applicable";
                    Error (iLine, msg.c_str (), &oResult);
                    return Token (Token::TK_ERROR);
                }

                if (uop == '-')
                    oResult.SetValue (-val);
                else if (uop == '!')
                    oResult.SetValue (!val);
                else if (uop == '~')
                    oResult.SetValue (~val);
            }
            else if (oResult.String () [0] == '(')
            {
                op = GetExpression (oResult, iLine, 1);
                if (op.Type == Token::TK_ERROR)
                    return op;
                if (op.Type == Token::TK_EOS)
                {
                    Error (iLine, "Unclosed parenthesis in #if expression");
                    return Token (Token::TK_ERROR);
                }

                // op should be the closing ')'. Whatever follows a complete
                // expression is skipped, as before; this must not assert,
                // because it is the input that is malformed, not the program.
                op = GetToken (true);
            }
        }

        while (Token::IsBlankOrNewline (op.Type))
            op = GetToken (true);

        while (true)
        {
            if (op.Type != Token::TK_PUNCTUATION)
                return op;

            int prio = 0;
            if (op.Length () == 1)
                switch (op.String () [0])
                {
                case ')': return op;
                case '|': prio = 4; break;
                case '^': prio = 5; break;
                case '&': prio = 6; break;
                case '<':
                case '>': prio = 8; break;
                case '+':
                case '-': prio = 10; break;
                case '*':
                case '/':
                case '%': prio = 11; break;
                }
            else if (op.Length () == 2)
                switch (op.String () [0])
                {
                case '|': if (op.String () [1] == '|') prio = 2; break;
                case '&': if (op.String () [1] == '&') prio = 3; break;
                case '=': if (op.String () [1] == '=') prio = 7; break;
                case '!': if (op.String () [1] == '=') prio = 7; break;
                case '<':
                    if (op.String () [1] == '=')
                        prio = 8;
                    else if (op.String () [1] == '<')
                        prio = 9;
                    break;
                case '>':
                    if (op.String () [1] == '=')
                        prio = 8;
                    else if (op.String () [1] == '>')
                        prio = 9;
                    break;
                }

            if (!prio)
            {
                Error (iLine, "Expecting operator, got", &op);
                return Token (Token::TK_ERROR);
            }

            if (iOpPriority >= prio)
                return op;

            Token rop;
            Token nextop = GetExpression (rop, iLine, prio);
            long vlop, vrop;
            if (!GetValue (oResult, vlop, iLine))
            {
                std::string msg = "Left operand of '" + std::string (op.String (), op.Length ()) +
                    "' is not a number";
                Error (iLine, msg.c_str (), &oResult);
                return Token (Token::TK_ERROR);
            }
            if (!GetValue (rop, vrop, iLine))
            {
                std::string msg = "Right operand of '" + std::string (op.String (), op.Length ()) +
                    "' is not a number";
                Error (iLine, msg.c_str (), &rop);
                return Token (Token::TK_ERROR);
            }

            switch (op.String () [0])
            {
            case '|':
                if (prio == 2)
                    oResult.SetValue (vlop || vrop);
                else
                    oResult.SetValue (vlop | vrop);
                break;
            case '&':
                if (prio == 3)
                    oResult.SetValue (vlop && vrop);
                else
                    oResult.SetValue (vlop & vrop);
                break;
            case '<':
                if (op.Length () == 1)
                    oResult.SetValue (vlop < vrop);
                else if (prio == 8)
                    oResult.SetValue (vlop <= vrop);
                else if (prio == 9)
                    oResult.SetValue (vlop << (vrop & 63));
                break;
            case '>':
                if (op.Length () == 1)
                    oResult.SetValue (vlop > vrop);
                else if (prio == 8)
                    oResult.SetValue (vlop >= vrop);
                else if (prio == 9)
                    oResult.SetValue (vlop >> (vrop & 63));
                break;
            case '^': oResult.SetValue (vlop ^ vrop); break;
            case '!': oResult.SetValue (vlop != vrop); break;
            case '=': oResult.SetValue (vlop == vrop); break;
            case '+': oResult.SetValue (vlop + vrop); break;
            case '-': oResult.SetValue (vlop - vrop); break;
            case '*': oResult.SetValue (vlop * vrop); break;
            case '/':
            case '%':
                if (vrop == 0)
                {
                    Error (iLine, "Division by zero");
                    return Token (Token::TK_ERROR);
                }
                if (op.String () [0] == '/')
                    oResult.SetValue (vlop / vrop);
                else
                    oResult.SetValue (vlop % vrop);
                break;
            }

            op = std::move (nextop);
        }
    }


    bool CPreprocessor::GetValue (const Token &iToken, long &oValue, int iLine)
    {
        Token r;
        const Token *vt = &iToken;

        if ((vt->Type == Token::TK_KEYWORD ||
             vt->Type == Token::TK_TEXT ||
             vt->Type == Token::TK_NUMBER) &&
            !vt->String ())
        {
            Error (iLine, "Trying to evaluate an empty expression");
            return false;
        }

        if (vt->Type == Token::TK_TEXT)
        {
            CPreprocessor cpp (iToken, iLine);
            std::swap(cpp.MacroList, MacroList);

            Token t;
            t = cpp.GetExpression (r, iLine);

            std::swap(cpp.MacroList, MacroList);

            if (t.Type == Token::TK_ERROR)
                return false;

            if (t.Type != Token::TK_EOS)
            {
                Error (iLine, "Garbage after expression", &t);
                return false;
            }

            // An expansion can still be text containing a suppressed identifier.
            // Evaluate it with its rescan metadata instead of treating it as a number.
            return GetValue (r, oValue, iLine);
        }

        Macro *m;
        switch (vt->Type)
        {
        case Token::TK_EOS:
        case Token::TK_ERROR:
            return false;

        case Token::TK_KEYWORD:
            // Try to expand the macro
            if ((m = MacroList.IsDefined (vt->View ())) && !m->Expanding && !vt->Str.IsDisabled (m->Name))
            {
                MacroTable::Pin pin (MacroList, *m);
                Token x = ExpandMacro (vt->Borrow ());
                m->Expanding = true;
                bool rc = GetValue (x, oValue, iLine);
                m->Expanding = false;
                return rc;
            }

            // Undefined macro, "expand" to 0 (mimic cpp behaviour)
            oValue = 0;
            break;

        case Token::TK_TEXT:
        case Token::TK_NUMBER:
            if (!vt->GetValue (oValue))
            {
                Error (iLine, "Not a numeric expression", vt);
                return false;
            }
            break;

        default:
            Error (iLine, "Unexpected token", vt);
            return false;
        }

        return true;
    }


    CPreprocessor::Token CPreprocessor::GetArgument (Token &oArg, bool iExpand,
                                                     bool shouldAppendArg)
    {
        do
        {
            oArg = GetToken (iExpand);
        } while (Token::IsBlankOrNewline (oArg.Type));

        if (!iExpand)
        {
            if (oArg.Type == Token::TK_EOS)
                return Token (Token::TK_EOS);
            else if (oArg.Type == Token::TK_PUNCTUATION &&
                     (oArg.String () [0] == ',' ||
                      oArg.String () [0] == ')'))
            {
                Token t = std::move (oArg);
                oArg = Token (Token::TK_TEXT, "", 0);
                return t;
            }
            else if (oArg.Type != Token::TK_KEYWORD)
            {
                Error (Line, "Unexpected token", &oArg);
                return Token (Token::TK_ERROR);
            }
        }

        size_t braceCount = 0;
        bool directive = oArg.Type == Token::TK_DIRECTIVE;

        if( oArg.Type == Token::TK_PUNCTUATION && oArg.String ()[0] == '(' )
            ++braceCount;

        size_t len = oArg.Length ();
        while (true)
        {
            // Rescan directives later, before expanding their operands.
            Token t = GetToken (iExpand && !directive);
            switch (t.Type)
            {
            case Token::TK_EOS:
                Error (Line, "Unfinished list of arguments");
                OGRE_FALLTHROUGH;
            case Token::TK_ERROR:
                return Token (Token::TK_ERROR);
            case Token::TK_PUNCTUATION:
                if( t.String () [0] == '(' )
                {
                    ++braceCount;
                }
                else if( !braceCount )
                {
                    if (t.String () [0] == ',' ||
                        t.String () [0] == ')')
                    {
                        // Trim whitespaces at the end
                        oArg.Str.Truncate (len);

                        //Append "__arg_" to all macro arguments, otherwise if user does:
                        //  #define mad( a, b, c ) fma( a, b, c )
                        //  mad( x.s, y, a );
                        //It will be translated to:
                        //  fma( x.s, y, x.s );
                        //instead of:
                        //  fma( x.s, y, a );
                        //This does not fix the problem by the root, but
                        //typing "__arg_" by the user is extremely rare.
                        if( shouldAppendArg )
                            oArg.Str.AppendCopy ("__arg_");
                        return t;
                    }
                }
                else
                {
                    if( t.String () [0] == ')' )
                        --braceCount;
                }
                break;
            case Token::TK_DIRECTIVE:
                if (iExpand && oArg.Length () && oArg.String ()[oArg.Length () - 1] != '\n')
                    oArg.Str.AppendCopy ("\n");
                directive = true;
                break;
            case Token::TK_NEWLINE:
                if (iExpand && directive)
                    oArg.Str.Append (std::move (t.Str));
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
                Error (Line, "Unexpected token", &oArg);
                return Token (Token::TK_ERROR);
            }

            oArg.Str.Append (std::move (t.Str));

            if (t.Type != Token::TK_WHITESPACE)
                len = oArg.Length ();
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
    CPreprocessor::Token CPreprocessor::GetArguments (std::vector<Token>& oArgs,
                                                      bool iExpand, bool shouldAppendArg)
    {
        oArgs.clear();

        Token t = GetToken (false);
        const bool isFirstTokenAnOpenBrace = t.IsPunctuation ('(');
        while (Token::IsBlank (t.Type))
            t = GetToken (false);
        if (!isFirstTokenAnOpenBrace)
            return t;

        if (oArgs.capacity () == 0)
            oArgs.reserve (4);

        for (;;)
        {
            if (oArgs.size () == MAX_MACRO_ARGS)
            {
                Error (Line, "Too many arguments to macro");
                oArgs.clear ();
                return Token (Token::TK_ERROR);
            }

            oArgs.emplace_back ();
            t = GetArgument (oArgs.back (), iExpand, shouldAppendArg);

            if (t.Type == Token::TK_EOS)
                Error (Line, "Unfinished list of arguments");
            if (t.Type == Token::TK_EOS || t.Type == Token::TK_ERROR)
            {
                oArgs.clear ();
                return Token (Token::TK_ERROR);
            }

            if (t.Type != Token::TK_PUNCTUATION)
                Error (Line, "Unexpected token", &t);
            else if (t.IsPunctuation (')'))
                return GetToken (false);
            // otherwise we've got a ',' : next argument
        }
    }


    bool CPreprocessor::HandleDefine (Token &iBody, int iLine)
    {
        // The text of a macro expansion is short-lived: a macro defined in it
        // must not point into it
        if (SupplimentaryExpand && !iBody.Str.IsAllocated ())
            iBody.Str = iBody.Str.Clone ();

        // Create an additional preprocessor to process macro body
        CPreprocessor cpp (iBody, iLine);

        Token t = cpp.GetToken (false);
        if (t.Type != Token::TK_KEYWORD)
        {
            Error (iLine, "Macro name expected after #define");
            return false;
        }

        Macro m;
        m.Name = t.View ();
        t = cpp.GetArguments (m.Args, false, true);
        while (t.Type == Token::TK_WHITESPACE)
            t = cpp.GetToken (false);

        switch (t.Type)
        {
        case Token::TK_NEWLINE:
        case Token::TK_EOS:
            // Assign "" to token
            t = Token (Token::TK_TEXT, "", 0);
            break;

        case Token::TK_ERROR:
            return false;

        default:
            t.Type = Token::TK_TEXT;
            assert (t.String () + t.Length () == cpp.Source);
            t.Str.ResizeBorrowed (cpp.SourceEnd - t.String ());
            break;
        }

        if( !m.Args.empty() )
        {
            CPreprocessor cpp2;

            //We need to convert:
            //  #define mad( a__arg_, b__arg_, c__arg_ ) fma( a, b, c )
            //into:
            //  #define mad( a__arg_, b__arg_, c__arg_ ) fma( a__arg_, b__arg_, c__arg_ )
            for (const Token& arg : m.Args)
            {
                if (arg.Length () >= 6) // empty for "#define F()"
                    cpp2.Define(arg.String (), arg.Length () - 6, arg.String (), arg.Length ());
            }

            // Now run the macro expansion through the supplimentary preprocessor
            Token xt = cpp2.Parse( t );
            t = std::move (xt);
        }

        m.Value = std::move (t);
        // The macro keeps the text that its name, arguments and value point into
        m.Body = std::move (iBody);
        MacroList.Define (std::move (m));
        return true;
    }


    bool CPreprocessor::HandleUnDef (Token &iBody, int iLine)
    {
        CPreprocessor cpp (iBody, iLine);

        Token t = cpp.GetToken (false);

        if (t.Type != Token::TK_KEYWORD)
        {
            Error (iLine, "Expecting a macro name after #undef, got", &t);
            return false;
        }

        // Don't barf if macro does not exist - standard C behaviour
        Undef (t.String (), t.Length ());

        do
        {
            t = cpp.GetToken (false);
        } while (Token::IsBlank (t.Type));

        if (t.Type != Token::TK_EOS)
            Error (iLine, "Warning: Ignoring garbage after directive", &t);

        return true;
    }

    bool CPreprocessor::HandleIf(bool val, int iLine)
    {
        if (Conditions.IsFull())
        {
            Error (iLine, "Too many embedded #if directives");
            return false;
        }

        Conditions.Push(val);

        return true;
    }

    bool CPreprocessor::HandleIfDef (Token &iBody, int iLine)
    {
        CPreprocessor cpp (iBody, iLine);

        Token t = cpp.GetToken (false);

        if (t.Type != Token::TK_KEYWORD)
        {
            Error (iLine, "Expecting a macro name after #ifdef, got", &t);
            return false;
        }

        if (!HandleIf(MacroList.IsDefined (t.View ()) != NULL, iLine))
            return false;

        do
        {
            t = cpp.GetToken (false);
        } while (Token::IsBlank (t.Type));

        if (t.Type != Token::TK_EOS)
            Error (iLine, "Warning: Ignoring garbage after directive", &t);

        return true;
    }


    CPreprocessor::Token CPreprocessor::ExpandDefined (CPreprocessor *iParent, const std::vector<Token>& iArgs)
    {
        if (iArgs.size() != 1)
        {
            iParent->Error (iParent->Line, "The defined() function takes exactly one argument");
            return Token (Token::TK_ERROR);
        }

        const char *v = iParent->MacroList.IsDefined (iArgs [0].View ()) ? "1" : "0";
        return Token (Token::TK_NUMBER, v, 1);
    }

    bool CPreprocessor::GetValueDef(const Token &iToken, long &oValue, int iLine)
    {
        // Temporary add the defined() function to the macro list
        Macro defined;
        defined.Name = StringView ("defined");
        defined.ExpandFunc = ExpandDefined;
        defined.Args.resize(1);
        MacroList.Define (std::move (defined));

        bool rc = GetValue (iToken, oValue, iLine);

        // Restore the macro list
        MacroList.PopFront ();

        return rc;
    }

    bool CPreprocessor::HandleIf (Token &iBody, int iLine)
    {
        long val;
        return GetValueDef(iBody, val, iLine) && HandleIf(val, iLine);
    }

    bool CPreprocessor::HandleElif (Token &iBody, int iLine)
    {
        if (Conditions.IsEmpty())
        {
            Error (iLine, "#elif without #if");
            return false;
        }

        long val;
        if (!GetValueDef(iBody, val, iLine))
            return false;

        Conditions.Elif(val != 0);

        return true;
    }


    bool CPreprocessor::HandleElse (Token &iBody, int iLine)
    {
        if (Conditions.IsEmpty())
        {
            Error (iLine, "#else without #if");
            return false;
        }

        // Negate the result of last #if
        Conditions.Else();

        if (iBody.Length ())
            Error (iLine, "Warning: Ignoring garbage after #else", &iBody);

        return true;
    }


    bool CPreprocessor::HandleEndIf (Token &iBody, int iLine)
    {
        if (!Conditions.Pop())
        {
            Error (iLine, "#endif without #if");
            return false;
        }

        if (iBody.Length ())
            Error (iLine, "Warning: Ignoring garbage after #endif", &iBody);

        return true;
    }


    /// Collect the remaining part of the directive until EOL (comments and line
    /// continuations removed). oLast receives the token that ended it.
    CPreprocessor::Token CPreprocessor::GetDirectiveBody (Token& oLast)
    {
        Token t;
        do
        {
            t = GetToken (false);
            if (t.Type == Token::TK_NEWLINE)
            {
                // No directive arguments
                oLast = t.Borrow ();
                t.Str.Truncate (0);
                return t;
            }
        } while (Token::IsBlankOrLineCont (t.Type));

        for (;;)
        {
            oLast = GetToken (false);
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
            t.Str.Append (oLast.Str);
            t.Type = Token::TK_TEXT;
        }
    }


    CPreprocessor::Token CPreprocessor::HandleDirective (Token &iToken, int iLine)
    {
        // Analyze preprocessor directive
        const char *directive = iToken.String () + 1;
        size_t dirlen = iToken.Length () - 1;
        while (dirlen && isSpace (*directive))
            dirlen--, directive++;

        int old_line = Line;

        Token last;
        Token t = GetDirectiveBody (last);
        if (last.Type == Token::TK_ERROR)
            return last;

#define IS_DIRECTIVE(s)                                                 \
        (dirlen == strlen(s) && (strncmp (directive, s, dirlen) == 0))

        bool outputEnabled = Conditions.IsOutputEnabled();
        bool rc;

        if (IS_DIRECTIVE ("define") && outputEnabled)
            rc = HandleDefine (t, iLine);
        else if (IS_DIRECTIVE ("undef") && outputEnabled)
            rc = HandleUnDef (t, iLine);
        else if (IS_DIRECTIVE ("ifdef"))
            rc = HandleIfDef (t, iLine);
        else if (IS_DIRECTIVE ("ifndef"))
        {
            rc = HandleIfDef (t, iLine);
            if (rc)
                Conditions.Negate();
        }
        else if (IS_DIRECTIVE ("if"))
            rc = HandleIf (t, iLine);
        else if (IS_DIRECTIVE ("elif"))
            rc = HandleElif (t, iLine);

        else if (IS_DIRECTIVE ("else"))
            rc = HandleElse (t, iLine);
        else if (IS_DIRECTIVE ("endif"))
            rc = HandleEndIf (t, iLine);
        else
        {
            //Error (iLine, "Unknown preprocessor directive", &iToken);
            //return Token (Token::TK_ERROR);

            // Unknown preprocessor directive, roll back and pass through
            Line = old_line;
            Source = iToken.String () + iToken.Length ();
            iToken.Type = Token::TK_TEXT;
            return std::move (iToken);
        }

#undef IS_DIRECTIVE

        if (!rc)
            return Token (Token::TK_ERROR);
        return last;
    }


    void CPreprocessor::Define (const char *iMacroName, size_t iMacroNameLen,
                                const char *iMacroValue, size_t iMacroValueLen)
    {
        MacroList.Define (StringView (iMacroName, iMacroNameLen), Text (iMacroValue, iMacroValueLen));
    }


    void CPreprocessor::Define (const char *iMacroName, size_t iMacroNameLen)
    {
        MacroList.Define (StringView (iMacroName, iMacroNameLen), Text ());
    }


    bool CPreprocessor::Undef (const char *iMacroName, size_t iMacroNameLen)
    {
        return MacroList.Undef (StringView (iMacroName, iMacroNameLen));
    }


    CPreprocessor::Token CPreprocessor::Parse (const Token &iSource)
    {
        SourceToken = &iSource;
        Source = iSource.String ();
        SourceEnd = Source + iSource.Length ();
        Line = 1;
        BOL = true;
        Conditions = ConditionalStack();

        // Accumulate output into this token
        Token output (Token::TK_TEXT);
        int empty_lines = 0;

        // Enable output only if all embedded #if's were true
        bool old_output_enabled = true;
        bool output_enabled = true;
        int output_disabled_line = 0;

        while (Source < SourceEnd)
        {
            int old_line = Line;
            Token t = GetToken (true);

            if (t.Type == Token::TK_DIRECTIVE)
            {
                // Handle preprocessor directives
                t = HandleDirective (t, old_line);

                output_enabled = Conditions.IsOutputEnabled();
                if (output_enabled != old_output_enabled)
                {
                    if (output_enabled)
                        output.Str.AppendNL (old_line - output_disabled_line);
                    else
                        output_disabled_line = old_line;
                    old_output_enabled = output_enabled;
                }

                if (output_enabled)
                    output.Str.AppendNL (Line - old_line - t.CountNL ());
            }

            switch (t.Type)
            {
            case Token::TK_ERROR:
                return t;

            case Token::TK_EOS:
                return output; // Force termination

            case Token::TK_COMMENT:
                // C comments are replaced with single spaces.
                if (output_enabled)
                {
                    output.Str.AppendCopy (" ");
                    output.Str.AppendNL (Line - old_line);
                }
                break;

            case Token::TK_LINECOMMENT:
                // C++ comments are ignored
                break;

            case Token::TK_LINECONT:
                // Backslash-Newline sequences are deleted, no matter where.
                empty_lines++;
                break;

            case Token::TK_PUNCTUATION:
                if (output_enabled && (!SupplimentaryExpand || t.String ()[0] != '#'))
                    output.Str.Append (std::move (t.Str));
                break;

            default:
                if (t.Type == Token::TK_NEWLINE && empty_lines)
                {
                    // Compensate for the backslash-newline combinations
                    // we have encountered, otherwise line numeration is broken
                    if (output_enabled)
                        output.Str.AppendNL (empty_lines);
                    empty_lines = 0;
                }
                // Passthrough all other tokens
                if (output_enabled)
                    output.Str.Append (std::move (t.Str));
                break;
            }
        }

        if (!Conditions.IsEmpty())
        {
            Error (Line, "Unclosed #if at end of source");
            return Token (Token::TK_ERROR);
        }

        return output;
    }


    char *CPreprocessor::Parse (const char *iSource, size_t iLength, size_t &oLength)
    {
        Token retval = Parse (Token (Token::TK_TEXT, iSource, iLength));
        if (retval.Type == Token::TK_ERROR)
            return NULL;

        oLength = retval.Length ();
        return retval.Str.Release ();
    }

} // namespace Ogre
