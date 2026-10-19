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

#include <cstddef>
#include <cstring>
#include <forward_list>
#include <string>
#include <utility>
#include <vector>

namespace Ogre {

    /**
     * A simplistic C/C++-like preprocessor.
     * It takes a non-zero-terminated string on input and outputs a
     * non-zero-terminated string buffer.
     *
     * This preprocessor was designed specifically for GLSL shaders, so
     * if you want to use it for other purposes you might want to check
     * if the feature set it provides is enough for you.
     *
     * Supported features:
     * - Fast, (mostly) allocation-free operation: the source is parsed in situ,
     *     tokens are views into it, and untouched input is returned as-is.
     * - Line continuation (backslash-newline) is swallowed.
     * - Line numeration is fully preserved by inserting empty lines where
     *     required, so compiler error line numbers stay valid.
     * - @c \#define: parametrized and non-parametrized macros. Invoking a macro
     *     with fewer arguments than it takes assigns empty values to the rest.
     * - @c \#undef: forget defined macros
     * - @c \#ifdef / @c \#ifndef / @c \#else / @c \#endif: conditional suppression.
     * - @c \#if / @c \#elif: numeric expressions of any complexity, including the
     *     defined() pseudo-function.
     *
     * @par Lifetime (in-situ contract)
     * Macros keep *views* into the memory they were defined from: both the
     * strings passed to Define() and the source text passed to Parse() (for
     * macros defined by \#define) must outlive every later use of this object.
     */
    class CPreprocessor
    {
    public:
        /// Create an empty preprocessor object
        CPreprocessor() = default;
        ~CPreprocessor() = default;

        CPreprocessor(CPreprocessor&&) noexcept = default;
        CPreprocessor& operator=(CPreprocessor&&) noexcept = default;

        /**
         * Define a macro with a value. The strings are not copied.
         * @param iMacroName      The name of the defined macro
         * @param iMacroNameLen   The length of the name
         * @param iMacroValue     The value of the defined macro
         * @param iMacroValueLen  The length of the value
         */
        void Define(const char* iMacroName, size_t iMacroNameLen, const char* iMacroValue,
                    size_t iMacroValueLen);

        /// Define a symbolical macro (no value). The name is not copied.
        void Define(const char* iMacroName, size_t iMacroNameLen);

        /// Undefine a macro; @return false if it did not exist.
        bool Undef(const char* iMacroName, size_t iMacroNameLen);

        /**
         * Parse the input string and return a newly-allocated output string.
         * @note The result is NOT zero-terminated (just like the input).
         * @param iSource  The source text
         * @param iLength  Its length in bytes
         * @param oLength  Receives the length of the result
         * @return The result, or NULL on error (errors are logged).
         *     The result is allocated with malloc(), *except* when nothing had
         *     to be changed: then it points *into* @a iSource. The caller must
         *     free() it only if it does not lie inside the source buffer.
         */
        char* Parse(const char* iSource, size_t iLength, size_t& oLength);

    private:
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

        /// A macro definition
        struct Macro
        {
            /// Macro name
            StringView Name;
            /// The names of the arguments, each with "__arg_" appended
            std::vector<Token> Args;
            /// The macro value
            Token Value;
            /// Unparsed macro body; owns the text Name/Args/Value may point into
            Token Body;
            /// true if this macro is the built-in defined() function
            bool IsDefinedFunc = false;
            /// true if macro expansion is in progress (blocks recursion)
            bool Expanding = false;
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

        /// Splits a Text into tokens, in situ. Defined in the implementation file.
        class Lexer;
        /// Handles directives, macro expansion and output. Defined in the
        /// implementation file.
        class Processor;

        /// The list of macros defined so far
        MacroTable MacroList;
    };

} // namespace Ogre

#endif // __OGRE_CPREPROCESSOR_H__
