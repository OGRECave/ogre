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

#include <ctype.h>
#include <stdio.h>
#include <assert.h>

namespace Ogre {

    // Limit max number of macro arguments to this
#define MAX_MACRO_ARGS 16

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined( __MINGW32__ )
#define snprintf _snprintf
#endif

    /// Return closest power of two not smaller than given number
    static size_t ClosestPow2 (size_t x)
    {
        if (!(x & (x - 1)))
            return x;
        while (x & (x + 1))
            x |= (x + 1);
        return x + 1;
    }

    void CPreprocessor::Token::Append (const char *iString, size_t iLength)
    {
        Token t (Token::TK_TEXT, iString, iLength);
        Append (t);
    }

    void CPreprocessor::Token::Append (const Token &iOther)
    {
        if (!iOther.String)
            return;

        if (!String)
        {
            String = iOther.String;
            Length = iOther.Length;
            Allocated = iOther.Allocated;
            iOther.Allocated = 0; // !!! not quite correct but effective
            return;
        }

        if (Allocated)
        {
            size_t new_alloc = ClosestPow2 (Length + iOther.Length);
            if (new_alloc < 64)
                new_alloc = 64;
            if (new_alloc != Allocated)
            {
                Allocated = new_alloc;
                Buffer = (char *)realloc (Buffer, Allocated);
            }
        }
        else if (String + Length != iOther.String)
        {
            Allocated = ClosestPow2 (Length + iOther.Length);
            if (Allocated < 64)
                Allocated = 64;
            char *newstr = (char *)malloc (Allocated);
            memcpy (newstr, String, Length);
            Buffer = newstr;
        }

        if (Allocated)
            memcpy (Buffer + Length, iOther.String, iOther.Length);
        Length += iOther.Length;
    }

    bool CPreprocessor::Token::GetValue (long &oValue) const
    {
        long val = 0;
        size_t i = 0;

        while (isspace (String [i]))
            i++;

        long base = 10;
        if (String [i] == '0')
        {
            if (Length > i + 1 && String [i + 1] == 'x')
                base = 16, i += 2;
            else
                base = 8;
        }

        for (; i < Length; i++)
        {
            int c = int (String [i]);
            if (isspace (c))
                // Possible end of number
                break;

            if (c >= 'a' && c <= 'z')
                c -= ('a' - 'A');

            c -= '0';
            if (c < 0)
                return false;

            if (c > 9)
                c -= ('A' - '9' - 1);

            if (c >= base)
                return false;

            val = (val * base) + c;
        }

        // Check that all other characters are just spaces
        for (; i < Length; i++)
            if (!isspace (String [i]))
                return false;

        oValue = val;
        return true;
    }


    void CPreprocessor::Token::SetValue (long iValue)
    {
        static char tmp [21];
        int len = snprintf (tmp, sizeof (tmp), "%ld", iValue);
        Length = 0;
        Append (tmp, len);
        Type = TK_NUMBER;
    }


    void CPreprocessor::Token::AppendNL (int iCount)
    {
        static const char newlines [8] =
            { '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n' };

        while (iCount > 8)
        {
            Append (newlines, 8);
            iCount -= 8;
        }
        if (iCount > 0)
            Append (newlines, iCount);
    }


    int CPreprocessor::Token::CountNL ()
    {
        if (Type == TK_EOS || Type == TK_ERROR)
            return 0;

        const char *s = String;
        size_t l = Length;
        int c = 0;
        while (l > 0)
        {
            const char *n = (const char *)memchr (s, '\n', l);
            if (!n)
                return c;
            c++;
            l -= (n - s + 1);
            s = n + 1;
        }
        return c;
    }

