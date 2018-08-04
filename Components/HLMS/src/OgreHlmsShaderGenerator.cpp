/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#include "OgreHlmsShaderGenerator.h"

namespace Ogre
{
    class SubStringRef
    {
        String const *mOriginal;
        size_t  mStart;
        size_t  mEnd;

    public:
        SubStringRef(const String *original, size_t start) :
            mOriginal( original ),
            mStart( start ),
            mEnd( original->size() )
        {
            assert( start <= original->size() );
        }

        SubStringRef(const String *original, size_t _start, size_t _end) :
            mOriginal( original ),
            mStart( _start ),
            mEnd( _end )
        {
            assert( _start <= _end );
            assert( _end <= original->size() );
        }

        SubStringRef(const String *original, String::const_iterator _start) :
            mOriginal( original ),
            mStart( _start - original->begin() ),
            mEnd( original->size() )
        {
        }

        size_t find( const char *value, size_t pos=0 ) const
        {
            size_t retVal = mOriginal->find( value, mStart + pos );
            if( retVal >= mEnd )
                retVal = String::npos;
            else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

        size_t find(const String &value) const
        {
            size_t retVal = mOriginal->find( value, mStart );
            if( retVal >= mEnd )
                retVal = String::npos;
            else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

        size_t findFirstOf( const char *c, size_t pos ) const
        {
            size_t retVal = mOriginal->find_first_of( c, mStart + pos );
            if( retVal >= mEnd )
                retVal = String::npos;
            else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

        bool matchEqual( const char *stringCompare ) const
        {
            const char *origStr = mOriginal->c_str() + mStart;
            ptrdiff_t length = mEnd - mStart;
            while( *origStr == *stringCompare && *origStr && --length )
                ++origStr, ++stringCompare;

            return length == 0 && *origStr == *stringCompare;
        }

        void setStart( size_t newStart )            { mStart = std::min( newStart, mOriginal->size() ); }
        void setEnd( size_t newEnd )                { mEnd = std::min( newEnd, mOriginal->size() ); }
        size_t getStart(void) const                 { return mStart; }
        size_t getEnd(void) const                   { return mEnd; }
        size_t getSize(void) const                  { return mEnd - mStart; }
        String::const_iterator begin() const        { return mOriginal->begin() + mStart; }
        String::const_iterator end() const          { return mOriginal->begin() + mEnd; }
        const String& getOriginalBuffer() const     { return *mOriginal; }
    };
	//-----------------------------------------------------------------------------------
	void ShaderGenerator::findBlockEnd(SubStringRef &outSubString, bool &syntaxError)
	{
		const char *blockNames[] =
		{
			"foreach",
			"property",
			"piece"
		};

		String::const_iterator it = outSubString.begin();
		String::const_iterator en = outSubString.end();

		int nesting = 0;

		while (it != en && nesting >= 0)
		{
			if (*it == '@')
			{
				SubStringRef subString(&outSubString.getOriginalBuffer(), it + 1);

				size_t idx = subString.find("end");
				if (idx == 0)
				{
					--nesting;
					it += sizeof("end") - 1;
				}
				else
				{
					for (size_t i = 0; i < sizeof(blockNames) / sizeof(char*); ++i)
					{
						size_t idxBlock = subString.find(blockNames[i]);
						if (idxBlock == 0)
						{
							it = subString.begin() + strlen(blockNames[i]);
							++nesting;
							break;
						}
					}
				}
			}

			++it;
		}

		assert(nesting == -1);

		if (nesting == -1)
			outSubString.setEnd(it - outSubString.getOriginalBuffer().begin() - sizeof("end"));
		else
		{
			syntaxError = false;
			printf("Syntax Error at line %zu: start block (e.g. @foreach; @property) "
				"without matching @end\n", calculateLineCount(outSubString));
		}
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::evaluateExpression(SubStringRef &outSubString, bool &outSyntaxError, PropertyMap &properties)
	{
		size_t expEnd = evaluateExpressionEnd(outSubString);

		if (expEnd == String::npos)
		{
			outSyntaxError = true;
			return false;
		}

		SubStringRef subString(&outSubString.getOriginalBuffer(), outSubString.getStart(),
			outSubString.getStart() + expEnd);

		outSubString = SubStringRef(&outSubString.getOriginalBuffer(),
			outSubString.getStart() + expEnd + 1);

		bool textStarted = false;
		bool syntaxError = false;
		bool nextExpressionNegates = false;

		std::vector<Expression*> expressionParents;
		ExpressionVec outExpressions;
		outExpressions.clear();
		outExpressions.resize(1);

		Expression *currentExpression = &outExpressions.back();

		String::const_iterator it = subString.begin();
		String::const_iterator en = subString.end();

		while (it != en && !syntaxError)
		{
			char c = *it;

			if (c == '(')
			{
				currentExpression->children.push_back(Expression());
				expressionParents.push_back(currentExpression);

				currentExpression->children.back().negated = nextExpressionNegates;

				textStarted = false;
				nextExpressionNegates = false;

				currentExpression = &currentExpression->children.back();
			}
			else if (c == ')')
			{
				if (expressionParents.empty())
					syntaxError = true;
				else
				{
					currentExpression = expressionParents.back();
					expressionParents.pop_back();
				}

				textStarted = false;
			}
			else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			{
				textStarted = false;
			}
			else if (c == '!')
			{
				nextExpressionNegates = true;
			}
			else
			{
				if (!textStarted)
				{
					textStarted = true;
					currentExpression->children.push_back(Expression());
					currentExpression->children.back().negated = nextExpressionNegates;
				}

				if (c == '&' || c == '|')
				{
					if (currentExpression->children.empty() || nextExpressionNegates)
					{
						syntaxError = true;
					}
					else if (!currentExpression->children.back().value.empty() &&
						c != *(currentExpression->children.back().value.end() - 1))
					{
						currentExpression->children.push_back(Expression());
					}
				}

				currentExpression->children.back().value.push_back(c);
				nextExpressionNegates = false;
			}

			++it;
		}

		bool retVal = false;

		if (!expressionParents.empty())
			syntaxError = true;

		if (!syntaxError)
			retVal = evaluateExpressionRecursive(outExpressions, syntaxError, properties);

		if (syntaxError)
			printf("Syntax Error at line %zu\n", calculateLineCount(subString));

		outSyntaxError = syntaxError;

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::evaluateExpressionRecursive(ExpressionVec &expression, bool &outSyntaxError, PropertyMap &properties)
	{
		ExpressionVec::iterator itor = expression.begin();
		ExpressionVec::iterator end = expression.end();

		while (itor != end)
		{
			if (itor->value == "&&")
				itor->type = EXPR_OPERATOR_AND;
			else if (itor->value == "||")
				itor->type = EXPR_OPERATOR_OR;
			else if (!itor->children.empty())
				itor->type = EXPR_OBJECT;
			else
				itor->type = EXPR_VAR;

			++itor;
		}

		bool syntaxError = outSyntaxError;
		bool lastExpWasOperator = true;

		itor = expression.begin();

		while (itor != end && !syntaxError)
		{
			Expression &exp = *itor;
			if (((exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND) && lastExpWasOperator) ||
				((exp.type == EXPR_VAR || exp.type == EXPR_OBJECT) && !lastExpWasOperator))
			{
				syntaxError = true;
				printf("Unrecognized token '%s'", exp.value.c_str());
			}
			else if (exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND)
			{
				lastExpWasOperator = true;
			}
			else if (exp.type == EXPR_VAR)
			{
				exp.result = properties.getProperty(exp.value) != 0;
				lastExpWasOperator = false;
			}
			else
			{
				exp.result = evaluateExpressionRecursive(exp.children, syntaxError, properties);
				lastExpWasOperator = false;
			}

			++itor;
		}

		bool retVal = true;

		if (!syntaxError)
		{
			itor = expression.begin();
			bool andMode = true;

			while (itor != end)
			{
				if (itor->type == EXPR_OPERATOR_OR)
					andMode = false;
				else if (itor->type == EXPR_OPERATOR_AND)
					andMode = true;
				else
				{
					if (andMode)
						retVal &= itor->negated ? !itor->result : itor->result;
					else
						retVal |= itor->negated ? !itor->result : itor->result;
				}

				++itor;
			}
		}

		outSyntaxError = syntaxError;

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	size_t ShaderGenerator::evaluateExpressionEnd(const SubStringRef &outSubString)
	{
		String::const_iterator it = outSubString.begin();
		String::const_iterator en = outSubString.end();

		int nesting = 0;

		while (it != en && nesting >= 0)
		{
			if (*it == '(')
				++nesting;
			else if (*it == ')')
				--nesting;
			++it;
		}

		assert(nesting >= -1);

		size_t retVal = String::npos;
		if (it != en && nesting < 0)
		{
			retVal = it - outSubString.begin() - 1;
		}
		else
		{
			printf("Syntax Error at line %zu: opening parenthesis without matching closure\n",
				calculateLineCount(outSubString));
		}

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	void ShaderGenerator::evaluateParamArgs(SubStringRef &outSubString, StringVector &outArgs,
		bool &outSyntaxError)
	{
		size_t expEnd = evaluateExpressionEnd(outSubString);

		if (expEnd == String::npos)
		{
			outSyntaxError = true;
			return;
		}

		SubStringRef subString(&outSubString.getOriginalBuffer(), outSubString.getStart(),
			outSubString.getStart() + expEnd);

		outSubString = SubStringRef(&outSubString.getOriginalBuffer(),
			outSubString.getStart() + expEnd + 1);

		int expressionState = 0;
		bool syntaxError = false;

		outArgs.clear();
		outArgs.push_back(String());

		String::const_iterator it = subString.begin();
		String::const_iterator en = subString.end();

		while (it != en && !syntaxError)
		{
			char c = *it;

			if (c == '(' || c == ')' || c == '@' || c == '&' || c == '|')
			{
				syntaxError = true;
			}
			else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			{
				if (expressionState == 1)
					expressionState = 2;
			}
			else if (c == ',')
			{
				expressionState = 0;
				outArgs.push_back(String());
			}
			else
			{
				if (expressionState == 2)
				{
					printf("Syntax Error at line %zu: ',' or ')' expected\n",
						calculateLineCount(subString));
					syntaxError = true;
				}
				else
				{
					outArgs.back().push_back(*it);
					expressionState = 1;
				}
			}

			++it;
		}

		if (syntaxError)
			printf("Syntax Error at line %zu\n", calculateLineCount(subString));

		outSyntaxError = syntaxError;
	}
	//-----------------------------------------------------------------------------------
	void ShaderGenerator::copy(String &outBuffer, const SubStringRef &inSubString, size_t length)
	{
		String::const_iterator itor = inSubString.begin();
		String::const_iterator end = inSubString.begin() + length;

		while (itor != end)
			outBuffer.push_back(*itor++);
	}
	//-----------------------------------------------------------------------------------
	void ShaderGenerator::repeat(String &outBuffer, const SubStringRef &inSubString, size_t length,
		size_t passNum, const String &counterVar)
	{
		String::const_iterator itor = inSubString.begin();
		String::const_iterator end = inSubString.begin() + length;

		while (itor != end)
		{
			if (*itor == '@' && !counterVar.empty())
			{
				SubStringRef subString(&inSubString.getOriginalBuffer(), itor + 1);
				if (subString.find(counterVar) == 0)
				{
					outBuffer += StringConverter::toString(passNum);
					itor += counterVar.size() + 1;
				}
				else
				{
					outBuffer.push_back(*itor++);
				}
			}
			else
			{
				outBuffer.push_back(*itor++);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	static int setOp(int op1, int op2) { return op2; }
	static int addOp(int op1, int op2) { return op1 + op2; }
	static int subOp(int op1, int op2) { return op1 - op2; }
	static int mulOp(int op1, int op2) { return op1 * op2; }
	static int divOp(int op1, int op2) { return op1 / op2; }
	static int modOp(int op1, int op2) { return op1 % op2; }

	struct Operation
	{
		const char *opName;
		size_t length;
		int(*opFunc)(int, int);
		Operation(const char *_name, size_t len, int(*_opFunc)(int, int)) :
			opName(_name), length(len), opFunc(_opFunc) {}
	};

	const Operation c_operations[6] =
	{
		Operation("pset", sizeof("@pset"), &setOp),
		Operation("padd", sizeof("@padd"), &addOp),
		Operation("psub", sizeof("@psub"), &subOp),
		Operation("pmul", sizeof("@pmul"), &mulOp),
		Operation("pdiv", sizeof("@pdiv"), &divOp),
		Operation("pmod", sizeof("@pmod"), &modOp)
	};
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::parseMath(const String &inBuffer, String &outBuffer, PropertyMap &properties)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		StringVector argValues;
		SubStringRef subString(&inBuffer, 0);

		size_t pos;
		pos = subString.find("@");
		size_t keyword = ~0;

		while (pos != String::npos && keyword == (size_t)~0)
		{
			size_t maxSize = subString.findFirstOf(" \t(", pos + 1);
			maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
			SubStringRef keywordStr(&inBuffer, subString.getStart() + pos + 1,
				subString.getStart() + maxSize);

			for (size_t i = 0; i < 6 && keyword == (size_t)~0; ++i)
			{
				if (keywordStr.matchEqual(c_operations[i].opName))
					keyword = i;
			}

			if (keyword == (size_t)~0)
				pos = subString.find("@", pos + 1);
		}

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + c_operations[keyword].length);
			evaluateParamArgs(subString, argValues, syntaxError);

			syntaxError |= argValues.size() < 2 || argValues.size() > 3;

			if (!syntaxError)
			{
				IdString dstProperty;
				IdString srcProperty;
				int op1Value;
				int op2Value;

				dstProperty = argValues[0];
				size_t idx = 1;
				srcProperty = dstProperty;
				if (argValues.size() == 3)
					srcProperty = argValues[idx++];
				op1Value = properties.getProperty(srcProperty);
				op2Value = StringConverter::parseInt(argValues[idx],
					-std::numeric_limits<int>::max());

				if (op2Value == -std::numeric_limits<int>::max())
				{
					//Not a number, interpret as property
					op2Value = properties.getProperty(argValues[idx]);
				}

				int result = c_operations[keyword].opFunc(op1Value, op2Value);
				properties.setProperty(dstProperty, result);
			}
			else
			{
				size_t lineCount = calculateLineCount(subString);
				if (keyword <= 1)
				{
					printf("Syntax Error at line %zu: @%s expects one parameter",
						lineCount, c_operations[keyword].opName);
				}
				else
				{
					printf("Syntax Error at line %zu: @%s expects two or three parameters",
						lineCount, c_operations[keyword].opName);
				}
			}

			pos = subString.find("@");
			keyword = ~0;

			while (pos != String::npos && keyword == (size_t)~0)
			{
				size_t maxSize = subString.findFirstOf(" \t(", pos + 1);
				maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
				SubStringRef keywordStr(&inBuffer, subString.getStart() + pos + 1,
					subString.getStart() + maxSize);

				for (size_t i = 0; i < 6 && keyword == (size_t)~0; ++i)
				{
					if (keywordStr.matchEqual(c_operations[i].opName))
						keyword = i;
				}

				if (keyword == (size_t)~0)
					pos = subString.find("@", pos + 1);
			}
		}

		copy(outBuffer, subString, subString.getSize());

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::parseForEach(const String &inBuffer, String &outBuffer, PropertyMap &properties)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		StringVector argValues;
		SubStringRef subString(&inBuffer, 0);
		size_t pos = subString.find("@foreach");

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + sizeof("@foreach"));
			evaluateParamArgs(subString, argValues, syntaxError);

			SubStringRef blockSubString = subString;
			findBlockEnd(blockSubString, syntaxError);

			if (!syntaxError)
			{
				// Arg 1 (var)
				String counterVar;
				counterVar = argValues[0];

				// Agr 2 (start)
				size_t start;
				if (!StringConverter::parse(argValues[1], start))
				{
					//This isn't a number. Let's try if it's a variable
					start = properties.getProperty(argValues[1], 0);
				}

				// Arg 3 (count)
				size_t count;
				if (!StringConverter::parse(argValues[1], count))
				{
					//This isn't a number. Let's try if it's a variable
					count = properties.getProperty(argValues[2], 0);
				}

				// Repeat the block
				for (size_t i = 0; i < count; ++i)
					repeat(outBuffer, blockSubString, blockSubString.getSize(), start + i, counterVar);
			}

			subString.setStart(blockSubString.getEnd() + sizeof("@end"));
			pos = subString.find("@foreach");
		}

		copy(outBuffer, subString, subString.getSize());

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::parseProperties(String &inBuffer, String &outBuffer, PropertyMap &properties)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		SubStringRef subString(&inBuffer, 0);
		size_t pos = subString.find("@property");

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + sizeof("@property"));
			bool result = evaluateExpression(subString, syntaxError, properties);

			SubStringRef blockSubString = subString;
			findBlockEnd(blockSubString, syntaxError);

			if (result && !syntaxError)
				copy(outBuffer, blockSubString, blockSubString.getSize());

			subString.setStart(blockSubString.getEnd() + sizeof("@end") - 1);
			pos = subString.find("@property");
		}

		copy(outBuffer, subString, subString.getSize());

		while (!syntaxError && outBuffer.find("@property") != String::npos)
		{
			inBuffer.swap(outBuffer);
			syntaxError = parseProperties(inBuffer, outBuffer, properties);
		}

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::collectPieces(const String &inBuffer, String &outBuffer, PropertyMap &properties, PiecesMap& pieces)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		StringVector argValues;
		SubStringRef subString(&inBuffer, 0);
		size_t pos = subString.find("@piece");

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + sizeof("@piece"));
			evaluateParamArgs(subString, argValues, syntaxError);

			syntaxError |= argValues.size() != 1;

			if (!syntaxError)
			{
				const IdString pieceName(argValues[0]);
				PiecesMap::const_iterator it = pieces.find(pieceName);
				if (it != pieces.end())
				{
					syntaxError = true;
					printf("Error at line %zu: @piece '%s' already defined",
						calculateLineCount(subString), argValues[0].c_str());
				}
				else
				{
					SubStringRef blockSubString = subString;
					findBlockEnd(blockSubString, syntaxError);

					String tmpBuffer;
					copy(tmpBuffer, blockSubString, blockSubString.getSize());
					pieces[pieceName] = tmpBuffer;

					subString.setStart(blockSubString.getEnd() + sizeof("@end"));
				}
			}
			else
			{
				printf("Syntax Error at line %zu: @piece expects one parameter",
					calculateLineCount(subString));
			}

			pos = subString.find("@piece");
		}

		copy(outBuffer, subString, subString.getSize());

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::insertPieces(String &inBuffer, String &outBuffer, PropertyMap &properties, PiecesMap& pieces)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		StringVector argValues;
		SubStringRef subString(&inBuffer, 0);
		size_t pos = subString.find("@insertpiece");

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + sizeof("@insertpiece"));
			evaluateParamArgs(subString, argValues, syntaxError);

