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

#ifndef __ScriptLexer_H_
#define __ScriptLexer_H_

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreDataStream.h"
#include "OgreStringConverter.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** These codes represent token IDs which are numerical translations of
		specific lexemes. Specific compilers using the lexer can register their
		own token IDs which are given precedence over these built-in ones.
	*/
	enum{
		TID_LBRACKET = 0, // {
		TID_RBRACKET, // }
		TID_COLON, // :
		TID_VARIABLE, // $...
		TID_WORD, // *
		TID_QUOTE, // "*"
		TID_NEWLINE, // \n
		TID_UNKNOWN,
		TID_END
	};

	/** This struct represents a token, which is an ID'd lexeme from the
		parsing input stream.
	*/
	struct ScriptToken
	{
		/// This is the lexeme for this token
		String lexeme, file;
		/// This is the id associated with the lexeme, which comes from a lexeme-token id mapping
		uint32 type;
		/// This holds the line number of the input stream where the token was found.
		uint32 line;
	};
	typedef SharedPtr<ScriptToken> ScriptTokenPtr;
	typedef vector<ScriptTokenPtr>::type ScriptTokenList;
	typedef SharedPtr<ScriptTokenList> ScriptTokenListPtr;

	class _OgreExport ScriptLexer : public ScriptCompilerAlloc
	{
	public:
		ScriptLexer();
		virtual ~ScriptLexer() {}

		/** Tokenizes the given input and returns the list of tokens found */
		ScriptTokenListPtr tokenize(const String &str, const String &source);
	private: // Private utility operations
		void setToken(const String &lexeme, uint32 line, const String &source, ScriptTokenList *tokens);
		bool isWhitespace(Ogre::String::value_type c) const;
		bool isNewline(Ogre::String::value_type c) const;
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
