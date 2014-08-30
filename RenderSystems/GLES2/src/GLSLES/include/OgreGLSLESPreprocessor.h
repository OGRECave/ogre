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

#ifndef __OGRE_CPREPROCESSOR_H__
#define __OGRE_CPREPROCESSOR_H__

#include <string.h>
#include <stdlib.h>

namespace Ogre {

/**
 * This is a simplistic C/C++-like preprocessor.
 * It takes an non-zero-terminated string on input and outputs a
 * non-zero-terminated string buffer.
 *
 * This preprocessor was designed specifically for GLSL ES shaders, so
 * if you want to use it for other purposes you might want to check
 * if the feature set it provides is enough for you.
 *
 * Here's a list of supported features:
 * <ul>
 * <li>Fast memory allocation-less operation (mostly).
 * <li>Line continuation (backslash-newline) is swallowed.
 * <li>Line numeration is fully preserved by inserting empty lines where
 *     required. This is crucial if, say, GLSL ES compiler reports you an error
 *     with a line number.
 * <li>#define: Parametrized and non-parametrized macros. Invoking a macro with
 *     less arguments than it takes assignes empty values to missing arguments.
 * <li>#undef: Forget defined macros
 * <li>#ifdef/#ifndef/#else/#endif: Conditional suppression of parts of code.
 * <li>#if: Supports numeric expression of any complexity, also supports the
 *     defined() pseudo-function.
 * </ul>
 */
class CPreprocessor
{
    /**
     * A input token.
     *
     * For performance reasons most tokens will point to portions of the
     * input stream, so no unneeded memory allocation is done. However,
     * in some cases we must allocate different memory for token storage,
     * in this case this is signalled by setting the Allocated member
     * to non-zero in which case the destructor will know that it must
     * free memory on object destruction.
     *
     * Again for performance reasons we use malloc/realloc/free here because
     * C++-style new[] lacks the realloc() counterpart.
     */
    class Token
    {
    public:
        enum Kind
        {
            TK_EOS,          // End of input stream
            TK_ERROR,        // An error has been encountered
            TK_WHITESPACE,   // A whitespace span (but not newline)
            TK_NEWLINE,      // A single newline (CR & LF)
            TK_LINECONT,     // Line continuation ('\' followed by LF)
            TK_NUMBER,       // A number
            TK_KEYWORD,      // A keyword
            TK_PUNCTUATION,  // A punctuation character
            TK_DIRECTIVE,    // A preprocessor directive
            TK_STRING,       // A string
            TK_COMMENT,      // A block comment
            TK_LINECOMMENT,  // A line comment
            TK_TEXT          // An unparsed text (cannot be returned from GetToken())
        };

        /// Token type
        Kind Type;
        /// True if string was allocated (and must be freed)
        mutable size_t Allocated;
        union
        {
            /// A pointer somewhere into the input buffer
            const char *String;
            /// A memory-allocated string
            char *Buffer;
        };
        /// Token length in bytes
        size_t Length;

        Token () : Allocated (0), String (NULL)
        { }

        Token (Kind iType) : Type (iType), Allocated (0), String (NULL)
        { }

        Token (Kind iType, const char *iString, size_t iLength) :
            Type (iType), Allocated (0), String (iString), Length (iLength)
        { }

        Token (const Token &iOther)
        {
            Type = iOther.Type;
            Allocated = iOther.Allocated;
            iOther.Allocated = 0; // !!! not quite correct but effective
            String = iOther.String;
            Length = iOther.Length;
        }

        ~Token ()
        { if (Allocated) free (Buffer); }

        /// Assignment operator
        Token &operator = (const Token &iOther)
        {
            if (Allocated) free (Buffer);
            Type = iOther.Type;
            Allocated = iOther.Allocated;
            iOther.Allocated = 0; // !!! not quite correct but effective
            String = iOther.String;
            Length = iOther.Length;
            return *this;
        }

        /// Append a string to this token
        void Append (const char *iString, size_t iLength);

        /// Append a token to this token
        void Append (const Token &iOther);

        /// Append given number of newlines to this token
        void AppendNL (int iCount);

        /// Count number of newlines in this token
        int CountNL ();

        /// Get the numeric value of the token
        bool GetValue (long &oValue) const;

        /// Set the numeric value of the token
        void SetValue (long iValue);

        /// Test two tokens for equality
        bool operator == (const Token &iOther)
        {
            if (iOther.Length != Length)
                return false;
            return (memcmp (String, iOther.String, Length) == 0);
        }
    };