    CPreprocessor::Token CPreprocessor::Macro::Expand(const std::vector<Token>& iArgs,
                                                      std::forward_list<Macro>& iMacros)
    {
        Expanding = true;

        CPreprocessor cpp;
        cpp.SupplimentaryExpand = true;
        std::swap(cpp.MacroList, iMacros);

        // Define a new macro for every argument
        size_t i;
        for (i = 0; i < iArgs.size(); i++)
            cpp.Define (Args [i].String, Args [i].Length,
                        iArgs [i].String, iArgs [i].Length);
        // The rest arguments are empty
        for (; i < Args.size(); i++)
            cpp.Define (Args [i].String, Args [i].Length, "", 0);

        Token xt;
        // make sure that no one down the line sets Value.Allocated = 0
        Token new_xt = Token(Token::TK_TEXT, Value.String, Value.Length);
        bool first = true;
        do {
            xt = new_xt;
            // Now run the macro expansion through the supplimentary preprocessor
            new_xt = cpp.Parse (xt);

            // Remove the extra macros we have defined, only needed once.
            if (first) {
                first = false;
                for (int j = Args.size() - 1; j >= 0; j--)
                    cpp.Undef (Args [j].String, Args [j].Length);
            }
            // Repeat until there is no more change between parses
        } while (xt.String != new_xt.String);

        Expanding = false;
        std::swap(cpp.MacroList, iMacros);

        return xt;
    }

    void CPreprocessor::Error(int iLine, const char *iError, const Token *iToken)
    {
        String msg;
        if (iToken)
            msg = StringUtil::format("line %d: %s: `%.*s'\n", iLine, iError, int(iToken->Length), iToken->String);
        else
            msg = StringUtil::format("line %d: %s\n", iLine, iError);
        LogManager::getSingleton().logMessage(msg, LML_CRITICAL);
    }

    CPreprocessor::CPreprocessor (const Token &iToken, int iLine)
    {
        Source = iToken.String;
        SourceEnd = iToken.String + iToken.Length;
        EnableOutput = 1;
        EnableElif = 0;
        Line = iLine;
        BOL = true;
        SupplimentaryExpand = false;
    }

