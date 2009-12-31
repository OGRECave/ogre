/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

//---------------------------------------------------------------------------//

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
        if (Length > i + 1 && String [i + 1] == 'x')
            base = 16, i += 2;
        else
            base = 8;

    for (; i < Length; i++)
    {
        long c = long (String [i]);
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
    char tmp [21];
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
    int l = Length;
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

//---------------------------------------------------------------------------//

CPreprocessor::Token CPreprocessor::Macro::Expand (
    int iNumArgs, CPreprocessor::Token *iArgs, Macro *iMacros)
{
    Expanding = true;

    CPreprocessor cpp;
    cpp.MacroList = iMacros;

    // Define a new macro for every argument
    int i;
    for (i = 0; i < iNumArgs; i++)
        cpp.Define (Args [i].String, Args [i].Length,
                    iArgs [i].String, iArgs [i].Length);
    // The rest arguments are empty
    for (; i < NumArgs; i++)
        cpp.Define (Args [i].String, Args [i].Length, "", 0);

    // Now run the macro expansion through the supplimentary preprocessor
    Token xt = cpp.Parse (Value);

    Expanding = false;

    // Remove the extra macros we have defined
    for (int i = NumArgs - 1; i >= 0; i--)
        cpp.Undef (Args [i].String, Args [i].Length);

    cpp.MacroList = NULL;

    return xt;
}

//---------------------------------------------------------------------------//

static void DefaultError (void *iData, int iLine, const char *iError,
                          const char *iToken, size_t iTokenLen)
{
    (void)iData;
    char line [1000];
    if (iToken)
        snprintf (line, sizeof (line), "line %d: %s: `%.*s'\n",
                  iLine, iError, int (iTokenLen), iToken);
    else
        snprintf (line, sizeof (line), "line %d: %s\n", iLine, iError);
    LogManager::getSingleton ().logMessage (line);
}

//---------------------------------------------------------------------------//

CPreprocessor::ErrorHandlerFunc CPreprocessor::ErrorHandler = DefaultError;

CPreprocessor::CPreprocessor (const Token &iToken, int iLine) : MacroList (NULL)
{
    Source = iToken.String;
    SourceEnd = iToken.String + iToken.Length;
    EnableOutput = 1;
    Line = iLine;
    BOL = true;
}

CPreprocessor::~CPreprocessor ()
{
    delete MacroList;
}

void CPreprocessor::Error (int iLine, const char *iError, const Token *iToken)
{
    if (iToken)
        ErrorHandler (ErrorData, iLine, iError, iToken->String, iToken->Length);
    else
        ErrorHandler (ErrorData, iLine, iError, NULL, 0);
}

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
    for (Macro *cur = MacroList; cur; cur = cur->Next)
        if (cur->Name == iToken)
            return cur;

    return NULL;
}

