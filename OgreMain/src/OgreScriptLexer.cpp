/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreStableHeaders.h"
#include "OgreException.h"
#include "OgreScriptLexer.h"

namespace Ogre{

	ScriptLexer::ScriptLexer()
	{
	}

	ScriptTokenListPtr ScriptLexer::tokenize(const String &str, const String &source)
	{
		// State enums
		enum{ READY = 0, COMMENT, MULTICOMMENT, WORD, QUOTE, VAR, POSSIBLECOMMENT };

		// Set up some constant characters of interest
#if OGRE_WCHAR_T_STRINGS
		const wchar_t varopener = L'$', quote = L'\"', slash = L'/', backslash = L'\\', openbrace = L'{', closebrace = L'}', colon = L':', star = L'*';
		wchar_t c = 0, lastc = 0;
#else
		const wchar_t varopener = '$', quote = '\"', slash = '/', backslash = '\\', openbrace = '{', closebrace = '}', colon = ':', star = '*';
		char c = 0, lastc = 0;
#endif

		String lexeme;
		uint32 line = 1, state = READY, lastQuote = 0;
		ScriptTokenListPtr tokens(OGRE_NEW_T(ScriptTokenList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

		// Iterate over the input
		String::const_iterator i = str.begin(), end = str.end();
		while(i != end)
		{
			lastc = c;
			c = *i;

			if(c == quote)
				lastQuote = line;

			switch(state)
			{
			case READY:
				if(c == slash && lastc == slash)
				{
					// Comment start, clear out the lexeme
					lexeme = "";
					state = COMMENT;
				}
				else if(c == star && lastc == slash)
				{
					lexeme = "";
					state = MULTICOMMENT;
				}
				else if(c == quote)
				{
					// Clear out the lexeme ready to be filled with quotes!
					lexeme = c;
					state = QUOTE;
				}
				else if(c == varopener)
				{
					// Set up to read in a variable
					lexeme = c;
					state = VAR;
				}
				else if(isNewline(c))
				{
					lexeme = c;
					setToken(lexeme, line, source, tokens.get());
				}
				else if(!isWhitespace(c))
				{
					lexeme = c;
					if(c == slash)
						state = POSSIBLECOMMENT;
					else
						state = WORD;
				}
				break;
			case COMMENT:
				// This newline happens to be ignored automatically
				if(isNewline(c))
					state = READY;
				break;
			case MULTICOMMENT:
				if(c == slash && lastc == star)
					state = READY;
				break;
			case POSSIBLECOMMENT:
				if(c == slash && lastc == slash)
				{
					lexeme = "";
					state = COMMENT;
					break;	
				}
				else if(c == star && lastc == slash)
				{
					lexeme = "";
					state = MULTICOMMENT;
					break;
				}
				else
				{
					state = WORD;
				}
			case WORD:
				if(isNewline(c))
				{
					setToken(lexeme, line, source, tokens.get());
					lexeme = c;
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else if(isWhitespace(c))
				{
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else if(c == openbrace || c == closebrace || c == colon)
				{
					setToken(lexeme, line, source, tokens.get());
					lexeme = c;
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else
				{
					lexeme += c;
				}
				break;
			case QUOTE:
				if(c != backslash)
				{
					// Allow embedded quotes with escaping
					if(c == quote && lastc == backslash)
					{
						lexeme += c;
					}
					else if(c == quote)
					{
						lexeme += c;
						setToken(lexeme, line, source, tokens.get());
						state = READY;
					}
					else
					{
						// Backtrack here and allow a backslash normally within the quote
						if(lastc == backslash)
							lexeme = lexeme + "\\" + c;
						else
							lexeme += c;
					}
				}
				break;
			case VAR:
				if(isNewline(c))
				{
					setToken(lexeme, line, source, tokens.get());
					lexeme = c;
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else if(isWhitespace(c))
				{
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else if(c == openbrace || c == closebrace || c == colon)
				{
					setToken(lexeme, line, source, tokens.get());
					lexeme = c;
					setToken(lexeme, line, source, tokens.get());
					state = READY;
				}
				else
				{
					lexeme += c;
				}
				break;
			}

			// Separate check for newlines just to track line numbers
			if(isNewline(c))
				line++;
			
			i++;
		}

		// Check for valid exit states
		if(state == WORD || state == VAR)
		{
			if(!lexeme.empty())
				setToken(lexeme, line, source, tokens.get());
		}
		else
		{
			if(state == QUOTE)
			{
				OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
					Ogre::String("no matching \" found for \" at line ") + 
						Ogre::StringConverter::toString(lastQuote),
					"ScriptLexer::tokenize");
			}
		}

		return tokens;
	}

	void ScriptLexer::setToken(const Ogre::String &lexeme, Ogre::uint32 line, const String &source, Ogre::ScriptTokenList *tokens)
	{
#if OGRE_WCHAR_T_STRINGS
		const wchar_t newline = L'\n', openBracket = L'{', closeBracket = L'}', colon = L':', 
			quote = L'\"', var = L'$';
#else
		const char newline = '\n', openBracket = '{', closeBracket = '}', colon = ':', 
			quote = '\"', var = '$';
#endif

		ScriptTokenPtr token(OGRE_NEW_T(ScriptToken, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
		token->lexeme = lexeme;
		token->line = line;
		token->file = source;
		bool ignore = false;

		// Check the user token map first
		if(lexeme.size() == 1 && lexeme[0] == newline)
		{
			token->type = TID_NEWLINE;
			if(!tokens->empty() && tokens->back()->type == TID_NEWLINE)
				ignore = true;
		}
		else if(lexeme.size() == 1 && lexeme[0] == openBracket)
			token->type = TID_LBRACKET;
		else if(lexeme.size() == 1 && lexeme[0] == closeBracket)
			token->type = TID_RBRACKET;
		else if(lexeme.size() == 1 && lexeme[0] == colon)
			token->type = TID_COLON;
		else if(lexeme[0] == var)
			token->type = TID_VARIABLE;
		else
		{
			// This is either a non-zero length phrase or quoted phrase
			if(lexeme.size() >= 2 && lexeme[0] == quote && lexeme[lexeme.size() - 1] == quote)
			{
				token->type = TID_QUOTE;
			}
			else
			{
				token->type = TID_WORD;
			}
		}

		if(!ignore)
			tokens->push_back(token);
	}

	bool ScriptLexer::isWhitespace(Ogre::String::value_type c) const
	{
#ifdef OGRE_WCHAR_T_STRINGS
		return c == L' ' || c == L'\r' || c == L'\t';
#else
		return c == ' ' || c == '\r' || c == '\t';
#endif
	}

	bool ScriptLexer::isNewline(Ogre::String::value_type c) const
	{
#ifdef OGRE_WCHAR_T_STRINGS
		return c == L'\n';
#else
		return c == '\n';
#endif
	}

}