    /// A macro definition
    class Macro
    {
    public:
        /// Macro name
        Token Name;
        /// Number of arguments
        int NumArgs;
        /// The names of the arguments
        Token *Args;
        /// The macro value
        Token Value;
        /// Unparsed macro body (keeps the whole raw unparsed macro body)
        Token Body;
        /// Next macro in chained list
        Macro *Next;
        /// A pointer to function implementation (if macro is really a func)
        Token (*ExpandFunc) (CPreprocessor *iParent, int iNumArgs, Token *iArgs);
        /// true if macro expansion is in progress
        bool Expanding;

        Macro (const Token &iName) :
            Name (iName), NumArgs (0), Args (NULL), Next (NULL),
            ExpandFunc (NULL), Expanding (false)
        { }

        ~Macro ()
        //{ OGRE_DELETE [] Args; OGRE_DELETE Next; }
        { delete [] Args; delete Next; }

        /// Expand the macro value (will not work for functions)
        Token Expand (int iNumArgs, Token *iArgs, Macro *iMacros);
    };

    friend class CPreprocessor::Macro;

    /// The current source text input
    const char *Source;
    /// The end of the source text
    const char *SourceEnd;
    /// Current line number
    int Line;
    /// True if we are at beginning of line
    bool BOL;
    /// A stack of 32 booleans packed into one value :)
    unsigned EnableOutput;
    /// The list of macros defined so far
    Macro *MacroList;

    /**
     * Private constructor to re-parse a single token.
     */
    CPreprocessor (const Token &iToken, int iLine);

    /**
     * Stateless tokenizer: Parse the input text and return the next token.
     * @param iExpand
     *     If true, macros will be expanded to their values
     * @return
     *     The next token from the input stream
     */
    Token GetToken (bool iExpand);

    /**
     * Handle a preprocessor directive.
     * @param iToken
     *     The whole preprocessor directive line (until EOL)
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     The last input token that was not proceeded.
     */
    Token HandleDirective (Token &iToken, int iLine);

    /**
     * Handle a #define directive.
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleDefine (Token &iBody, int iLine);

    /**
     * Undefine a previously defined macro
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleUnDef (Token &iBody, int iLine);

    /**
     * Handle an #ifdef directive.
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleIfDef (Token &iBody, int iLine);

    /**
     * Handle an #if directive.
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleIf (Token &iBody, int iLine);

    /**
     * Handle an #else directive.
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleElse (Token &iBody, int iLine);

    /**
     * Handle an #endif directive.
     * @param iBody
     *     The body of the directive (everything after the directive
     *     until end of line).
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if everything went ok, false if not
     */
    bool HandleEndIf (Token &iBody, int iLine);

    /**
     * Get a single function argument until next ',' or ')'.
     * @param oArg
     *     The argument is returned in this variable.
     * @param iExpand
     *     If false, parameters are not expanded and no expressions are
     *     allowed; only a single keyword is expected per argument.
     * @return
     *     The first unhandled token after argument.
     */
    Token GetArgument (Token &oArg, bool iExpand);

    /**
     * Get all the arguments of a macro: '(' arg1 { ',' arg2 { ',' ... }} ')'
     * @param oNumArgs
     *     Number of parsed arguments is stored into this variable.
     * @param oArgs
     *     This is set to a pointer to an array of parsed arguments.
     * @param iExpand
     *     If false, parameters are not expanded and no expressions are
     *     allowed; only a single keyword is expected per argument.
     */
    Token GetArguments (int &oNumArgs, Token *&oArgs, bool iExpand);

    /**
     * Parse an expression, compute it and return the result.
     * @param oResult
     *     A token containing the result of expression
     * @param iLine
     *     The line at which the expression starts (for error reports)
     * @param iOpPriority
     *     Operator priority (at which operator we will stop if
     *     proceeding recursively -- used internally. Parser stops
     *     when it encounters an operator with higher or equal priority).
     * @return
     *     The last unhandled token after the expression
     */
    Token GetExpression (Token &oResult, int iLine, int iOpPriority = 0);

    /**
     * Get the numeric value of a token.
     * If the token was produced by expanding a macro, we will get
     * an TEXT token which can contain a whole expression; in this
     * case we will call GetExpression to parse it. Otherwise we
     * just call the token's GetValue() method.
     * @param iToken
     *     The token to get the numeric value of
     * @param oValue
     *     The variable to put the value into
     * @param iLine
     *     The line where the directive begins (for error reports)
     * @return
     *     true if ok, false if not
     */
    bool GetValue (const Token &iToken, long &oValue, int iLine);

