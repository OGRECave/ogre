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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Compiler2Pass.h"

Compiler2Pass::Compiler2Pass()
{
	// reserve some memory space in the containers being used
	mTokenInstructions.reserve(100);
	mConstants.reserve(80);
	// default contexts allows all contexts
	// subclass should change it to fit the language being compiled
	mActiveContexts = 0xffffffff;

}



void Compiler2Pass::InitSymbolTypeLib()
{
	uint token_ID;
	// find a default text for all Symbol Types in library

	// scan through all the rules and initialize TypeLib with index to text and index to rules for non-terminal tokens
	for(int i = 0; i < mRulePathLibCnt; i++) {
		token_ID = mRootRulePath[i].mTokenID;
		// make sure SymbolTypeLib holds valid token
		assert(mSymbolTypeLib[token_ID].mID == token_ID);
		switch(mRootRulePath[i].mOperation) {
			case otRULE:
				// if operation is a rule then update typelib
				mSymbolTypeLib[token_ID].mRuleID = i;

			case otAND:
			case otOR:
			case otOPTIONAL:
				// update text index in typelib
				if (mRootRulePath[i].mSymbol != NULL) mSymbolTypeLib[token_ID].mDefTextID = i;
				break;
            case otREPEAT:
            case otEND:
                break;
		}
	}

}


bool Compiler2Pass::compile(const char* source)
{
	bool Passed = false;

	mSource = source;
	// start compiling if there is a rule base to work with
	if(mRootRulePath != NULL) {
		 Passed = doPass1();

		if(Passed) {
			Passed = doPass2();
		}
	}
	return Passed;
}


bool Compiler2Pass::doPass1()
{
	// scan through Source string and build a token list using TokenInstructions
	// this is a simple brute force lexical scanner/analyzer that also parses the formed
	// token for proper semantics and context in one pass

	mCurrentLine = 1;
	mCharPos = 0;
	// reset position in Constants container
	mConstants.clear();
	mEndOfSource = strlen(mSource);

	// start with a clean slate
	mTokenInstructions.clear();
	// tokenize and check semantics untill an error occurs or end of source is reached
	// assume RootRulePath has pointer to rules so start at index + 1 for first rule path
	// first rule token would be a rule definition so skip over it
	bool passed = processRulePath(0);
	// if a symbol in source still exists then the end of source was not reached and there was a problem some where
	if (positionToNextSymbol()) passed = false;
	return passed;

}


bool Compiler2Pass::processRulePath( uint rulepathIDX)
{
	// rule path determines what tokens and therefore what symbols are acceptable from the source
	// it is assumed that the tokens with the longest similar symbols are arranged first so
	// if a match is found it is accepted and no further searching is done

	// record position of last token in container
	// to be used as the rollback position if a valid token is not found
	uint TokenContainerOldSize = mTokenInstructions.size();
	int OldCharPos = mCharPos;
	int OldLinePos = mCurrentLine;
	uint OldConstantsSize = mConstants.size();

	// keep track of what non-terminal token activated the rule
	uint ActiveNTTRule = mRootRulePath[rulepathIDX].mTokenID;
	// start rule path at next position for definition
	rulepathIDX++;

	// assume the rule will pass
	bool Passed = true;
	bool EndFound = false;

	// keep following rulepath until the end is reached
	while (EndFound == false) {
		switch (mRootRulePath[rulepathIDX].mOperation) {

			case otAND:
				// only validate if the previous rule passed
				if(Passed) Passed = ValidateToken(rulepathIDX, ActiveNTTRule); 
				break;

			case otOR:
				// only validate if the previous rule failed
				if ( Passed == false ) {
					// clear previous tokens from entry and try again
					mTokenInstructions.resize(TokenContainerOldSize);
					Passed = ValidateToken(rulepathIDX, ActiveNTTRule);
				}
				else { // path passed up to this point therefore finished so pretend end marker found
					EndFound = true;
				}
				break;

			case otOPTIONAL:
				// if previous passed then try this rule but it does not effect succes of rule since its optional
				if(Passed) ValidateToken(rulepathIDX, ActiveNTTRule); 
				break;

			case otREPEAT:
				// repeat until no tokens of this type found 
				// at least one must be found
				if(Passed) {
					int TokensPassed = 0;
					// keep calling until failure
					while ( Passed = ValidateToken(rulepathIDX, ActiveNTTRule)) {
						// increment count for previous passed token
						TokensPassed++;
					}
					// defaults to Passed = fail
					// if at least one token found then return passed = true
					if (TokensPassed > 0) Passed = true;
				}
				break;

			case otEND:
				// end of rule found so time to return
				EndFound = true;
				if(Passed == false) {
					// the rule did not validate so get rid of tokens decoded
					// roll back the token container end position to what it was when rule started
					// this will get rid of all tokens that had been pushed on the container while
					// trying to validating this rule
					mTokenInstructions.resize(TokenContainerOldSize);
					mConstants.resize(OldConstantsSize);
					mCharPos = OldCharPos;
					mCurrentLine = OldLinePos;
				}
				break;

			default:
				// an exception should be raised since the code should never get here
				Passed = false;
				EndFound = true;
				break;

		}


		// move on to the next rule in the path
		rulepathIDX++;
	}

	return Passed;

}