    CPreprocessor::CPreprocessor()
    {
        Source = 0;
        SourceEnd = 0;
        EnableOutput = 1;
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


        if (c == '\n' || (c == '\r' && *Source == '\n'))
        {
            Line++;
            BOL = true;
            if (c == '\r')
                Source++;
            return Token (Token::TK_NEWLINE, begin, Source - begin);
        }
        else if (isspace (c))
        {
            while (Source < SourceEnd &&
                   *Source != '\r' &&
                   *Source != '\n' &&
                   isspace (*Source))
                Source++;

            return Token (Token::TK_WHITESPACE, begin, Source - begin);
        }
        else if (isdigit (c))
        {
            BOL = false;
            if (c == '0' && Source < SourceEnd && Source [0] == 'x') // hex numbers
            {
                Source++;
                while (Source < SourceEnd && isxdigit (*Source))
                    Source++;
            }
            else
                while (Source < SourceEnd && isdigit (*Source))
                    Source++;
            return Token (Token::TK_NUMBER, begin, Source - begin);
        }
        else if (c == '_' || isalnum (c))
        {
            BOL = false;
            while (Source < SourceEnd && (*Source == '_' || isalnum (*Source)))
                Source++;
            Token t (Token::TK_KEYWORD, begin, Source - begin);
            if (iExpand)
                t = ExpandMacro (t);
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
        else if (c == '/' && *Source == '/')
        {
            BOL = false;
            Source++;
            while (Source < SourceEnd && *Source != '\r' && *Source != '\n')
                Source++;
            return Token (Token::TK_LINECOMMENT, begin, Source - begin);
        }
        else if (c == '/' && *Source == '*')
        {
            BOL = false;
            Source++;
            while (Source < SourceEnd && (Source [0] != '*' || Source [1] != '/'))
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
            while (Source < SourceEnd && isspace (*Source))
                Source++;
            while (Source < SourceEnd && !isspace (*Source))
                Source++;
            return Token (Token::TK_DIRECTIVE, begin, Source - begin);
        }
        else if (c == '\\' && Source < SourceEnd && (*Source == '\r' || *Source == '\n'))
        {
            // Treat backslash-newline as a whole token
            if (*Source == '\r')
                Source++;
            if (*Source == '\n')
                Source++;
            Line++;
            BOL = true;
            return Token (Token::TK_LINECONT, begin, Source - begin);
        }
        else
        {
            BOL = false;
            // Handle double-char operators here
            if (c == '>' && (*Source == '>' || *Source == '='))
                Source++;
            else if (c == '<' && (*Source == '<' || *Source == '='))
                Source++;
            else if (c == '!' && *Source == '=')
                Source++;
            else if (c == '=' && *Source == '=')
                Source++;
            else if ((c == '|' || c == '&' || c == '^') && *Source == c)
                Source++;
            return Token (Token::TK_PUNCTUATION, begin, Source - begin);
        }
    }


    CPreprocessor::Macro *CPreprocessor::IsDefined (const Token &iToken)
    {
        for (Macro& cur : MacroList)
            if (cur.Name == iToken)
                return &cur;

        return NULL;
    }


    CPreprocessor::Token CPreprocessor::ExpandMacro (const Token &iToken)
    {
        Macro *cur = IsDefined (iToken);
        if (cur && !cur->Expanding)
        {
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
                if (t.String)
                {
                    // Returned token should never be allocated on heap
                    assert (t.Allocated == 0);
                    Source = t.String;
                    Line -= t.CountNL ();
                }
                // If a macro is defined with arguments but gets not "called" it should behave like normal text
                if (args.size() == 0 && (t.Type != Token::TK_PUNCTUATION || t.String [0] != '('))
                {
                    return iToken;
                }
            }

            if (args.size() > cur->Args.size())
            {
                char tmp [60];
                snprintf (tmp, sizeof (tmp), "Macro `%.*s' passed %zu arguments, but takes just %zu",
                          int (cur->Name.Length), cur->Name.String,
                          args.size(), cur->Args.size());
                Error (old_line, tmp);
                return Token (Token::TK_ERROR);
            }

            Token t = cur->ExpandFunc ?
                cur->ExpandFunc (this, args) :
                cur->Expand (args, MacroList);
            t.AppendNL (Line - old_line);

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
        char tmp [40];

        do
        {
            oResult = GetToken (true);
        } while (oResult.Type == Token::TK_WHITESPACE ||
                 oResult.Type == Token::TK_NEWLINE ||
                 oResult.Type == Token::TK_COMMENT ||
                 oResult.Type == Token::TK_LINECOMMENT ||
                 oResult.Type == Token::TK_LINECONT);

        Token op (Token::TK_WHITESPACE, "", 0);

        // Handle unary operators here
        if (oResult.Type == Token::TK_PUNCTUATION && oResult.Length == 1)
        {
            if (strchr ("+-!~", oResult.String [0]))
            {
                char uop = oResult.String [0];
                op = GetExpression (oResult, iLine, 12);
                long val;
                if (!GetValue (oResult, val, iLine))
                {
                    snprintf (tmp, sizeof (tmp), "Unary '%c' not applicable", uop);
                    Error (iLine, tmp, &oResult);
                    return Token (Token::TK_ERROR);
                }

                if (uop == '-')
                    oResult.SetValue (-val);
                else if (uop == '!')
                    oResult.SetValue (!val);
                else if (uop == '~')
                    oResult.SetValue (~val);
            }
            else if (oResult.String [0] == '(')
            {
                op = GetExpression (oResult, iLine, 1);
                if (op.Type == Token::TK_ERROR)
                    return op;
                if (op.Type == Token::TK_EOS)
                {
                    Error (iLine, "Unclosed parenthesis in #if expression");
                    return Token (Token::TK_ERROR);
                }

                assert (op.Type == Token::TK_PUNCTUATION &&
                        op.Length == 1 &&
                        op.String [0] == ')');
                op = GetToken (true);
            }
        }

        while (op.Type == Token::TK_WHITESPACE ||
               op.Type == Token::TK_NEWLINE ||
               op.Type == Token::TK_COMMENT ||
               op.Type == Token::TK_LINECOMMENT ||
               op.Type == Token::TK_LINECONT)
            op = GetToken (true);

        while (true)
        {
            if (op.Type != Token::TK_PUNCTUATION)
                return op;

            int prio = 0;
            if (op.Length == 1)
                switch (op.String [0])
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
            else if (op.Length == 2)
                switch (op.String [0])
                {
                case '|': if (op.String [1] == '|') prio = 2; break;
                case '&': if (op.String [1] == '&') prio = 3; break;
                case '=': if (op.String [1] == '=') prio = 7; break;
                case '!': if (op.String [1] == '=') prio = 7; break;
                case '<':
                    if (op.String [1] == '=')
                        prio = 8;
                    else if (op.String [1] == '<')
                        prio = 9;
                    break;
                case '>':
                    if (op.String [1] == '=')
                        prio = 8;
                    else if (op.String [1] == '>')
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
                snprintf (tmp, sizeof (tmp), "Left operand of '%.*s' is not a number",
                          int (op.Length), op.String);
                Error (iLine, tmp, &oResult);
                return Token (Token::TK_ERROR);
            }
            if (!GetValue (rop, vrop, iLine))
            {
                snprintf (tmp, sizeof (tmp), "Right operand of '%.*s' is not a number",
                          int (op.Length), op.String);
                Error (iLine, tmp, &rop);
                return Token (Token::TK_ERROR);
            }

            switch (op.String [0])
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
                if (op.Length == 1)
                    oResult.SetValue (vlop < vrop);
                else if (prio == 8)
                    oResult.SetValue (vlop <= vrop);
                else if (prio == 9)
                    oResult.SetValue (vlop << vrop);
                break;
            case '>':
                if (op.Length == 1)
                    oResult.SetValue (vlop > vrop);
                else if (prio == 8)
                    oResult.SetValue (vlop >= vrop);
                else if (prio == 9)
                    oResult.SetValue (vlop >> vrop);
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
                if (op.String [0] == '/')
                    oResult.SetValue (vlop / vrop);
                else
                    oResult.SetValue (vlop % vrop);
                break;
            }

            op = nextop;
        }
    }