    /**
     * Expand the given macro, if it exists.
     * If macro has arguments, they are collected from source stream.
     * @param iToken
     *     A KEYWORD token containing the (possible) macro name.
     * @return
     *     The expanded token or iToken if it is not a macro
     */
    Token ExpandMacro (const Token &iToken);

    /**
     * Check if a macro is defined, and if so, return it
     * @param iToken
     *     Macro name
     * @return
     *     The macro object or NULL if a macro with this name does not exist
     */
    Macro *IsDefined (const Token &iToken);

    /**
     * The implementation of the defined() preprocessor function
     * @param iParent
     *     The parent preprocessor object
     * @param iNumArgs
     *     Number of arguments
     * @param iArgs
     *     The arguments themselves
     * @return
     *     The return value encapsulated in a token
     */
    static Token ExpandDefined (CPreprocessor *iParent, int iNumArgs, Token *iArgs);

    /**
     * Parse the input string and return a token containing the whole output.
     * @param iSource
     *     The source text enclosed in a token
     * @return
     *     The output text enclosed in a token
     */
    Token Parse (const Token &iSource);

    /**
     * Call the error handler
     * @param iLine
     *     The line at which the error happened.
     * @param iError
     *     The error string.
     * @param iToken
     *     If not NULL contains the erroneous token
     */
    void Error (int iLine, const char *iError, const Token *iToken = NULL);

public:
    /// Create an empty preprocessor object
    CPreprocessor () : MacroList (NULL)
    { }

    /// Destroy the preprocessor object
    virtual ~CPreprocessor ();

    /**
     * Define a macro without parameters.
     * @param iMacroName
     *     The name of the defined macro
     * @param iMacroNameLen
     *     The length of the name of the defined macro
     * @param iMacroValue
     *     The value of the defined macro
     * @param iMacroValueLen
     *     The length of the value of the defined macro
     */
    void Define (const char *iMacroName, size_t iMacroNameLen,
                 const char *iMacroValue, size_t iMacroValueLen);

    /**
     * Define a numerical macro.
     * @param iMacroName
     *     The name of the defined macro
     * @param iMacroNameLen
     *     The length of the name of the defined macro
     * @param iMacroValue
     *     The value of the defined macro
     */
    void Define (const char *iMacroName, size_t iMacroNameLen, long iMacroValue);

    /**
     * Undefine a macro.
     * @param iMacroName
     *     The name of the macro to undefine
     * @param iMacroNameLen
     *     The length of the name of the macro to undefine
     * @return
     *     true if the macro has been undefined, false if macro doesn't exist
     */
    bool Undef (const char *iMacroName, size_t iMacroNameLen);

    /**
     * Parse the input string and return a newly-allocated output string.
     * @note
     *     The returned preprocessed string is NOT zero-terminated
     *     (just like the input string).
     * @param iSource
     *     The source text
     * @param iLength
     *     The length of the source text in characters
     * @param oLength
     *     The length of the output string.
     * @return
     *     The output from preprocessor, allocated with malloc().
     *     The parser can actually allocate more than needed for performance
     *     reasons, but this should not be a problem unless you will want
     *     to store the returned pointer for long time in which case you
     *     might want to realloc() it.
     *     If an error has been encountered, the function returns NULL.
     *     In some cases the function may return an unallocated address
     *     that's *inside* the source buffer. You must free() the result
     *     string only if the returned address is not inside the source text.
     */
    char *Parse (const char *iSource, size_t iLength, size_t &oLength);

    /**
     * An error handler function type.
     * The default implementation just drops a note to stderr and
     * then the parser ends, returning NULL.
     * @param iData
     *     User-specific pointer from the corresponding CPreprocessor object.
     * @param iLine
     *     The line at which the error happened.
     * @param iError
     *     The error string.
     * @param iToken
     *     If not NULL contains the erroneous token
     * @param iTokenLen
     *     The length of iToken. iToken is never zero-terminated!
     */
    typedef void (*ErrorHandlerFunc) (
        void *iData, int iLine, const char *iError,
        const char *iToken, size_t iTokenLen);

    /**
     * A pointer to the preprocessor's error handler.
     * You can assign the address of your own function to this variable
     * and implement your own error handling (e.g. throwing an exception etc).
     */
    static ErrorHandlerFunc ErrorHandler;

    /// User-specific storage, passed to Error()
    void *ErrorData;
};

} // namespace Ogre

#endif // __OGRE_CPREPROCESSOR_H__