			syntaxError |= argValues.size() != 1;

			if (!syntaxError)
			{
				const IdString pieceName(argValues[0]);
				PiecesMap::const_iterator it = pieces.find(pieceName);
				if (it != pieces.end())
					outBuffer += it->second;
				else
				    LogManager::getSingleton().logError("Piece not found: "+argValues[0]);
			}
			else
			{
				printf("Syntax Error at line %zu: @insertpiece expects one parameter",
					calculateLineCount(subString));
			}

			pos = subString.find("@insertpiece");
		}

		copy(outBuffer, subString, subString.getSize());

		while (!syntaxError && outBuffer.find("@insertpiece") != String::npos)
		{
			inBuffer.swap(outBuffer);
			syntaxError = insertPieces(inBuffer, outBuffer, properties, pieces);
		}

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	const Operation c_counterOperations[8] =
	{
		Operation("counter", sizeof("@counter"), 0),
		Operation("value", sizeof("@value"), 0),
		Operation("set", sizeof("@set"), &setOp),
		Operation("add", sizeof("@add"), &addOp),
		Operation("sub", sizeof("@sub"), &subOp),
		Operation("mul", sizeof("@mul"), &mulOp),
		Operation("div", sizeof("@div"), &divOp),
		Operation("mod", sizeof("@mod"), &modOp)
	};
	//-----------------------------------------------------------------------------------
	bool ShaderGenerator::parseCounter(const String &inBuffer, String &outBuffer, PropertyMap &properties)
	{
		outBuffer.clear();
		outBuffer.reserve(inBuffer.size());

		StringVector argValues;
		SubStringRef subString(&inBuffer, 0);

		size_t pos;
		pos = subString.find("@");
		size_t keyword = ~0;

		if (pos != String::npos)
		{
			size_t maxSize = subString.findFirstOf(" \t(", pos + 1);
			SubStringRef keywordStr(&inBuffer, subString.getStart() + pos + 1,
				subString.getStart() + maxSize);

			for (size_t i = 0; i < 8 && keyword == (size_t)~0; ++i)
			{
				if (keywordStr.matchEqual(c_counterOperations[i].opName))
					keyword = i;
			}

			if (keyword == (size_t)~0)
				pos = String::npos;
		}

		bool syntaxError = false;

		while (pos != String::npos && !syntaxError)
		{
			//Copy what comes before the block
			copy(outBuffer, subString, pos);

			subString.setStart(subString.getStart() + pos + c_counterOperations[keyword].length);
			evaluateParamArgs(subString, argValues, syntaxError);

			if (keyword <= 1)
				syntaxError |= argValues.size() != 1;
			else
				syntaxError |= argValues.size() < 2 || argValues.size() > 3;

			if (!syntaxError)
			{
				IdString dstProperty;
				IdString srcProperty;
				int op1Value;
				int op2Value;

				if (argValues.size() == 1)
				{
					dstProperty = argValues[0];
					srcProperty = dstProperty;
					op1Value = properties.getProperty(srcProperty);
					op2Value = op1Value;

					//@value & @counter write, the others are invisible
					outBuffer += StringUtil::format("%i", op1Value);

					if (keyword == 0)
					{
						++op1Value;
						properties.setProperty(dstProperty, op1Value);
					}
				}
				else
				{
					dstProperty = argValues[0];
					size_t idx = 1;
					srcProperty = dstProperty;
					if (argValues.size() == 3)
						srcProperty = argValues[idx++];
					op1Value = properties.getProperty(srcProperty);
					op2Value = StringConverter::parseInt(argValues[idx],
						-std::numeric_limits<int>::max());

					if (op2Value == -std::numeric_limits<int>::max())
					{
						//Not a number, interpret as property
						op2Value = properties.getProperty(argValues[idx]);
					}

					int result = c_counterOperations[keyword].opFunc(op1Value, op2Value);
					properties.setProperty(dstProperty, result);
				}
			}
			else
			{
				size_t lineCount = calculateLineCount(subString);
				if (keyword <= 1)
				{
					printf("Syntax Error at line %zu: @%s expects one parameter",
						lineCount, c_counterOperations[keyword].opName);
				}
				else
				{
					printf("Syntax Error at line %zu: @%s expects two or three parameters",
						lineCount, c_counterOperations[keyword].opName);
				}
			}

			pos = subString.find("@");
			keyword = ~0;

			if (pos != String::npos)
			{
				size_t maxSize = subString.findFirstOf(" \t(", pos + 1);
				SubStringRef keywordStr(&inBuffer, subString.getStart() + pos + 1,
					subString.getStart() + maxSize);

				for (size_t i = 0; i < 8 && keyword == (size_t)~0; ++i)
				{
					if (keywordStr.matchEqual(c_counterOperations[i].opName))
						keyword = i;
				}

				if (keyword == (size_t)~0)
					pos = String::npos;
			}
		}

		copy(outBuffer, subString, subString.getSize());

		return syntaxError;
	}
	//-----------------------------------------------------------------------------------
	String ShaderGenerator::parse(String &inBuffer, PropertyMap &properties, const StringVector& pieceFiles)
	{
		String outBuffer;
		outBuffer.reserve(inBuffer.size());

		PiecesMap pieces;

		if (!pieceFiles.empty())
		{
			//Collect pieces
			StringVector::const_iterator itor = pieceFiles.begin();
			StringVector::const_iterator end = pieceFiles.end();

			String inPiece;
			String outPiece;

			while (itor != end)
			{
				parseMath(*itor, inPiece, properties);
				parseForEach(inPiece, outPiece, properties);
				parseProperties(outPiece, inPiece, properties);
				collectPieces(inPiece, outPiece, properties, pieces);
				++itor;
			}
		}

		parseMath(inBuffer, outBuffer, properties);
		parseForEach(outBuffer, inBuffer, properties);
		parseProperties(inBuffer, outBuffer, properties);
		collectPieces(outBuffer, inBuffer, properties, pieces);
		insertPieces(inBuffer, outBuffer, properties, pieces);
		parseCounter(outBuffer, inBuffer, properties);

		outBuffer.swap(inBuffer);

		return outBuffer;
	}
	//-----------------------------------------------------------------------------------
	size_t ShaderGenerator::calculateLineCount(const String &buffer, size_t idx)
	{
		String::const_iterator itor = buffer.begin();
		String::const_iterator end = buffer.begin() + idx;

		size_t lineCount = 0;

		while (itor != end)
		{
			if (*itor == '\n')
				++lineCount;
			++itor;
		}

		return lineCount + 1;
	}
	//-----------------------------------------------------------------------------------
	size_t ShaderGenerator::calculateLineCount(const SubStringRef &subString)
	{
		return calculateLineCount(subString.getOriginalBuffer(), subString.getStart());
	}
	//-----------------------------------------------------------------------------------
}