    bool CPreprocessor::GetValue (const Token &iToken, long &oValue, int iLine)
    {
        Token r;
        const Token *vt = &iToken;

        if ((vt->Type == Token::TK_KEYWORD ||
             vt->Type == Token::TK_TEXT ||
             vt->Type == Token::TK_NUMBER) &&
            !vt->String)
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

            vt = &r;
        }

        Macro *m;
        switch (vt->Type)
        {
        case Token::TK_EOS:
        case Token::TK_ERROR:
            return false;

        case Token::TK_KEYWORD:
            // Try to expand the macro
            if ((m = IsDefined (*vt)) && !m->Expanding)
            {
                Token x = ExpandMacro (*vt);
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
        } while (oArg.Type == Token::TK_WHITESPACE ||
                 oArg.Type == Token::TK_NEWLINE ||
                 oArg.Type == Token::TK_COMMENT ||
                 oArg.Type == Token::TK_LINECOMMENT ||
                 oArg.Type == Token::TK_LINECONT);

        if (!iExpand)
        {
            if (oArg.Type == Token::TK_EOS)
                return oArg;
            else if (oArg.Type == Token::TK_PUNCTUATION &&
                     (oArg.String [0] == ',' ||
                      oArg.String [0] == ')'))
            {
                Token t = oArg;
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

        if( oArg.Type == Token::TK_PUNCTUATION && oArg.String[0] == '(' )
            ++braceCount;

        size_t len = oArg.Length;
        while (true)
        {
            Token t = GetToken (iExpand);
            switch (t.Type)
            {
            case Token::TK_EOS:
                Error (Line, "Unfinished list of arguments");
                OGRE_FALLTHROUGH;
            case Token::TK_ERROR:
                return Token (Token::TK_ERROR);
            case Token::TK_PUNCTUATION:
                if( t.String [0] == '(' )
                {
                    ++braceCount;
                }
                else if( !braceCount )
                {
                    if (t.String [0] == ',' ||
                        t.String [0] == ')')
                    {
                        // Trim whitespaces at the end
                        oArg.Length = len;

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
                            oArg.Append( "__arg_", 6 );
                        return t;
                    }
                }
                else
                {
                    if( t.String [0] == ')' )
                        --braceCount;
                }
                break;
            case Token::TK_LINECONT:
            case Token::TK_COMMENT:
            case Token::TK_LINECOMMENT:
            case Token::TK_NEWLINE:
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

            oArg.Append (t);

            if (t.Type != Token::TK_WHITESPACE)
                len = oArg.Length;
        }
    }


    CPreprocessor::Token CPreprocessor::GetArguments (std::vector<Token>& oArgs,
                                                      bool iExpand, bool shouldAppendArg)
    {
        Token args [MAX_MACRO_ARGS];
        int nargs = 0;

        // Suppose we'll leave by the wrong path
        oArgs.clear();

        bool isFirstTokenParsed = false;
        bool isFirstTokenNotAnOpenBrace = false;

        Token t;
        do
        {
            t = GetToken (iExpand);

            if( !isFirstTokenParsed &&
                (t.Type != Token::TK_PUNCTUATION || t.String [0] != '(') )
            {
                isFirstTokenNotAnOpenBrace = true;
            }
            isFirstTokenParsed = true;
        } while (t.Type == Token::TK_WHITESPACE ||
                 t.Type == Token::TK_COMMENT ||
                 t.Type == Token::TK_LINECOMMENT);

        if( isFirstTokenNotAnOpenBrace )
        {
            oArgs.clear();
            return t;
        }

        while (true)
        {
            if (nargs == MAX_MACRO_ARGS)
            {
                Error (Line, "Too many arguments to macro");
                return Token (Token::TK_ERROR);
            }

            t = GetArgument (args [nargs++], iExpand, shouldAppendArg);

            switch (t.Type)
            {
            case Token::TK_EOS:
                Error (Line, "Unfinished list of arguments");
                OGRE_FALLTHROUGH;
            case Token::TK_ERROR:
                return Token (Token::TK_ERROR);

            case Token::TK_PUNCTUATION:
                if (t.String [0] == ')')
                {
                    t = GetToken (iExpand);
                    goto Done;
                } // otherwise we've got a ','
                break;

            default:
                Error (Line, "Unexpected token", &t);
                break;
            }
        }

    Done:
        oArgs.insert(oArgs.begin(), args, args + nargs);
        return t;
    }


    bool CPreprocessor::HandleDefine (Token &iBody, int iLine)
    {
        // Create an additional preprocessor to process macro body
        CPreprocessor cpp (iBody, iLine);

        Token t = cpp.GetToken (false);
        if (t.Type != Token::TK_KEYWORD)
        {
            Error (iLine, "Macro name expected after #define");
            return false;
        }

        Macro m(t);
        m.Body = iBody;
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
            assert (t.String + t.Length == cpp.Source);
            t.Length = cpp.SourceEnd - t.String;
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
                cpp2.Define(arg.String, arg.Length - 6, arg.String, arg.Length);
            }

            // Now run the macro expansion through the supplimentary preprocessor
            Token xt = cpp2.Parse( t );
            t = xt;
        }

        m.Value = t;
        MacroList.push_front(std::move(m));
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
        Undef (t.String, t.Length);

        do
        {
            t = cpp.GetToken (false);
        } while (t.Type == Token::TK_WHITESPACE ||
                 t.Type == Token::TK_COMMENT ||
                 t.Type == Token::TK_LINECOMMENT);

        if (t.Type != Token::TK_EOS)
            Error (iLine, "Warning: Ignoring garbage after directive", &t);

        return true;
    }

    bool CPreprocessor::HandleIf(bool val, int iLine)
    {
        if (EnableOutput & (1 << 31))
        {
            Error (iLine, "Too many embedded #if directives");
            return false;
        }

        EnableElif <<= 1;
        EnableOutput <<= 1;
        if (val)
            EnableOutput |= 1;
        else
            EnableElif |= 1;

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

        if (!HandleIf(IsDefined(t), iLine))
            return false;

        do
        {
            t = cpp.GetToken (false);
        } while (t.Type == Token::TK_WHITESPACE ||
                 t.Type == Token::TK_COMMENT ||
                 t.Type == Token::TK_LINECOMMENT);

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

        const char *v = iParent->IsDefined (iArgs [0]) ? "1" : "0";
        return Token (Token::TK_NUMBER, v, 1);
    }

    bool CPreprocessor::GetValueDef(const Token &iToken, long &oValue, int iLine)
    {
        // Temporary add the defined() function to the macro list
        MacroList.emplace_front(Token(Token::TK_KEYWORD, "defined", 7));
        MacroList.front().ExpandFunc = ExpandDefined;
        MacroList.front().Args.resize(1);

        bool rc = GetValue (iToken, oValue, iLine);

        // Restore the macro list
        MacroList.pop_front();

        return rc;
    }

    bool CPreprocessor::HandleIf (Token &iBody, int iLine)
    {
        long val;
        return GetValueDef(iBody, val, iLine) && HandleIf(val, iLine);
    }

    bool CPreprocessor::HandleElif (Token &iBody, int iLine)
    {
        if (EnableOutput == 1)
        {
            Error (iLine, "#elif without #if");
            return false;
        }

        long val;
        if (!GetValueDef(iBody, val, iLine))
            return false;

        if (val && (EnableElif & 1))
        {
            EnableOutput |= 1;
            EnableElif &= ~1;
        }
        else
            EnableOutput &= ~1;

        return true;
    }


    bool CPreprocessor::HandleElse (Token &iBody, int iLine)
    {
        if (EnableOutput == 1)
        {
            Error (iLine, "#else without #if");
            return false;
        }

        // Negate the result of last #if
        if ((EnableElif & 1) || (EnableOutput & 1))
            EnableOutput ^= 1;

        if (iBody.Length)
            Error (iLine, "Warning: Ignoring garbage after #else", &iBody);

        return true;
    }


    bool CPreprocessor::HandleEndIf (Token &iBody, int iLine)
    {
        EnableElif >>= 1;
        EnableOutput >>= 1;
        if (EnableOutput == 0)
        {
            Error (iLine, "#endif without #if");
            return false;
        }

        if (iBody.Length)
            Error (iLine, "Warning: Ignoring garbage after #endif", &iBody);

        return true;
    }


    CPreprocessor::Token CPreprocessor::HandleDirective (Token &iToken, int iLine)
    {
        // Analyze preprocessor directive
        const char *directive = iToken.String + 1;
        size_t dirlen = iToken.Length - 1;
        while (dirlen && isspace (*directive))
            dirlen--, directive++;

        int old_line = Line;

        // Collect the remaining part of the directive until EOL
        Token t, last;
        do
        {
            t = GetToken (false);
            if (t.Type == Token::TK_NEWLINE)
            {
                // No directive arguments
                last = t;
                t.Length = 0;
                goto Done;
            }
        } while (t.Type == Token::TK_WHITESPACE ||
                 t.Type == Token::TK_LINECONT ||
                 t.Type == Token::TK_COMMENT ||
                 t.Type == Token::TK_LINECOMMENT);

        for (;;)
        {
            last = GetToken (false);
            switch (last.Type)
            {
            case Token::TK_EOS:
                // Can happen and is not an error
                goto Done;

            case Token::TK_LINECOMMENT:
            case Token::TK_COMMENT:
                // Skip comments in macros
                continue;

            case Token::TK_ERROR:
                return last;

            case Token::TK_LINECONT:
                continue;

            case Token::TK_NEWLINE:
                goto Done;

            default:
                break;
            }

            t.Append (last);
            t.Type = Token::TK_TEXT;
        }
    Done:

#define IS_DIRECTIVE(s)                                                 \
        (dirlen == strlen(s) && (strncmp (directive, s, dirlen) == 0))

        bool outputEnabled = ((EnableOutput & (EnableOutput + 1)) == 0);
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
            {
                EnableOutput ^= 1;
                EnableElif ^= 1;
            }
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
            Source = iToken.String + iToken.Length;
            iToken.Type = Token::TK_TEXT;
            return iToken;
        }

#undef IS_DIRECTIVE

        if (!rc)
            return Token (Token::TK_ERROR);
        return last;
    }


