/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreCompiler2Pass.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringConverter.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    // instantiate static members
    Compiler2Pass::TokenState Compiler2Pass::mBNFTokenState;
    Compiler2Pass::TokenStateContainer Compiler2Pass::mClientTokenStates;
    //-----------------------------------------------------------------------
    Compiler2Pass::Compiler2Pass()
        : mActiveTokenState(&mBNFTokenState)
        , mSource(0)
        , mSourceName("system")
    {
	    // reserve some memory space in the containers being used
	    mBNFTokenState.tokenQue.reserve(100);
        mBNFTokenState.lexemeTokenDefinitions.reserve(50);

        initBNFCompiler();
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::initBNFCompiler(void)
    {
        if (mBNFTokenState.lexemeTokenDefinitions.empty())
        {
            /* Every Token ID must be manually generated during the compiler bootstrap phase
               since the rule base is manually defined.
           */

            addLexemeToken("UNKNOWN", BNF_UNKOWN);
            addLexemeToken("syntax", BNF_SYNTAX);
            addLexemeToken("rule", BNF_RULE);
            addLexemeToken("identifier", BNF_IDENTIFIER);
            addLexemeToken("identifier_right", BNF_IDENTIFIER_RIGHT);
            addLexemeToken("identifier_characters", BNF_IDENTIFIER_CHARACTERS);
            addLexemeToken("<", BNF_ID_BEGIN, false, true);
            addLexemeToken(">", BNF_ID_END, false, true);
            addLexemeToken("<#", BNF_CONSTANT_BEGIN, false, true);
            addLexemeToken("::=", BNF_SET_RULE, false, true);
            addLexemeToken("expression", BNF_EXPRESSION);
            addLexemeToken("and_term", BNF_AND_TERM);
            addLexemeToken("or_term", BNF_OR_TERM);
            addLexemeToken("term", BNF_TERM);
            addLexemeToken("term_id", BNF_TERM_ID);
            addLexemeToken("constant", BNF_CONSTANT);
            addLexemeToken("|", BNF_OR, false, true);
            addLexemeToken("terminal_symbol", BNF_TERMINAL_SYMBOL);
            addLexemeToken("terminal_start", BNF_TERMINAL_START);
            addLexemeToken("repeat_expression", BNF_REPEAT_EXPRESSION);
            addLexemeToken("not_expression", BNF_NOT_EXPRESSION);
            addLexemeToken("{", BNF_REPEAT_BEGIN, false, true);
            addLexemeToken("}", BNF_REPEAT_END, false, true);
            addLexemeToken("set", BNF_SET);
            addLexemeToken("(", BNF_SET_BEGIN, false, true);
            addLexemeToken(")", BNF_SET_END, false, true);
            addLexemeToken("set_end_exc", BNF_SET_END_EXC);
            addLexemeToken("optional_expression", BNF_OPTIONAL_EXPRESSION);
            addLexemeToken("[", BNF_OPTIONAL_BEGIN, false, true);
            addLexemeToken("]", BNF_OPTIONAL_END, false, true);
            addLexemeToken("not_test", BNF_NOT_TEST);
            addLexemeToken("not_chk", BNF_NOT_CHK);
            addLexemeToken("(?!", BNF_NOT_TEST_BEGIN, false, true);
            addLexemeToken("'", BNF_SINGLEQUOTE, false, true);
            addLexemeToken(":", BNF_CONDITIONAL_TOKEN_INSERT, false, true);
            addLexemeToken("-'", BNF_NO_TOKEN_START, false, true);
            addLexemeToken("any_character", BNF_ANY_CHARACTER);
            addLexemeToken("single_quote_exc", BNF_SINGLE_QUOTE_EXC);
            addLexemeToken("white_space_chk", BNF_WHITE_SPACE_CHK);
            addLexemeToken("special_characters1", BNF_SPECIAL_CHARACTERS1);
            addLexemeToken("special_characters2", BNF_SPECIAL_CHARACTERS2);

            addLexemeToken("letter", BNF_LETTER);
            addLexemeToken("letter_digit", BNF_LETTER_DIGIT);
            addLexemeToken("digit", BNF_DIGIT);
            addLexemeToken("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", BNF_ALPHA_SET, false, true);
            addLexemeToken("0123456789", BNF_NUMBER_SET, false, true);
            addLexemeToken("`~!@#$%^&*(-_=+\\|[]{}:;\"<>,.?/\n\r\t", BNF_SPECIAL_CHARACTER_SET2, false, true);
            addLexemeToken("$_", BNF_SPECIAL_CHARACTER_SET1, false, true);
            addLexemeToken(" ", BNF_WHITE_SPACE, false, true);
            addLexemeToken("?!", BNF_NOT_CHARS, false, true);
        }

        if (mBNFTokenState.rootRulePath.empty())
        {
            // first entry is set to unknown in order to trap rule id's not set for non-terminal tokens
            mBNFTokenState.rootRulePath.resize(1);
            //  used by bootstrap BNF text parser
            //  <>	- non-terminal token
            //  ()  - set of
            // ::=	- rule definition
            #define _rule_(id)	    mBNFTokenState.rootRulePath.push_back(TokenRule(otRULE, id));
            #define _is_(id)	    mBNFTokenState.rootRulePath.push_back(TokenRule(otAND, id));
            //     - blank space is an implied "AND" meaning the token is required
            #define _and_(id)		mBNFTokenState.rootRulePath.push_back(TokenRule(otAND, id));
            // |	- or
            #define _or_(id)		mBNFTokenState.rootRulePath.push_back(TokenRule(otOR,	id));
            // []	- optional
            #define _optional_(id)	mBNFTokenState.rootRulePath.push_back(TokenRule(otOPTIONAL, id));
            // {}	- repeat 0 or more times until fail or rule does not progress
            #define _repeat_(id)	mBNFTokenState.rootRulePath.push_back(TokenRule(otREPEAT,	id));
            #define _data_(id)      mBNFTokenState.rootRulePath.push_back(TokenRule(otDATA, id));
            // (?! ) - lookahead negative (not test)
            #define _not_(id)       mBNFTokenState.rootRulePath.push_back(TokenRule(otNOT_TEST, id));
            #define _end_   		mBNFTokenState.rootRulePath.push_back(TokenRule(otEND,0));

            // <syntax>     ::=  { rule }
            _rule_(BNF_SYNTAX) _repeat_(BNF_RULE) _end_

            // <rule>       ::=  <identifier>  "::="  <expression>
            _rule_(BNF_RULE)
                _is_(BNF_IDENTIFIER)
                _and_(BNF_SET_RULE)
                _and_(BNF_EXPRESSION)
            _end_

            // <expression> ::=  <and_term> { <or_term> }
            _rule_(BNF_EXPRESSION)
                _is_(BNF_AND_TERM)
                _repeat_(BNF_OR_TERM)
            _end_
            // <or_term>    ::=  "|" <and_term>
            _rule_(BNF_OR_TERM)
                _is_(BNF_OR)
                _and_(BNF_AND_TERM)
            _end_
            // <and_term>   ::=  <term> { <term> }
            _rule_(BNF_AND_TERM)
                _is_(BNF_TERM)
                _repeat_(BNF_TERM)
            _end_
            // <term>       ::=  <term_id> | <repeat_expression> | <optional_expression> | <not_expression>
            _rule_(BNF_TERM)
                _is_(BNF_TERM_ID)
                _or_(BNF_REPEAT_EXPRESSION)
                _or_(BNF_OPTIONAL_EXPRESSION)
                _or_(BNF_NOT_EXPRESSION)
            _end_

            // <term_id>    ::= <constant> | <identifier_right> | <terminal_symbol> | <set>
            _rule_(BNF_TERM_ID)
                _is_(BNF_CONSTANT)
                _or_(BNF_IDENTIFIER_RIGHT)
                _or_(BNF_TERMINAL_SYMBOL)
                _or_(BNF_SET)
            _end_

            // <repeat_expression> ::=  "{"  <term_id>  "}"
            _rule_(BNF_REPEAT_EXPRESSION)
                _is_(BNF_REPEAT_BEGIN)
                _and_(BNF_TERM_ID)
                _and_(BNF_REPEAT_END)
            _end_

            // <optional_expression> ::= "["  <term_id>  "]"
            _rule_(BNF_OPTIONAL_EXPRESSION)
                _is_(BNF_OPTIONAL_BEGIN)
                _and_(BNF_TERM_ID)
                _and_(BNF_OPTIONAL_END)
            _end_

            // <not_expression> ::= "(?!" <term_id> ")"
            _rule_(BNF_NOT_EXPRESSION)
                _is_(BNF_NOT_TEST_BEGIN)
                _and_(BNF_TERM_ID)
                _and_(BNF_SET_END)
            _end_

            // <identifier_right> ::= <indentifier> (?!"::=")
            _rule_(BNF_IDENTIFIER_RIGHT)
                _is_(BNF_IDENTIFIER)
                _not_(BNF_SET_RULE)
            _end_

            // <identifier> ::=  "<" <letter> {<identifier_characters>} ">"
            _rule_(BNF_IDENTIFIER)
                _is_(BNF_ID_BEGIN)
                _and_(BNF_LETTER)
                _repeat_(BNF_IDENTIFIER_CHARACTERS)
                _and_(BNF_ID_END)
            _end_

            // <identifier_characters> ::= <letter_digit> | <special_characters1>
            _rule_(BNF_IDENTIFIER_CHARACTERS)
                _is_(BNF_LETTER_DIGIT)
                _or_(BNF_SPECIAL_CHARACTERS1)
            _end_

            // <terminal_symbol> ::= <terminal_start> @{ <any_character> } "'" [":"]
            _rule_(BNF_TERMINAL_SYMBOL)
                _is_(BNF_TERMINAL_START)
                _and_(_no_space_skip_)
                _repeat_(BNF_ANY_CHARACTER)
                _and_(BNF_SINGLEQUOTE)
                _optional_(BNF_CONDITIONAL_TOKEN_INSERT)
            _end_

            // <terminal_start> ::= "-'" | "'"
            _rule_(BNF_TERMINAL_START)
                _is_(BNF_NO_TOKEN_START)
                _or_(BNF_SINGLEQUOTE)
            _end_


            // <constant> ::= "<#" <letter> {<identifier_characters>} ">"
            _rule_(BNF_CONSTANT)
                _is_(BNF_CONSTANT_BEGIN)
                _and_(BNF_LETTER)
                _repeat_(BNF_IDENTIFIER_CHARACTERS)
                _and_(BNF_ID_END)
            _end_

            // <set> ::= "(" (?!<not_chk>) @{<any_character>} ")"
            _rule_(BNF_SET)
                _is_(BNF_SET_BEGIN)
                _not_(BNF_NOT_CHK)
                _and_(_no_space_skip_)
                _repeat_(BNF_ANY_CHARACTER)
                _and_(BNF_SET_END)
            _end_

            // <any_character> ::= <letter_digit> | <special_characters2>
            _rule_(BNF_ANY_CHARACTER)
                _is_(BNF_LETTER_DIGIT)
                _or_(BNF_SPECIAL_CHARACTERS2)
            _end_

			// <letter_digit> ::= <letter> | <digit>
            _rule_(BNF_LETTER_DIGIT)
                _is_(BNF_LETTER)
                _or_(BNF_DIGIT)
            _end_

            // <letter> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ)
            _rule_(BNF_LETTER)
                _is_(_character_)
                _data_(BNF_ALPHA_SET)// "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
            _end_

            // <digit> ::= (0123456789)
            _rule_(BNF_DIGIT)
                _is_(_character_)
                _data_(BNF_NUMBER_SET)
            _end_

            // <special_characters1> ::= ($_)
            _rule_(BNF_SPECIAL_CHARACTERS1)
                _is_(_character_)
                _data_(BNF_SPECIAL_CHARACTER_SET1)
            _end_

            // <special_characters2> ::= (`~!@#$%^&*(-_=+\|[]{}:;"<>,.?/) | <single_quote_exc>
            //                           | <white_space_chk> | <set_end_exc>
            _rule_(BNF_SPECIAL_CHARACTERS2)
                _is_(_character_)
                _data_(BNF_SPECIAL_CHARACTER_SET2)
                _or_(BNF_WHITE_SPACE_CHK)
                _or_(BNF_SINGLE_QUOTE_EXC)
                _or_(BNF_SET_END_EXC)
            _end_

            // <single_quote_exc> ::= "'" (?!" ")
            _rule_(BNF_SINGLE_QUOTE_EXC)
                _is_(_character_)
                _data_(BNF_SINGLEQUOTE)
                _not_(BNF_WHITE_SPACE_CHK)
            _end_

            // <set_end_exc> ::= ")" (?!" ")
            _rule_(BNF_SET_END_EXC)
                _is_(_character_)
                _data_(BNF_SET_END)
                _not_(BNF_WHITE_SPACE_CHK)
            _end_

            // <white_space_chk> ::= ( )
            _rule_(BNF_WHITE_SPACE_CHK)
                _is_(_character_)
                _data_(BNF_WHITE_SPACE)
            _end_
            // <not_chk> ::= (?!)
            _rule_(BNF_NOT_CHK)
                _is_(BNF_NOT_CHARS)
                //_data_(BNF_NOT_CHARS)
            _end_

            // now that all the rules are added, update token definitions with rule links
            verifyTokenRuleLinks("system");
        }
        // switch to client state
        mActiveTokenState = mClientTokenState;
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::verifyTokenRuleLinks(const String& grammerName)
    {
	    size_t token_ID;

	    // scan through all the rules and initialize index to rules for non-terminal tokens
        const size_t ruleCount = mActiveTokenState->rootRulePath.size();
        for (size_t i = 0; i < ruleCount; ++i)
        {
		   // make sure token definition holds valid token
		   if (mActiveTokenState->rootRulePath[i].operation == otRULE)
		   {
    		   token_ID = mActiveTokenState->rootRulePath[i].tokenID;
    		   // system token id's can never have a rule assigned to them so no need to check if token is system token
    		   // but do make sure the id is within defined bounds
               if (token_ID >= mActiveTokenState->lexemeTokenDefinitions.size())
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "For grammer: " + grammerName +
                        ", a token ID was out of token definition range.",
                        "Compiler2Pass::verifyTokenRuleLinks");

               LexemeTokenDef& tokenDef = mActiveTokenState->lexemeTokenDefinitions[token_ID];
        	   if (tokenDef.ID != token_ID)
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "For grammer: " + grammerName +
                        ", lexeme non-terminal token definition: " +
                        tokenDef.lexeme + " is corrupted and does not match its assigned rule.",
                        "Compiler2Pass::verifyTokenRuleLinks");
			   // if operation is a rule then update token definition
               tokenDef.ruleID = i;
               tokenDef.isNonTerminal = true;
		   }
	    } // end for

	    // test all non terminals for valid rule ID
        const size_t definitionCount = mActiveTokenState->lexemeTokenDefinitions.size();
        bool errorsFound = false;
        // report all non-terminals that don't have a rule then throw an exception
        for (token_ID = 0; token_ID < definitionCount; ++token_ID)
        {
            const LexemeTokenDef& tokenDef = mActiveTokenState->lexemeTokenDefinitions[token_ID];
            if (tokenDef.isNonTerminal && (tokenDef.ruleID == 0))
            {
                errorsFound = true;
                LogManager::getSingleton().logMessage(
                "For grammer: " + grammerName +
                ", lexeme non-terminal token definition: " + tokenDef.lexeme +
                " found with no rule definition or corrupted."
                );
            }
        }
        if (errorsFound)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,  "For grammer: " + grammerName +
                ", lexeme non-terminal token definition(s) found with no rule definition or corrupted.",
                "Compiler2Pass::verifyTokenRuleLinks");
        }
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::compile(const String& source, const String& sourceName)
    {
        // make sure BNF compiler is setup to compile BNF grammer if required
        initBNFCompiler();
        // compile the client's BNF grammer
        setClientBNFGrammer();

	    bool Passed = false;

	    mSource = &source;
	    mSourceName = sourceName;
        mActiveTokenState = mClientTokenState;
	    // start compiling if there is a rule base to work with
        if (mActiveTokenState->rootRulePath.size() > 1)
	    {
		    Passed = doPass1();

		    if (Passed)
		    {
			    Passed = doPass2();
		    }
	    }

	    return Passed;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::doPass1()
    {
	    // scan through Source string and build a token list using TokenInstructions
	    // this is a simple brute force lexical scanner/analyzer that also parses the formed
	    // token for proper semantics and context in one pass

	    mCurrentLine = 1;
	    mCharPos = 0;
	    // reset position in Constants container
	    mConstants.clear();
        mLabels.clear();
        // there is no active label when first starting pass 1
        mLabelIsActive = false;
        mActiveLabelKey = 0;
        mEndOfSource = mSource->length();

	    // start with a clean slate
	    mActiveTokenState->tokenQue.clear();
	    mPass2TokenQuePosition = 0;
	    mPreviousActionQuePosition = 0;
	    mNextActionQuePosition = 0;
	    mNoTerminalToken = false;
	    mNoSpaceSkip = false;
	    mErrorCharPos = 0;
	    mInsertTokenID = 0;
	    // tokenize and check semantics untill an error occurs or end of source is reached
	    // assume RootRulePath has pointer to rules so start at index + 1 for first rule path
	    // first rule token would be a rule definition so skip over it
	    bool passed = false;

	    try
	    {
            passed = processRulePath(1);
            // if a lexeme in source still exists then the end of source was not reached and there was a problem some where
            if (positionToNextLexeme()) passed = false;
            if (passed)
            {
                // special condition at end of script.  The last action needs to be triggered if
                // parsing reached the end of the source.
                activatePreviousTokenAction();
            }
            else if (mCharPos != mEndOfSource && mErrorCharPos == 0)
            {
                LogManager::getSingleton().logMessage(
                    "*** ERROR *** : in " + getClientGrammerName() +
                    " Source: " + mSourceName +
                    "\nUnknown token found on line " + StringConverter::toString(mCurrentLine) +
                    "\nFound: >>>" + mSource->substr(mCharPos, 20) +
                    "<<<\n"
                );

            }

	    }
	    catch (Exception& e)
	    {
            LogManager::getSingleton().logMessage( "Exception caught "
                " while trying to parse "
                + getClientGrammerName()
                + ": "
                + mSourceName
                + ". Exception was '"
				+ e.getFullDescription()
				+ "'. Pass 1 terminated"
            );
	    }
	    catch (...)
	    {
            LogManager::getSingleton().logMessage( "Unkown exception while trying to parse: "
            + getClientGrammerName()
            + ": "
            + mSourceName
            );
	    }

	    return passed;

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::doPass2()
    {
        bool passed = true;
        // step through tokens container and execute until end found or error occurs

        return passed;
    }
    //-----------------------------------------------------------------------
    const Compiler2Pass::TokenInst& Compiler2Pass::getCurrentToken(const size_t expectedTokenID) const
    {
        if (mPass2TokenQuePosition <= mActiveTokenState->tokenQue.size() - 1)
        {
            const TokenInst& tokenInst = mActiveTokenState->tokenQue[mPass2TokenQuePosition];

            if (expectedTokenID > 0 && (tokenInst.tokenID != expectedTokenID))
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, getClientGrammerName() + ":" + mSourceName
                    + ", expected token ID not found" ,
                    "Compiler2Pass::getCurrentToken");
            }

            return tokenInst;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, getClientGrammerName() + ":" + mSourceName +
                ", Line " + StringConverter::toString(mActiveTokenState->tokenQue.back().line) +
                "\n no token available, all pass 2 tokens processed" ,
                "Compiler2Pass::getCurrentToken");
        }
    }
    //-----------------------------------------------------------------------
    bool Compiler2Pass::testNextTokenID(const size_t expectedTokenID) const
    {
        const size_t nextTokenIndex = mPass2TokenQuePosition + 1;

        if (nextTokenIndex < mActiveTokenState->tokenQue.size())
            return mActiveTokenState->tokenQue[nextTokenIndex].tokenID == expectedTokenID;

        return false;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::skipToken(void) const
    {
        if (mPass2TokenQuePosition < mActiveTokenState->tokenQue.size() - 1)
        {
            ++mPass2TokenQuePosition;
        }
        else
        {
            // no more tokens left for pass 2 processing
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, getClientGrammerName() + ":" + mSourceName
                + ", no more tokens available for pass 2 processing" ,
                "Compiler2Pass::skipToken");
        }
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::replaceToken(void)
    {
        // move instruction que index back one position
        if (mPass2TokenQuePosition > 0)
            --mPass2TokenQuePosition;
    }
    //-----------------------------------------------------------------------
    float Compiler2Pass::getCurrentTokenValue(void) const
    {
        // get float value from current token instruction
        const TokenInst& token = getCurrentToken();
        if ( token.tokenID == _value_)
        {
            std::map<size_t, float>::const_iterator i = mConstants.find(mPass2TokenQuePosition);
            if (i != mConstants.end())
            {
                return i->second;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "In " + mSourceName +
                    ", on line " + StringConverter::toString(token.line) +
                    ", no value was found in : >>>" + mSource->substr(token.pos, 20) +
                            "<<<",
                    "Compiler2Pass::getCurrentTokenValue");
            }
        }
        else
        {
            // if token is not for a value then throw an exception
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "In " + mSourceName +
                ", on line " + StringConverter::toString(token.line) +
                ", token is not for a value.  Found: >>>" + mSource->substr(token.pos, 20) +
                        "<<<",
                "Compiler2Pass::getCurrentTokenValue");
        }
    }
    //-----------------------------------------------------------------------
    const String& Compiler2Pass::getCurrentTokenLabel(void) const
    {
        // get label from current token instruction
        const TokenInst& token = getCurrentToken();
        if (token.tokenID == _character_)
        {
            std::map<size_t, String>::const_iterator i = mLabels.find(mPass2TokenQuePosition);
            if (i != mLabels.end())
            {
                return i->second;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "In " + mSourceName +
                    ", on line " + StringConverter::toString(token.line) +
                    ", no Label was found in : >>>" + mSource->substr(token.pos, 20) +
                            "<<<",
                    "Compiler2Pass::getCurrentTokenLabel");
            }
        }
        else
        {
            // if token is not for a label then throw an exception
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "In " + mSourceName +
                ", on line " + StringConverter::toString(token.line) +
                ", token is not for a label.  Found: >>>" + mSource->substr(token.pos, 20) +
                        "<<<",
                "Compiler2Pass::getCurrentTokenLabel");
        }
    }
    //-----------------------------------------------------------------------
    const String& Compiler2Pass::getCurrentTokenLexeme(void) const
    {
        // get label from current token instruction
        const TokenInst& token = getCurrentToken();
        if (token.tokenID < SystemTokenBase)
            return mActiveTokenState->lexemeTokenDefinitions[token.tokenID].lexeme;
        else
        {
            // if token is for system use then throw an exception
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "In " + mSourceName +
                ", on line " + StringConverter::toString(token.line) +
                ", token is for system use only.  Found: >>>" + mSource->substr(token.pos, 20) +
                        "<<<",
                "Compiler2Pass::getCurrentTokenLexeme");
        }

    }
    //-----------------------------------------------------------------------
    size_t Compiler2Pass::getPass2TokenQueCount(void) const
    {
        // calculate number of tokens between current token instruction and next token with action
        if(mActiveTokenState->tokenQue.size() > mPass2TokenQuePosition)
            return mActiveTokenState->tokenQue.size() - 1 - mPass2TokenQuePosition;
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    size_t Compiler2Pass::getRemainingTokensForAction(void) const
    {
        size_t remaingingTokens = 0;
        if (mNextActionQuePosition > mPass2TokenQuePosition)
        {
            // don't count next action nor the current position
            remaingingTokens = mNextActionQuePosition - mPass2TokenQuePosition - 1;
        }

        return remaingingTokens;
    }
    //-----------------------------------------------------------------------
    bool Compiler2Pass::setNextActionQuePosition(size_t pos, const bool search)
    {
        const size_t lastPos = mActiveTokenState->tokenQue.size();

        if (pos >= lastPos)
            return false;

        bool nextActionFound = false;

        // if searching then assume no next action will be found so set position to end of que
        if (search)
            mNextActionQuePosition = lastPos;

        while (!nextActionFound && (pos < lastPos))
        {
            const size_t tokenID = mActiveTokenState->tokenQue[pos].tokenID;

            if ((tokenID < SystemTokenBase) &&
                mActiveTokenState->lexemeTokenDefinitions.at(tokenID).hasAction)
            {
                mNextActionQuePosition = pos;
                nextActionFound = true;
            }

            if (search)
                ++pos;
            else
                pos = lastPos;
        }

        return nextActionFound;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::setPass2TokenQuePosition(size_t pos, const bool activateAction)
    {
        if (pos < mActiveTokenState->tokenQue.size())
        {
            mPass2TokenQuePosition = pos;
            ++pos;
            // find the next token with an action
            setNextActionQuePosition(pos, true);

            // activate action if token has one and it was requested
            if (activateAction)
            {
                const size_t tokenID = mActiveTokenState->tokenQue.at(mPass2TokenQuePosition).tokenID;
                if ((tokenID < SystemTokenBase) &&
                    mActiveTokenState->lexemeTokenDefinitions.at(tokenID).hasAction)
                {
                    // assume that pass 2 processing will use tokens downstream
                    executeTokenAction(tokenID);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::setClientBNFGrammer(void)
    {
        // switch to internal BNF Containers
        // clear client containers
        mClientTokenState = &mClientTokenStates[getClientGrammerName()];
        // attempt to compile the grammer into a rule base if no rules exist
        if (mClientTokenState->rootRulePath.size() == 0)
        {
            mClientTokenState->tokenQue.reserve(100);
            mClientTokenState->lexemeTokenDefinitions.reserve(100);
            // first entry in rule path is set as a bad entry and no token should reference it
            mClientTokenState->rootRulePath.resize(1);
            // allow the client to setup token definitions prior to
            // compiling the BNF grammer
            // ensure token definitions are added to the client state
            mActiveTokenState = mClientTokenState;
            // get client to setup token definitions and actions it wants to know about
            setupTokenDefinitions();
            // make sure active token state is for BNF compiling
            mActiveTokenState = &mBNFTokenState;
            mSource = &getClientBNFGrammer();

            if (doPass1())
            {
                buildClientBNFRulePaths();
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "BNF Grammar compilation failed for " +
                    getClientGrammerName(), "Compiler2Pass::setClientBNFGrammer");
            }
            // change token state to client data after compiling grammer
            mActiveTokenState = mClientTokenState;
            // verify the client rule paths and associated terminal and non-terminal lexemes
            verifyTokenRuleLinks(getClientGrammerName());
        }
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::processRulePath( size_t rulepathIDX)
    {
	    // rule path determines what tokens and therefore what lexemes are acceptable from the source
	    // it is assumed that the tokens with the longest similar lexemes are arranged first so
	    // if a match is found it is accepted and no further searching is done

        if (rulepathIDX >= mActiveTokenState->rootRulePath.size())
        {
            // This is very bad and no way to recover so raise exception
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "rule ID exceeds rule base bounds.", "Compiler2Pass::processRulePath");
        }
	    // record position of last token in container
	    // to be used as the rollback position if a valid token is not found
        const size_t TokenContainerOldSize = mActiveTokenState->tokenQue.size();
	    const size_t OldCharPos = mCharPos;
	    const size_t OldLinePos = mCurrentLine;
        const bool OldLabelIsActive = mLabelIsActive;
        const size_t OldActiveLabelKey = mActiveLabelKey;
        const String OldLabel = mLabels[OldActiveLabelKey];

	    // keep track of what non-terminal token activated the rule
	    size_t ActiveNTTRule = mActiveTokenState->rootRulePath[rulepathIDX].tokenID;
	    // start rule path at next position for definition
	    ++rulepathIDX;

	    // assume the rule will pass
	    bool passed = true;
        bool tokenFound = false;
	    bool endFound = false;
	    bool clearInsertTokenID = false;

	    // keep following rulepath until the end of the rule or the end of the source is reached
        while (!(endFound || isEndOfSource()))
	    {
		    switch (mActiveTokenState->rootRulePath[rulepathIDX].operation)
		    {

		    case otAND:
			    // only validate if the previous rule passed
			    if (passed)
				    passed = ValidateToken(rulepathIDX, ActiveNTTRule);
                // log error message if a previouse token was found in this rule path and current token failed
                if (tokenFound && (mCharPos != mErrorCharPos) && !passed)
                {
                    mErrorCharPos = mCharPos;
                    LogManager::getSingleton().logMessage(
                    "*** ERROR *** : in " + getClientGrammerName() +
                    " Source: " + mSourceName +
                    "\nUnknown token found on line " + StringConverter::toString(mCurrentLine) +
                    "\nFound: >>>" + mSource->substr(mCharPos, 20) +
                    "<<<\nbut was expecting form: " + getBNFGrammerTextFromRulePath(rulepathIDX, 2) +
                    "\nwhile in rule path: <" + mActiveTokenState->lexemeTokenDefinitions[ActiveNTTRule].lexeme +
                    ">"
                    );
                    // log last valid token found
                    const TokenInst& tokenInst = mActiveTokenState->tokenQue.back();
                    LogManager::getSingleton().logMessage(
                        "Last valid token found was on line " + StringConverter::toString(tokenInst.line));
                    LogManager::getSingleton().logMessage(
                        "source hint: >>>" + mSource->substr(tokenInst.pos, 20) + "<<<");
                }

			    break;

		    case otOR:
			    // only validate if the previous rule failed
			    if ( passed == false )
			    {
				    // clear previous tokens from entry and try again
				    mActiveTokenState->tokenQue.resize(TokenContainerOldSize);
				    passed = ValidateToken(rulepathIDX, ActiveNTTRule);
			    }
			    else
			    {
				    // path passed up to this point therefore finished so pretend end marker found
				    endFound = true;
			    }
			    break;

		    case otOPTIONAL:
			    // if previous passed then try this rule but it does not effect succes of rule since its optional
			    if(passed) ValidateToken(rulepathIDX, ActiveNTTRule);
			    break;

		    case otREPEAT:
			    // repeat until called rule fails or cursor does not advance
			    // repeat is 0 or more times
			    if (passed)
			    {
				    // keep calling until failure or no change in cursor position
            	    size_t prevPos = mCharPos;
				    while ( ValidateToken(rulepathIDX, ActiveNTTRule))
				    {
                        if (mCharPos > prevPos)
                        {
                            prevPos = mCharPos;
                        }
                        else
                        {
                            // repeat failed to advance the cursor position so time to quit since the repeating rule
                            // path isn't finding anything
                            // this can happen if the rule being called only has _optional_ rules
                            // this checking of the cursor positions prevents infinite loop from occuring
                            break;
                        }
				    }
			    }
			    break;

            case otDATA:
                // skip it, should have been handled by previous operation.
                break;

            case otNOT_TEST:
			    // only validate if the previous rule passed
			    if (passed)
                {

                    // perform look ahead and test if rule production fails
                    const size_t la_TokenContainerOldSize = mActiveTokenState->tokenQue.size();
	                const size_t la_OldCharPos = mCharPos;
	                const size_t la_OldLinePos = mCurrentLine;
	                const bool la_OldLabelIsActive = mLabelIsActive;
	                const size_t la_OldActiveLabelKey = mActiveLabelKey;
	                const String la_OldLabel = mLabels[la_OldActiveLabelKey];

                    passed = !ValidateToken(rulepathIDX, ActiveNTTRule);

                    // only wanted to take a peek as to what was ahead so now restore back to current position
			        mActiveTokenState->tokenQue.resize(la_TokenContainerOldSize);
			        mCharPos = la_OldCharPos;
			        mCurrentLine = la_OldLinePos;
			        // restor label state if it was active before not test
			        if (la_OldLabelIsActive)
			        {
                        mActiveLabelKey = la_OldActiveLabelKey;
                        mLabelIsActive = la_OldLabelIsActive;
                        mActiveLabel = &mLabels[mActiveLabelKey];
                        *mActiveLabel = la_OldLabel;
			        }
                    // only perform full rollback if tokens found
			        if (!passed)
			        {
				        // the rule did not validate so get rid of tokens decoded
				        // roll back the token container end position to what it was when rule started
				        // this will get rid of all tokens that had been pushed on the container while
				        // trying to validating this rule
				        mActiveTokenState->tokenQue.resize(TokenContainerOldSize);
				        //mConstants.resize(OldConstantsSize);
				        mCharPos = OldCharPos;
				        mCurrentLine = OldLinePos;
                        // restor label state if it was active before not test
                        if (OldLabelIsActive)
                        {
                            mActiveLabelKey = OldActiveLabelKey;
                            mLabelIsActive = OldLabelIsActive;
                            mActiveLabel = &mLabels[mActiveLabelKey];
                            *mActiveLabel = OldLabel;
                        }

                        // terminate rule production processing
        			    endFound = true;
			        }
                }
                break;

            case otINSERT_TOKEN:
                mInsertTokenID = mActiveTokenState->rootRulePath[rulepathIDX].tokenID;
                clearInsertTokenID = true;
                break;

		    case otEND:
			    // end of rule found so time to return
			    endFound = true;
                // only rollback if no tokens found
			    if (!passed && !tokenFound)
			    {
				    // the rule did not validate so get rid of tokens decoded
				    // roll back the token container end position to what it was when rule started
				    // this will get rid of all tokens that had been pushed on the container while
				    // trying to validating this rule
				    mActiveTokenState->tokenQue.resize(TokenContainerOldSize);
				    //mConstants.resize(OldConstantsSize);
				    mCharPos = OldCharPos;
				    mCurrentLine = OldLinePos;
			    }
			    else
			    {
			        // if the rule path was partially completed, one or more tokens found then mark it as passed
                    if (!passed && tokenFound && !mLabelIsActive)
                    {
                        passed = true;
                    }
			    }
			    break;

		    default:
			    // an exception should be raised since the code should never get here
			    passed = false;
			    endFound = true;
			    break;

		    } // end switch
            // prevent rollback from occuring if a token was found but later part of rule fails
            // this allows pass2 to either fix the problem or report the error and continue on.
            // Don't do this for _no_token_ since its a special system token and has nothing todo with
            // a successfull parse of the source.  Can check this by looking at mNoTerminalToken state.
            // if _no_token had just been validated then mNoTerminalToken will be true.
            if (passed && !mNoTerminalToken && !mInsertTokenID)
                tokenFound = true;
		    // move on to the next rule in the path
		    ++rulepathIDX;
	    } // end while
	    
        // if this rule production requested a token insert, make sure its reset so it does not affect
        // the parent rule
        if (clearInsertTokenID)
            mInsertTokenID = 0;

	    return passed;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isCharacterLabel(const size_t rulepathIDX)
    {
        if (isEndOfSource())
            return false;
            
	    // assume the test is going to fail
	    bool Passed = false;

        // get token from next rule operation.
        // token string is list of valid or invalid single characters.
        // If the token string starts with a ! then the set is for invalid characters.
        // compare character at current cursor position in script to characters in list for a match
        // if match found then add character to active label
        // _character_ will not have  a token definition but the next rule operation should be
        // DATA and have the token ID required to get the character set.
        const TokenRule& rule = mActiveTokenState->rootRulePath[rulepathIDX + 1];
        if (rule.operation == otDATA)
        {
            const size_t TokenID = rule.tokenID;
            // check for ! as first character in character set indicating that an input character is
            // accepted if its not in the character set.
            // Otherwise a pass occurs if the input character is found in the character set.
            const String& characterSet = mActiveTokenState->lexemeTokenDefinitions[TokenID].lexeme;
            if ((characterSet.size() > 1) && characterSet[0] == '!')
                Passed = characterSet.find((*mSource)[mCharPos], 1) == String::npos;
            else
                Passed = characterSet.find((*mSource)[mCharPos]) != String::npos;

            if (Passed)
            {
                // is a new label starting?
                // if mLabelIsActive is false then starting a new label so need a new mActiveLabelKey
                if (!mLabelIsActive)
                {
                    // mActiveLabelKey will be the end of the instruction container ie the size of mTokenInstructions
                    mActiveLabelKey = mActiveTokenState->tokenQue.size();
                    // if a token insert is pending then use next key
                    if (mInsertTokenID)
                        ++mActiveLabelKey;
                    mLabelIsActive = true;
                    mNoSpaceSkip = true;
                    // reset the contents of the label since it might have been used prior to a rollback
                    // and cach string location so don't have to look it up for the rest of the label processing
                    mActiveLabel = &mLabels[mActiveLabelKey];
                    mActiveLabel->clear();
                }
                // add the single character to the end of the active label
                *mActiveLabel += (*mSource)[mCharPos];
            }
        }

        return Passed;
    }
    //-----------------------------------------------------------------------
    bool Compiler2Pass::ValidateToken(const size_t rulepathIDX, const size_t activeRuleID)
    {
	    size_t tokenlength = 0;
	    // assume the test is going to fail
	    bool Passed = false;
	    size_t tokenID = mActiveTokenState->rootRulePath[rulepathIDX].tokenID;
	    // if terminal token then compare text of lexeme with what is in source
	    if ( (tokenID >= SystemTokenBase) ||
            !mActiveTokenState->lexemeTokenDefinitions[tokenID].isNonTerminal )
	    {
            if (tokenID != _character_)
            {
                mLabelIsActive = false;
                // allow spaces to be skipped for next lexeme processing
                mNoSpaceSkip = false;
            }

	        if (tokenID == _no_space_skip_)
	        {
                // don't skip spaces to get to next lexeme
                mNoSpaceSkip = true;
                // move on to next rule
                Passed = true;
	        }
	        else if (tokenID == _no_token_)
	        {
	            // turn on no terminal token processing for next rule
                mNoTerminalToken = true;
                // move on to next rule
                Passed = true;
	        }
            // if label processing is active ie previous token was _character_
            // and current token is supposed to be a _character_ then don't
            // position to next lexeme in source
		    else if (mNoSpaceSkip || positionToNextLexeme())
		    {
			    // if Token is supposed to be a number then check if its a numerical constant
			    if (tokenID == _value_)
			    {
    			    float constantvalue = 0.0f;
				    if (Passed = isFloatValue(constantvalue, tokenlength))
				    {
                        // key is the next instruction index
                        size_t key = mActiveTokenState->tokenQue.size();
                        // if a token insert is pending then use next key
                        if (mInsertTokenID)
                            ++key;
                        mConstants[key] = constantvalue;
				    }
			    }
                else // check if user label or valid keyword token
                {
                    if (tokenID == _character_)
                    {
                        if (Passed = isCharacterLabel(rulepathIDX))
                            // only one character was processed
                            tokenlength = 1;
                    }
                    else
                    {
			            // compare token lexeme text with source text
                        if (Passed = isLexemeMatch(mActiveTokenState->lexemeTokenDefinitions[tokenID].lexeme, mActiveTokenState->lexemeTokenDefinitions[tokenID].isCaseSensitive))
                        {
                            tokenlength = mActiveTokenState->lexemeTokenDefinitions[tokenID].lexeme.length();
                            // check if terminal token should be ignored ie not put in instruction que
                            if (mNoTerminalToken)
                                tokenID = _no_token_;
                        }
                        // always clear no terminal token flag.  it only works for one pending terminal token.
                        mNoTerminalToken = false;
                    }
                }

                // turn off label processing if token ID was not for _character_
                if (tokenID == _character_)
                {
                    // _character_ token being processed
                    // turn off generation of a new token instruction if this is not
                    // the first _character_ in a sequence of _character_ terminal tokens.
                    // Only want one _character_ token which Identifies a label

                    if (mActiveTokenState->tokenQue.size() > mActiveLabelKey)
                    {
                        // this token is not the first _character_ in the label sequence
                        // so turn off the token by turning TokenID into _no_token_
                        tokenID = _no_token_;
                    }
                }

                // if valid terminal token was found then add it to the instruction container for pass 2 processing
			    if (Passed)
			    {
                    if (tokenID != _no_token_)
                    {
				        TokenInst newtoken;
				        // push token onto end of container
				        newtoken.NTTRuleID = activeRuleID;
				        newtoken.line = mCurrentLine;
				        newtoken.pos = mCharPos;
                        newtoken.found = true;

                        // check to see if a terminal token is waiting to be inserted based on the next
                        // token being found
                        if (mInsertTokenID)
                        {
                            newtoken.tokenID = mInsertTokenID;
                            mActiveTokenState->tokenQue.push_back(newtoken);
                            // token action processing
                            // if the token has an action then fire previous token action
                            checkTokenActionTrigger();
                            // reset the token ID that was inserted so that it will not get inserted until set again
                            mInsertTokenID = 0;
                        }

                        newtoken.tokenID = tokenID;
				        mActiveTokenState->tokenQue.push_back(newtoken);
				        // token action processing
				        // if the token has an action then fire previous token action
				        checkTokenActionTrigger();
                    }

                    // update source position
				    mCharPos += tokenlength;
			    } // end if
		    } // end else if

	    }
	    // else a non terminal token was found
	    else
	    {
		    // execute rule for non-terminal
		    // get rule_ID for index into  rulepath to be called
		    Passed = processRulePath(mActiveTokenState->lexemeTokenDefinitions[tokenID].ruleID);
	    }

	    return Passed;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isFloatValue(float& fvalue, size_t& charsize) const
    {
        if (isEndOfSource())
            return false;
            
	    // check to see if it is a numeric float value
	    bool valuefound = false;

        const char* startptr = mSource->c_str() + mCharPos;
	    char* endptr = NULL;

	    fvalue = static_cast<float>(strtod(startptr, &endptr));
	    // if a valid float was found then endptr will have the pointer to the first invalid character
	    if (endptr)
	    {
		    if (endptr > startptr)
		    {
			    // a valid value was found so process it
			    charsize = endptr - startptr;
			    valuefound = true;
		    }
	    }

	    return valuefound;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isLexemeMatch(const String& lexeme, const bool caseSensitive) const
    {
	    // compare text at source+charpos with the lexeme : limit testing to lexeme size
	    if (!caseSensitive)
	    {
	        String testItem = mSource->substr(mCharPos, lexeme.length());
	        StringUtil::toLowerCase(testItem);
            return (testItem.compare(lexeme) == 0);
	    }
	    else
	    {
            return (mSource->compare(mCharPos, lexeme.length(), lexeme) == 0);
	    }
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::positionToNextLexeme()
    {
        bool validlexemefound = false;
        size_t oldCharPos = mCharPos;
        
	    while (!validlexemefound && !isEndOfSource())
	    {
		    skipWhiteSpace();
		    skipEOL();
		    skipComments();
		    // have we reached the end of the source?
            if (!isEndOfSource())
		    {
			    // if ASCII > space then assume valid character is found
			    if (static_cast<uchar>((*mSource)[mCharPos]) > static_cast<uchar>(' '))
			    {
			        validlexemefound = true;
			    }
			    else // maybe a control character has been encountered?
			    {
                    // check if the char pos advanced in this iteration.
                    // If it didn't then we have found a char that
                    // is not relevent to the parse so skip it so that we don't
                    // end up in an infinite loop.
                    if (oldCharPos == mCharPos)
                        ++mCharPos;

			        // endofsource will get checked on next iteration of this loop so no need to check it here
			        // need to update oldCharPos so that position advancement can be varified on the next iteration
                    oldCharPos = mCharPos;
			    }
		    }
	    }// end of while

	    return validlexemefound;
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::skipComments()
    {
        if (isEndOfSource())
            return;
        // if current char and next are // then search for EOL
        if (mSource->compare(mCharPos, 2, "//") == 0)
			 findEOL();
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::findEOL()
    {
        if (isEndOfSource())
            return;
	    // find eol charter and move to this position
        mCharPos = mSource->find('\n', mCharPos);
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::skipEOL()
    {
        if (isEndOfSource())
            return;

	    if (((*mSource)[mCharPos] == '\n') || ((*mSource)[mCharPos] == '\r'))
	    {
		    mCurrentLine++;
		    mCharPos++;
            if (mCharPos >= mEndOfSource)
                return;
		    if (((*mSource)[mCharPos] == '\n') || ((*mSource)[mCharPos] == '\r'))
		    {
			    mCharPos++;
		    }
	    }
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::skipWhiteSpace()
    {
        if (isEndOfSource())
            return;

        mCharPos = mSource->find_first_not_of(" \t", mCharPos);
    }

    //-----------------------------------------------------------------------
    size_t Compiler2Pass::addLexemeToken(const String& lexeme, const size_t token, const bool hasAction, const bool caseSensitive)
    {
        size_t newTokenID = token;
        // if token ID is zero then auto-generate a new token ID
        if (newTokenID == 0)
        {
            // assume BNF system bootstrap is current state
            size_t autoTokenIDStart = BNF_AUTOTOKENSTART;
            // if in client state then get auto token start position from the client
            if (mActiveTokenState != &mBNFTokenState)
                autoTokenIDStart = getAutoTokenIDStart();
            // make sure new auto gen id starts at autoTokenIDStart or greater
            newTokenID = (mActiveTokenState->lexemeTokenDefinitions.size() <= autoTokenIDStart ) ? autoTokenIDStart : mActiveTokenState->lexemeTokenDefinitions.size();
        }

        if (newTokenID >= mActiveTokenState->lexemeTokenDefinitions.size())
        {
            mActiveTokenState->lexemeTokenDefinitions.resize(newTokenID + 1);
        }
        // since resizing guarentees the token definition will exist, just assign values to members
        LexemeTokenDef& tokenDef = mActiveTokenState->lexemeTokenDefinitions[newTokenID];
        if (tokenDef.ID != 0)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "In " + getClientGrammerName() +
                ", lexeme >>>" +
                lexeme + "<<< already exists in lexeme token definitions",
                "Compiler2Pass::addLexemeToken");
        }
        tokenDef.ID = newTokenID;
        tokenDef.lexeme = lexeme;
        if (!caseSensitive)
            StringUtil::toLowerCase(tokenDef.lexeme);
        tokenDef.hasAction = hasAction;
        tokenDef.isCaseSensitive = caseSensitive;

        mActiveTokenState->lexemeTokenMap[lexeme] = newTokenID;

        return newTokenID;
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::checkTokenActionTrigger(void)
    {
        size_t lastTokenQuePos = mActiveTokenState->tokenQue.size();
        // if there are no token instructions in the que then there is nothing todo
        if (lastTokenQuePos == 0)
            return;

        --lastTokenQuePos;

        if (lastTokenQuePos == mPreviousActionQuePosition)
            return;

        // check action trigger if last token has an action
        if (setNextActionQuePosition(lastTokenQuePos))
        {
            // only activate the action belonging to the token found previously
            activatePreviousTokenAction();
            // current token action now becomes the previous one
            mPreviousActionQuePosition = lastTokenQuePos;
        }
    }

    //-----------------------------------------------------------------------
    String Compiler2Pass::getBNFGrammerTextFromRulePath(size_t ruleID, const size_t level)
    {

        String grammerText;

        // default to using Client rule path
        // check if index is inbounds
        if (ruleID >= mActiveTokenState->rootRulePath.size())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "rule ID exceeds client rule path bounds.", "Compiler2Pass::getBNFGrammerRulePathText");
        }
        // iterate through rule path and get terminal and non-terminal strings
        const TokenRuleContainer& rulePath = mActiveTokenState->rootRulePath;

        while (rulePath[ruleID].operation != otEND)
        {
            // rule text processing - the op instructions, system tokens
            switch (rulePath[ruleID].operation)
            {
            // rule lexeme ::=
            case otRULE:
                grammerText += "\n" + getLexemeText(ruleID, level) + " ::=";
                break;
            // no special processing for AND op
            case otAND:
                grammerText += " " + getLexemeText(ruleID, level);
                break;
            // or | lexeme
            case otOR:
                grammerText += " | " + getLexemeText(ruleID, level);
                break;
            // optional [lexeme]
            case otOPTIONAL:
                grammerText += " [" + getLexemeText(ruleID, level) + "]";
                break;
            // repeat {lexeme}
            case otREPEAT:
                grammerText += " {" + getLexemeText(ruleID, level) + "}";
                break;
            // not test (?!lexeme)
            case otNOT_TEST:
                grammerText += " (?!" + getLexemeText(ruleID, level) + ")";
                break;
            default:
                grammerText += "*** Unknown Operation ***";
            }
            // lexeme/token text procesing
            ++ruleID;
        }

        return grammerText;
    }

    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //              Private Methods
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String Compiler2Pass::getLexemeText(size_t& ruleID, const size_t level)
    {
        if (ruleID >= mActiveTokenState->rootRulePath.size())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
             "rule ID exceeds client rule path bounds.", "Compiler2Pass::getLexemeText"
            );
        }

        String lexeme;

        const TokenRuleContainer& rulePath = mActiveTokenState->rootRulePath;
        const size_t tokenID = rulePath[ruleID].tokenID;

        if ( tokenID < SystemTokenBase)
        {
            // non-terminal tokens
            if (mActiveTokenState->lexemeTokenDefinitions[tokenID].isNonTerminal)
            {
                // allow expansion of non-terminals into terminals
                if (level > 0)
                {
                    size_t subRuleID = mActiveTokenState->lexemeTokenDefinitions[tokenID].ruleID + 1;
                    lexeme = getBNFGrammerTextFromRulePath(subRuleID, level - 1);
                }
                else
                {
                    lexeme = "<" + mActiveTokenState->lexemeTokenDefinitions[tokenID].lexeme + ">";
                }
            }
            else // terminal tokens
            {
                lexeme = "'" + mActiveTokenState->lexemeTokenDefinitions[tokenID].lexeme + "'";
            }
        }
        else // system token processing
        {
            switch (rulePath[ruleID].tokenID)
            {
            case _character_:
                // need to get next rule instruction for data
                ++ruleID;
                // data for _character_ is always a set so put () around text string
                lexeme = "(" + mActiveTokenState->lexemeTokenDefinitions[rulePath[ruleID].tokenID].lexeme + ")";
                break;
            case _value_:
                // <#> - need name of label?
                lexeme = "<#Number>";
                break;
            }
        }

        return lexeme;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::activatePreviousTokenAction(void)
    {
        const size_t previousTokenID = mActiveTokenState->tokenQue.at(mPreviousActionQuePosition).tokenID;
        const LexemeTokenDef& tokenDef = mActiveTokenState->lexemeTokenDefinitions.at(previousTokenID);
        if (tokenDef.hasAction)
        {
            // set the current pass 2 token que position to previous action que position
            // assume that pass 2 processing will use tokens downstream
            mPass2TokenQuePosition = mPreviousActionQuePosition;
            executeTokenAction(previousTokenID);
        }
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::buildClientBNFRulePaths(void)
    {
        bool isFirstToken = true;
        OperationType pendingRuleOp = otAND;

        // convert tokens in BNF token que to rule paths
        while (getPass2TokenQueCount() > 0)
        {
            // get a pass 2 token
            // if this is the first time getting a token then get the current token
            const TokenInst& currentToken  = isFirstToken ? getCurrentToken() : getNextToken();
            isFirstToken = false;
            // only process the token if its valid
            if (currentToken.found)
            {
                // a valid token has been found, convert to a rule
                switch (currentToken.tokenID)
                {
                case BNF_ID_BEGIN: // <
                    extractNonTerminal(pendingRuleOp);
                    pendingRuleOp = otAND;
                    break;


                case BNF_CONSTANT_BEGIN: // <#
                    extractNumericConstant(pendingRuleOp);
                    pendingRuleOp = otAND;
                    break;

                case BNF_OR: // |
                    pendingRuleOp = otOR;
                    break;

                case BNF_REPEAT_BEGIN: // {
                    pendingRuleOp = otREPEAT;
                    break;

                case BNF_NO_TOKEN_START: // -'
                    extractTerminal(pendingRuleOp, true);
                    pendingRuleOp = otAND;
                    break;

                case BNF_SINGLEQUOTE: // '
                    extractTerminal(pendingRuleOp);
                    pendingRuleOp = otAND;
                    break;

                case BNF_OPTIONAL_BEGIN: // [
                    pendingRuleOp = otOPTIONAL;
                    break;

                case BNF_NOT_TEST_BEGIN: // (?!
                    pendingRuleOp = otNOT_TEST;
                    break;

                case BNF_SET_BEGIN: // (
                    extractSet(pendingRuleOp);
                    pendingRuleOp = otAND;
                    break;

                case BNF_CONDITIONAL_TOKEN_INSERT:
                    setConditionalTokenInsert();
                    break;

                default:
                    // trap closings ie ] } )
                    break;
                } // end switch
            } // end if
        } // end while
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::modifyLastRule(const OperationType pendingRuleOp, const size_t tokenID)
    {
        // add operation using this token ID to the current rule expression
        size_t lastIndex = mClientTokenState->rootRulePath.size();
        if (lastIndex == 0)
        {
            // throw exception since there should have been at least one rule existing
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "BNF Grammar build rules failed: no previous rule op defined", "Compiler2Pass::modifyLastRule");
        }
        --lastIndex;
        mClientTokenState->rootRulePath[lastIndex].operation = pendingRuleOp;
        mClientTokenState->rootRulePath[lastIndex].tokenID = tokenID;
        // add new end op token rule
        mClientTokenState->rootRulePath.push_back(TokenRule(otEND, 0));
    }

    //-----------------------------------------------------------------------
    size_t Compiler2Pass::getClientLexemeTokenID(const String& lexeme, const bool isCaseSensitive)
    {
        size_t tokenID = mClientTokenState->lexemeTokenMap[lexeme];

        if (tokenID == 0)
        {
            // lexeme not found so a new entry is made by the system
            // note that all lexemes added by the system will not/can not have an action
            tokenID = mClientTokenState->lexemeTokenDefinitions.size();
            // add identifier to client lexeme tokens
            mActiveTokenState = mClientTokenState;
            addLexemeToken(lexeme, tokenID, false, isCaseSensitive);
            mActiveTokenState = &mBNFTokenState;
        }

        return tokenID;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::extractNonTerminal(const OperationType pendingRuleOp)
    {
        // begining of identifier
        // next token should be for a label
        const String& identifierLabel = getNextTokenLabel();
        // next token should be id end
        getNextToken(BNF_ID_END);
        // add identifier to lexeme token definitions but keep case sensitivity
        const size_t tokenID = getClientLexemeTokenID(identifierLabel, true);
        LexemeTokenDef& tokenDef = mClientTokenState->lexemeTokenDefinitions[tokenID];

        // peek at the next token isntruction to see if this
        // identifier is for a new rule or is part of the current rule
        if (testNextTokenID(BNF_SET_RULE))
        {
            // consume set rule
            getNextToken(BNF_SET_RULE);
            // check to make sure this is the first time this rule is being setup by
            // verifying rule id is 0
            if (tokenDef.ruleID != 0)
            {
                // this is not the first time for this identifier to be set up as a rule
                // since duplicate rules can not exist, throw an exception
                OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "while parsing BNF grammer for: " +
                    getClientGrammerName() +
                    ", an attempt was made to assign a rule to identifier: " +
                    tokenDef.lexeme + ", that already had a rule assigned",
                    "Compiler2Pass::extractNonTerminal");
            }
            // add new rule to end of rule path
            mClientTokenState->rootRulePath.push_back(TokenRule(otRULE, tokenID));
            tokenDef.ruleID = mClientTokenState->rootRulePath.size() - 1;
            // add new end op token rule
            mClientTokenState->rootRulePath.push_back(TokenRule(otEND, 0));
        }
        else // just a reference to a non-terminal
        {
            modifyLastRule(pendingRuleOp, tokenID);
        }

        tokenDef.isNonTerminal = true;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::extractTerminal(const OperationType pendingRuleOp, const bool notoken)
    {
        // begining of label
        // next token should be for a label
        const String& terminalLabel = getNextTokenLabel();
        // next token should be single quote end
        getNextToken(BNF_SINGLEQUOTE);
        // add terminal to lexeme token definitions
        // note that if label not in the map it is automatically added
        const size_t tokenID = getClientLexemeTokenID(terminalLabel);
        if (notoken)
            modifyLastRule(otAND, _no_token_);
        modifyLastRule(pendingRuleOp, tokenID);
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::setConditionalTokenInsert(void)
    {
        // get position of rule just before end rule
        size_t lastIndex = mClientTokenState->rootRulePath.size();
        if (lastIndex <= 1)
        {
            // throw exception since there should have been at least one rule existing
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "BNF Grammar build rules failed: no previous terminal token rule defined",
                        "Compiler2Pass::setConditionalTokenInsert");
        }
        lastIndex -= 2;
        mClientTokenState->rootRulePath[lastIndex].operation = otINSERT_TOKEN;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::extractSet(const OperationType pendingRuleOp)
    {
        const String& setLabel = getNextTokenLabel();
        // next token should be )
        getNextToken(BNF_SET_END);
        // add set to lexeme token definitions but keep case sensitivity
        const size_t tokenID = getClientLexemeTokenID(setLabel, true);
        // add operation using this token ID to the current rule expression
        modifyLastRule(pendingRuleOp, _character_);
        // add the data required by the character lookup operation
        modifyLastRule(otDATA, tokenID);
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::extractNumericConstant(const OperationType pendingRuleOp)
    {
        // consume label for constant, don't need it for anything
        getNextTokenLabel();

        getNextToken(BNF_ID_END); // >
        // add operation using this token ID to the current rule expression
        modifyLastRule(pendingRuleOp, _value_);
    }


}
