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

#pragma once

#include "Ogre.h"		 
#include "OgreHlmsPrerequisites.h"
#include "OgreHlmsPropertyMap.h"
#include "OgreHlmsShaderCommon.h"
#include "OgreIdString.h"

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
    class SubStringRef;

	class _OgreHlmsExport ShaderGenerator : public PassAlloc
	{
	protected:

		typedef std::map<IdString, String> PiecesMap;

		enum ExpressionType
		{
			EXPR_OPERATOR_OR,        //||
			EXPR_OPERATOR_AND,       //&&
			EXPR_OBJECT,             //(...)
			EXPR_VAR
		};

		struct Expression
		{
			bool                    result;
			bool                    negated;
			ExpressionType          type;
			std::vector<Expression> children;
			String            value;

			Expression() : result(false), negated(false), type(EXPR_VAR) {}
		};

		typedef std::vector<Expression> ExpressionVec;

		static void copy(String &outBuffer, const SubStringRef &inSubString, size_t length);
		static void repeat(String &outBuffer, const SubStringRef &inSubString, size_t length,
			size_t passNum, const String &counterVar);
		static bool parseMath(const String &inBuffer, String &outBuffer, PropertyMap &properties);
		static bool parseForEach(const String &inBuffer, String &outBuffer, PropertyMap &properties);
		static bool parseProperties(String &inBuffer, String &outBuffer, PropertyMap &properties);
		static bool collectPieces(const String &inBuffer, String &outBuffer, PropertyMap &properties, PiecesMap& pieces);
		static bool insertPieces(String &inBuffer, String &outBuffer, PropertyMap &properties, PiecesMap& pieces);
		static bool parseCounter(const String &inBuffer, String &outBuffer, PropertyMap &properties);

		static void findBlockEnd(SubStringRef &outSubString, bool &syntaxError);

		static bool evaluateExpression(SubStringRef &outSubString, bool &outSyntaxError, PropertyMap &properties);
		static bool evaluateExpressionRecursive(ExpressionVec &expression, bool &outSyntaxError, PropertyMap &properties);
		static size_t evaluateExpressionEnd(const SubStringRef &outSubString);

		static void evaluateParamArgs(SubStringRef &outSubString, StringVector &outArgs,
			bool &outSyntaxError);

		static size_t calculateLineCount(const String &buffer, size_t idx);
		static size_t calculateLineCount(const SubStringRef &subString);

	public:
		static String parse(String &inBuffer, PropertyMap &properties, const StringVector& pieceFiles);
	};
}