    void CPreprocessor::Define (const char *iMacroName, size_t iMacroNameLen,
                                const char *iMacroValue, size_t iMacroValueLen)
    {
        MacroList.emplace_front(Token(Token::TK_KEYWORD, iMacroName, iMacroNameLen));
        MacroList.front().Value = Token(Token::TK_TEXT, iMacroValue, iMacroValueLen);
    }


    void CPreprocessor::Define (const char *iMacroName, size_t iMacroNameLen)
    {
        MacroList.emplace_front(Token(Token::TK_KEYWORD, iMacroName, iMacroNameLen));
    }


    bool CPreprocessor::Undef (const char *iMacroName, size_t iMacroNameLen)
    {
        Token name (Token::TK_KEYWORD, iMacroName, iMacroNameLen);

        for (auto it = MacroList.before_begin();; ++it)
        {
            auto itpp = std::next(it);
            if(itpp == MacroList.end()) break;

            if (itpp->Name == name)
            {
                MacroList.erase_after(it);
                return true;
            }
        }

        return false;
    }


    CPreprocessor::Token CPreprocessor::Parse (const Token &iSource)
    {
        Source = iSource.String;
        SourceEnd = Source + iSource.Length;
        Line = 1;
        BOL = true;
        EnableOutput = 1;
        EnableElif = 0;

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

        NextToken:
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
                    output.Append (" ", 1);
                    output.AppendNL (Line - old_line);
                }
                break;

