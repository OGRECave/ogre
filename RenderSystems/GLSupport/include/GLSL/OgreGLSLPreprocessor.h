/**
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

#include "OgrePlatform.h"
#include <string.h>
#include <cstring>
#include <cstdint>
#include <stdlib.h>
#include <forward_list>
#include <vector>
#include <string>

namespace Ogre {

    /**
     * This is a simplistic C/C++-like preprocessor.
     * It takes an non-zero-terminated string on input and outputs a
     * non-zero-terminated string buffer.
     *
     * This preprocessor was designed specifically for GLSL shaders, so
     * if you want to use it for other purposes you might want to check
     * if the feature set it provides is enough for you.
     *
     * Here's a list of supported features:
     * - Fast memory allocation-less operation (mostly).
     * - Line continuation (backslash-newline) is swallowed.
     * - Line numeration is fully preserved by inserting empty lines where
     *     required. This is crucial if, say, GLSL compiler reports you an error
     *     with a line number.
     * - @c \#define: Parametrized and non-parametrized macros. Invoking a macro with
     *     less arguments than it takes assignes empty values to missing arguments.
     * - @c \#undef: Forget defined macros
     * - @c \#ifdef / @c \#ifndef / @c \#else / @c \#endif: Conditional suppression of parts of code.
     * - @c \#if: Supports numeric expression of any complexity, also supports the
     *     defined() pseudo-function.
     */
    class CPreprocessor
    {
        /// Non-owning (pointer, length) pair. A NULL pointer means "no text at
        /// all", which is distinct from an empty string.
        struct StringView
        {
            const char* String = nullptr;
            size_t Length = 0;

            StringView() = default;
            StringView(const char* iString, size_t iLength) : String(iString), Length(iLength) {}
            template <size_t N>
            StringView(const char (&iLiteral)[N]) : String(iLiteral), Length(N - 1) {}

            char operator[](size_t i) const { return String[i]; }

            bool operator==(const StringView& iOther) const
            {
                return Length == iOther.Length &&
                       (Length == 0 || std::memcmp(String, iOther.String, Length) == 0);
            }
        };

        /**
         * Inside text[Begin, End) the macro `Name` must not be expanded again.
         * This is the "hide set" of the C rescan rule: it keeps a macro that is
         * being expanded from re-expanding itself when its output is rescanned.
         */
        struct DisabledMacro
        {
            size_t Begin;
            size_t End;
            std::string Name;
        };

        /**
         * A string that either borrows a span of some other buffer (usually the
         * source) or owns a malloc'ed buffer. Move-only; an explicit Borrow() or
         * Clone() is required to duplicate it.
         *
         * Again for performance reasons we use malloc/realloc/free here because
         * C++-style new[] lacks the realloc() counterpart.
         */
        class Text
        {
        public:
            Text() = default;
            Text(const char* iString, size_t iLength) : String(iString), Length(iLength) {}
            ~Text() { FreeStorage(); }

            Text(Text&& iOther) noexcept;
            Text& operator=(Text&& iOther) noexcept;

            Text(const Text&) = delete;
            Text& operator=(const Text&) = delete;

            const char* GetString() const { return String; }
            size_t GetLength() const { return Length; }
            StringView View() const { return StringView(String, Length); }
            bool IsNull() const { return String == nullptr; }
            /// True if this text owns its buffer (and must free it)
            bool IsAllocated() const { return Allocated != 0; }

            /// Non-owning alias of the same characters (disabled ranges are copied)
            Text Borrow() const;
            /// Deep copy that owns its storage (disabled ranges are copied)
            Text Clone() const;

            /// Shorten the text (keeps capacity)
            void Truncate(size_t iLength);
            /// Change the length of a borrowed span, e.g. to extend it to the end of a line
            void ResizeBorrowed(size_t iLength);
            /// Replace the contents by an owned copy of the given characters
            void Assign(const char* iString, size_t iLength);

            /// Append characters that are known to live in a long-lived buffer:
            /// adjacent spans are merged without copying, and appending to NULL
            /// text just borrows.
            void AppendView(StringView iString);
            /// Append a copy of the characters; never borrows (for literals and
            /// other short-lived memory)
            void AppendCopy(StringView iString);
            /// Append another text including its disabled ranges
            void Append(const Text& iOther);
            /// Like Append(const Text&), but may adopt iOther's buffer (zero copy)
            /// when this text is still empty
            void Append(Text&& iOther);
            /// Append given number of newlines to this text
            void AppendNL(int iCount);

            /// Hand the buffer over to the caller; for borrowed text this is the
            /// (const-cast) pointer into the original buffer.
            char* Release();

            // --- rescan suppression -------------------------------------

            bool IsDisabled(StringView iMacroName) const;
            /// Disable `iMacroName` in the whole text
            void Disable(StringView iMacroName);
            /// Add disabled ranges of iSource that cover [iOffset, iOffset + iLength)
            void InheritDisabled(const Text& iSource, size_t iOffset, size_t iLength);
            bool HasDisabledMacros() const { return !DisabledMacros.empty(); }

        private:
            char* Buffer() { return const_cast<char*>(String); }
            void FreeStorage();
            void AppendDisabled(const Text& iOther);
            /// Make sure we own a buffer that can take `iExtra` more bytes
            void EnsureAllocated(size_t iExtra);
            /// Requires non-NULL text
            void AppendChars(const char* iString, size_t iLength);

            /// A pointer somewhere into the input buffer, or an owned buffer
            const char* String = nullptr;
            /// Text length in bytes
            size_t Length = 0;
            /// 0: borrowed; otherwise the size of the owned buffer
            size_t Allocated = 0;
            std::vector<DisabledMacro> DisabledMacros;
        };

        /// A token kind plus its text. Tokens from the Lexer are views into the source.
        class Token
        {
        public:
            enum Kind
            {
                TK_EOS,          ///< End of input stream
                TK_ERROR,        ///< An error has been encountered
                TK_WHITESPACE,   ///< A whitespace span (but not newline)
                TK_NEWLINE,      ///< A single newline (LF or CR LF)
                TK_LINECONT,     ///< Line continuation ('\' followed by a newline)
                TK_NUMBER,       ///< A number
                TK_KEYWORD,      ///< A keyword / identifier
                TK_PUNCTUATION,  ///< A punctuation character
                TK_DIRECTIVE,    ///< A preprocessor directive ('#' + name)
                TK_STRING,       ///< A string
                TK_COMMENT,      ///< A block comment
                TK_LINECOMMENT,  ///< A line comment
                TK_TEXT          ///< Unparsed text (never returned by the Lexer)
            };

            /// Token type
            Kind Type = TK_EOS;
            /// Token text
            Text Str;

            Token() = default;
            explicit Token(Kind iType) : Type(iType) {}
            Token(Kind iType, const char* iString, size_t iLength) : Type(iType), Str(iString, iLength) {}
            Token(Kind iType, Text&& iText) : Type(iType), Str(std::move(iText)) {}

            Token(Token&&) = default;
            Token& operator=(Token&&) = default;

            const char* String() const { return Str.GetString(); }
            size_t Length() const { return Str.GetLength(); }
            StringView View() const { return Str.View(); }

            /// Non-owning alias of this token
            Token Borrow() const { return Token(Type, Str.Borrow()); }

            bool IsPunctuation(char c) const { return Type == TK_PUNCTUATION && String()[0] == c; }

            static bool IsBlank(Kind iType)
            {
                return iType == TK_WHITESPACE || iType == TK_COMMENT || iType == TK_LINECOMMENT;
            }
            static bool IsBlankOrLineCont(Kind iType) { return IsBlank(iType) || iType == TK_LINECONT; }
            static bool IsBlankOrNewline(Kind iType)
            {
                return IsBlankOrLineCont(iType) || iType == TK_NEWLINE;
            }

            /// Count number of newlines in this token
            int CountNL() const;

            /// Get the numeric value of the token (decimal, octal or hex)
            bool GetValue(long& oValue) const;

            /// Set the numeric value of the token
            void SetValue(long iValue);
        };

        class MacroTable;

        /// A macro definition
        class Macro
        {
        public:
            /// Macro name
            StringView Name;
            /// The names of the arguments, each with "__arg_" appended
            std::vector<Token> Args;
            /// The macro value
            Token Value;
            /// Unparsed macro body; owns the text Name/Args/Value may point into
            Token Body;
            /// A pointer to function implementation (if macro is really a func)
            Token (*ExpandFunc) (CPreprocessor *iParent, const std::vector<Token>& iArgs) = nullptr;
            /// true if macro expansion is in progress (blocks recursion)
            bool Expanding = false;

            /// Expand the macro value (will not work for functions)
            Token Expand (const std::vector<Token>& iArgs, MacroTable& iMacros);
        };

        /// The macros defined so far; the most recent definition shadows older
        /// ones of the same name.
        class MacroTable
        {
        public:
            /// Check if a macro is defined, and if so, return it
            Macro* IsDefined(StringView iName);

            Macro& Define(Macro&& iMacro);
            /// Define a plain value macro
            Macro& Define(StringView iName, Text&& iValue);

            void PopFront();

            /// Remove the most recent definition of iName
            bool Undef(StringView iName);

            /// Keeps a macro alive across calls that might #undef it
            class Pin
            {
            public:
                Pin(MacroTable& iTable, const Macro& iMacro) : Table(iTable)
                {
                    Table.Pinned.push_back(&iMacro);
                }
                ~Pin() { Table.Pinned.pop_back(); }
                Pin(const Pin&) = delete;
                Pin& operator=(const Pin&) = delete;

            private:
                MacroTable& Table;
            };

        private:
            bool IsPinned(const Macro& iMacro) const;

            std::forward_list<Macro> Macros;
            std::forward_list<Macro> Retired;
            std::vector<const Macro*> Pinned;
        };

        friend class CPreprocessor::Macro;

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

        /// The current source text input
        const Token* SourceToken;
        const char *Source;
        /// The end of the source text
        const char *SourceEnd;
        /// Current line number
        int Line;
        /// True if we are at beginning of line
        bool BOL;
        /// Are we expanding a macro?
        bool SupplimentaryExpand;
        /// State of the nested #if/#elif/#else/#endif directives
        ConditionalStack Conditions;
        /// The list of macros defined so far
        MacroTable MacroList;

        /// The character @a iAhead positions past Source, or zero at the end of
        /// the source (the source need not be zero-terminated)
        char Peek (size_t iAhead = 0) const
        { return Source + iAhead < SourceEnd ? Source [iAhead] : '\0'; }

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
         * Collect the remaining part of the directive until EOL.
         * @param oLast
         *     Receives the token that ended the directive body.
         * @return
         *     The body of the directive (everything after the directive name).
         */
        Token GetDirectiveBody (Token &oLast);

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

        /// @overload
        bool HandleIf(bool val, int iLine);

        /**
         * Handle an #elif directive.
         * @param iBody
         *     The body of the directive (everything after the directive
         *     until end of line).
         * @param iLine
         *     The line where the directive begins (for error reports)
         * @return
         *     true if everything went ok, false if not
         */
        bool HandleElif (Token &iBody, int iLine);

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
         * @param shouldAppendArg
         *     When true, the argument will be appended the word word __arg_
         *     e.g. #define myMacro(x) --> #define myMacro(x__arg_)
         *     This workaround a bug where calling myMacro( x ) would cause
         *     issues.
         * @return
         *     The first unhandled token after argument.
         */
        Token GetArgument (Token &oArg, bool iExpand, bool shouldAppendArg);

        /**
         * Get all the arguments of a macro: '(' arg1 { ',' arg2 { ',' ... }} ')'
         * @param oArgs
         *     This is set to a pointer to an array of parsed arguments.
         * @param shouldAppendArg
         *      See GetArgument.
         * @param iExpand
         *     If false, parameters are not expanded and no expressions are
         *     allowed; only a single keyword is expected per argument.
         */
        Token GetArguments (std::vector<Token>& oArgs, bool iExpand, bool shouldAppendArg);

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

        /// @overload
        /// same as above, but considers the defined() function
        bool GetValueDef(const Token &iToken, long &oValue, int iLine);

        /**
         * Expand the given macro, if it exists.
         * If macro has arguments, they are collected from source stream.
         * @param iToken
         *     A KEYWORD token containing the (possible) macro name.
         * @return
         *     The expanded token or iToken if it is not a macro
         */
        Token ExpandMacro (Token iToken);


        /**
         * The implementation of the defined() preprocessor function
         * @param iParent
         *     The parent preprocessor object
         * @param iArgs
         *     The arguments themselves
         * @return
         *     The return value encapsulated in a token
         */
        static Token ExpandDefined (CPreprocessor *iParent, const std::vector<Token>& iArgs);

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
        CPreprocessor();

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
         * Define a symbolical macro.
         * @param iMacroName
         *     The name of the defined macro
         * @param iMacroNameLen
         *     The length of the name of the defined macro
         */
        void Define (const char *iMacroName, size_t iMacroNameLen);

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
    };

} // namespace Ogre

#endif // __OGRE_CPREPROCESSOR_H__