CPreprocessor::Token CPreprocessor::ExpandMacro (const Token &iToken)
{
    Macro *cur = IsDefined (iToken);
    if (cur && !cur->Expanding)
    {
        Token *args = NULL;
        int nargs = 0;
        int old_line = Line;

        if (cur->NumArgs != 0)
        {
            Token t = GetArguments (nargs, args, cur->ExpandFunc ? false : true);
            if (t.Type == Token::TK_ERROR)
            {
                delete [] args;
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
        }

        if (nargs > cur->NumArgs)
        {
            char tmp [60];
            snprintf (tmp, sizeof (tmp), "Macro `%.*s' passed %d arguments, but takes just %d",
                      int (cur->Name.Length), cur->Name.String,
                      nargs, cur->NumArgs);
            Error (old_line, tmp);
            return Token (Token::TK_ERROR);
        }

        Token t = cur->ExpandFunc ?
            cur->ExpandFunc (this, nargs, args) :
            cur->Expand (nargs, args, MacroList);
        t.AppendNL (Line - old_line);

        delete [] args;

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
        cpp.MacroList = MacroList;

        Token t;
        t = cpp.GetExpression (r, iLine);

        cpp.MacroList = NULL;

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

CPreprocessor::Token CPreprocessor::GetArgument (Token &oArg, bool iExpand)
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

    uint len = oArg.Length;
    while (true)
    {
        Token t = GetToken (iExpand);
        switch (t.Type)
        {
            case Token::TK_EOS:
                Error (Line, "Unfinished list of arguments");
            case Token::TK_ERROR:
                return Token (Token::TK_ERROR);
            case Token::TK_PUNCTUATION:
                if (t.String [0] == ',' ||
                    t.String [0] == ')')
                {
                    // Trim whitespaces at the end
                    oArg.Length = len;
                    return t;
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

CPreprocessor::Token CPreprocessor::GetArguments (int &oNumArgs, Token *&oArgs,
                                                  bool iExpand)
{
    Token args [MAX_MACRO_ARGS];
    int nargs = 0;

    // Suppose we'll leave by the wrong path
    oNumArgs = 0;
    oArgs = NULL;

    Token t;
    do
    {
        t = GetToken (iExpand);
    } while (t.Type == Token::TK_WHITESPACE ||
             t.Type == Token::TK_COMMENT ||
             t.Type == Token::TK_LINECOMMENT);

    if (t.Type != Token::TK_PUNCTUATION || t.String [0] != '(')
    {
        oNumArgs = 0;
        oArgs = NULL;
        return t;
    }

    while (true)
    {
        if (nargs == MAX_MACRO_ARGS)
        {
            Error (Line, "Too many arguments to macro");
            return Token (Token::TK_ERROR);
        }

        t = GetArgument (args [nargs++], iExpand);

        switch (t.Type)
        {
            case Token::TK_EOS:
                Error (Line, "Unfinished list of arguments");
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
    oNumArgs = nargs;
    oArgs = new Token [nargs];
    for (int i = 0; i < nargs; i++)
        oArgs [i] = args [i];
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

    Macro *m = new Macro (t);
    m->Body = iBody;
    t = cpp.GetArguments (m->NumArgs, m->Args, false);
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
            delete m;
            return false;

        default:
            t.Type = Token::TK_TEXT;
            assert (t.String + t.Length == cpp.Source);
            t.Length = cpp.SourceEnd - t.String;
            break;
    }

    m->Value = t;
    m->Next = MacroList;
    MacroList = m;
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

bool CPreprocessor::HandleIfDef (Token &iBody, int iLine)
{
    if (EnableOutput & (1 << 31))
    {
        Error (iLine, "Too many embedded #if directives");
        return false;
    }

    CPreprocessor cpp (iBody, iLine);

    Token t = cpp.GetToken (false);

    if (t.Type != Token::TK_KEYWORD)
    {
        Error (iLine, "Expecting a macro name after #ifdef, got", &t);
        return false;
    }

    EnableOutput <<= 1;
    if (IsDefined (t))
        EnableOutput |= 1;

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

CPreprocessor::Token CPreprocessor::ExpandDefined (CPreprocessor *iParent, int iNumArgs, Token *iArgs)
{
    if (iNumArgs != 1)
    {
        iParent->Error (iParent->Line, "The defined() function takes exactly one argument");
        return Token (Token::TK_ERROR);
    }

    const char *v = iParent->IsDefined (iArgs [0]) ? "1" : "0";
    return Token (Token::TK_NUMBER, v, 1);
}

bool CPreprocessor::HandleIf (Token &iBody, int iLine)
{
    Macro defined (Token (Token::TK_KEYWORD, "defined", 7));
    defined.Next = MacroList;
    defined.ExpandFunc = ExpandDefined;
    defined.NumArgs = 1;

    // Temporary add the defined() function to the macro list
    MacroList = &defined;

    long val;
    bool rc = GetValue (iBody, val, iLine);

    // Restore the macro list
    MacroList = defined.Next;
    defined.Next = NULL;

    if (!rc)
        return false;

    EnableOutput <<= 1;
    if (val)
        EnableOutput |= 1;

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
    EnableOutput ^= 1;

    if (iBody.Length)
        Error (iLine, "Warning: Ignoring garbage after #else", &iBody);

    return true;
}

bool CPreprocessor::HandleEndIf (Token &iBody, int iLine)
{
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
    int dirlen = iToken.Length - 1;
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

#define IS_DIRECTIVE(s) \
    ((dirlen == sizeof (s) - 1) && (strncmp (directive, s, sizeof (s) - 1) == 0))

    bool rc;
    if (IS_DIRECTIVE ("define"))
        rc = HandleDefine (t, iLine);
    else if (IS_DIRECTIVE ("undef"))
        rc = HandleUnDef (t, iLine);
    else if (IS_DIRECTIVE ("ifdef"))
        rc = HandleIfDef (t, iLine);
    else if (IS_DIRECTIVE ("ifndef"))
    {
        rc = HandleIfDef (t, iLine);
        if (rc)
            EnableOutput ^= 1;
    }
    else if (IS_DIRECTIVE ("if"))
        rc = HandleIf (t, iLine);
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
    Macro *m = new Macro (Token (Token::TK_KEYWORD, iMacroName, iMacroNameLen));
    m->Value = Token (Token::TK_TEXT, iMacroValue, iMacroValueLen);
    m->Next = MacroList;
    MacroList = m;
}

void CPreprocessor::Define (const char *iMacroName, size_t iMacroNameLen,
                            long iMacroValue)
{
    Macro *m = new Macro (Token (Token::TK_KEYWORD, iMacroName, iMacroNameLen));
    m->Value.SetValue (iMacroValue);
    m->Next = MacroList;
    MacroList = m;
}

bool CPreprocessor::Undef (const char *iMacroName, size_t iMacroNameLen)
{
    Macro **cur = &MacroList;
    Token name (Token::TK_KEYWORD, iMacroName, iMacroNameLen);
    while (*cur)
    {
        if ((*cur)->Name == name)
        {
            Macro *next = (*cur)->Next;
            (*cur)->Next = NULL;
            delete (*cur);
            *cur = next;
            return true;
        }

        cur = &(*cur)->Next;
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

            case Token::TK_NEWLINE:
                if (empty_lines)
                {
                    // Compensate for the backslash-newline combinations
                    // we have encountered, otherwise line numeration is broken
                    if (output_enabled)
                        output.AppendNL (empty_lines);
                    empty_lines = 0;
                }
                // Fallthrough to default
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