            case Token::TK_LINECOMMENT:
                // C++ comments are ignored
                continue;

            case Token::TK_DIRECTIVE:
                // Handle preprocessor directives
                t = HandleDirective (t, old_line);

                output_enabled = ((EnableOutput & (EnableOutput + 1)) == 0);
                if (output_enabled != old_output_enabled)
                {
                    if (output_enabled)
                        output.AppendNL (old_line - output_disabled_line);
                    else
                        output_disabled_line = old_line;
                    old_output_enabled = output_enabled;
                }

                if (output_enabled)
                    output.AppendNL (Line - old_line - t.CountNL ());
                goto NextToken;

            case Token::TK_LINECONT:
                // Backslash-Newline sequences are deleted, no matter where.
                empty_lines++;
                break;

            case Token::TK_PUNCTUATION:
                if (output_enabled && (!SupplimentaryExpand || t.String[0] != '#'))
                    output.Append (t);
                break;

            case Token::TK_NEWLINE:
                if (empty_lines)
                {
                    // Compensate for the backslash-newline combinations
                    // we have encountered, otherwise line numeration is broken
                    if (output_enabled)
                        output.AppendNL (empty_lines);
                    empty_lines = 0;
                }
                OGRE_FALLTHROUGH; // to default
            case Token::TK_WHITESPACE:
                // Fallthrough to default
            default:
                // Passthrough all other tokens
                if (output_enabled)
                    output.Append (t);
                break;
            }
        }

        if (EnableOutput != 1)
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

        oLength = retval.Length;
        retval.Allocated = 0;
        return retval.Buffer;
    }

} // namespace Ogre