bool Compiler2Pass::ValidateToken(const uint rulepathIDX, const uint activeRuleID)
{
	int tokenlength = 0;
	// assume the test is going to fail
	bool Passed = false;
	uint TokenID = mRootRulePath[rulepathIDX].mTokenID;
	// only validate token if context is correct
	if (mSymbolTypeLib[TokenID].mContextKey & mActiveContexts) {
	
		// if terminal token then compare text of symbol with what is in source
		if ( mSymbolTypeLib[TokenID].mRuleID == 0){

			if (positionToNextSymbol()) {
				// if Token is supposed to be a number then check if its a numerical constant
				if (TokenID == mValueID) {
					float constantvalue;
					if(Passed = isFloatValue(constantvalue, tokenlength)) {
						mConstants.push_back(constantvalue);
					}
					
				}
				// compare token symbol text with source text
				else Passed = isSymbol(mRootRulePath[rulepathIDX].mSymbol, tokenlength);
					
				if(Passed) {
					TokenInst newtoken;
					// push token onto end of container
					newtoken.mID = TokenID;
					newtoken.mNTTRuleID = activeRuleID;
					newtoken.mLine = mCurrentLine;
					newtoken.mPos = mCharPos;

					mTokenInstructions.push_back(newtoken);
					// update source position
					mCharPos += tokenlength;

					// allow token instruction to change the ActiveContexts
					// use token contexts pattern to clear ActiveContexts pattern bits
					mActiveContexts &= ~mSymbolTypeLib[TokenID].mContextPatternClear;
					// use token contexts pattern to set ActiveContexts pattern bits
					mActiveContexts |= mSymbolTypeLib[TokenID].mContextPatternSet;
				}
			}

		}
		// else a non terminal token was found
		else {

			// execute rule for non-terminal
			// get rule_ID for index into  rulepath to be called
			Passed = processRulePath(mSymbolTypeLib[TokenID].mRuleID);
		}
	}


	return Passed;

}


const char* Compiler2Pass::getTypeDefText(const uint sid)
{
	return mRootRulePath[mSymbolTypeLib[sid].mDefTextID].mSymbol;
}


bool Compiler2Pass::isFloatValue(float& fvalue, int& charsize)
{
	// check to see if it is a numeric float value
	bool valuefound = false;

	const char* startptr = mSource + mCharPos;
	char* endptr = NULL;

	fvalue = (float)strtod(startptr, &endptr);
	// if a valid float was found then endptr will have the pointer to the first invalid character
	if(endptr) {
		if(endptr>startptr) {
			// a valid value was found so process it
			charsize = endptr - startptr;
			valuefound = true;
		}
	}

	return valuefound;
}


bool Compiler2Pass::isSymbol(const char* symbol, int& symbolsize)
{
	// compare text at source+charpos with the symbol : limit testing to symbolsize
	bool symbolfound = false;
	symbolsize = strlen(symbol);
	if(strncmp(mSource + mCharPos, symbol, symbolsize)==0) {
		symbolfound = true;
	}

	return symbolfound;
}


bool Compiler2Pass::positionToNextSymbol()
{
	bool validsymbolfound = false;
	bool endofsource = false;
	while(!validsymbolfound && !endofsource) {
		skipWhiteSpace();
		skipEOL();
		skipComments();
		// have we reached the end of the string?
		if (mCharPos == mEndOfSource) endofsource = true;
		else {
			// if ASCII > space then assume valid character is found
			if (mSource[mCharPos] > ' ') validsymbolfound = true;
		}
	}// end of while

	return validsymbolfound;
}



void Compiler2Pass::skipComments()
{
  // if current char and next are // then search for EOL
	if(mCharPos < mEndOfSource) {
		if( ((mSource[mCharPos] == '/') && (mSource[mCharPos + 1] == '/')) ||
			(mSource[mCharPos] == ';') ||
			(mSource[mCharPos] == '#') ) findEOL();
	}
}


void Compiler2Pass::findEOL()
{
	// find eol charter and move to this position
	const char* newpos = strchr(&mSource[mCharPos], '\n');
	if(newpos) {
		mCharPos += newpos - &mSource[mCharPos];
	}
	// couldn't find end of line so skip to the end
	else mCharPos = mEndOfSource - 1;

}


void Compiler2Pass::skipEOL()
{
	if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r')) {
		mCurrentLine++;
		mCharPos++;
		if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r')) {
			mCharPos++;
		}
	}
}


void Compiler2Pass::skipWhiteSpace()
{
	// FIX - this method kinda slow
	while((mSource[mCharPos] == ' ') || (mSource[mCharPos] == '\t')) mCharPos++; // find first non white space character
}

