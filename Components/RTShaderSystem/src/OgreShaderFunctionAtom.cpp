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

#include "OgreShaderFunctionAtom.h"

namespace Ogre {
namespace RTShader {
//-----------------------------------------------------------------------------
Operand::Operand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask)
{
	mParameter = parameter;
	mSemantic = opSemantic;
	mMask = opMask;
}
//-----------------------------------------------------------------------------
Operand::Operand(const Operand& other) 
{
	*this = other;
}
//-----------------------------------------------------------------------------
Operand& Operand::operator= (const Operand & other)
{
	if (this != &other) 
	{
		mParameter = other.mParameter;
		mSemantic = other.mSemantic;
		mMask = other.mMask;
	}		
	return *this;
}
//-----------------------------------------------------------------------------
Operand::~Operand()
{
	// nothing todo
}
//-----------------------------------------------------------------------------
String Operand::toString() const
{
	String retVal = mParameter->toString();
	if ((mMask & OPM_ALL) || ((mMask & OPM_X) && (mMask & OPM_Y) && (mMask & OPM_Z) && (mMask & OPM_W)))
	{
		return retVal;
	}

	retVal += ".";

	if (mMask & OPM_X)
	{
		retVal += "x";
	}

	if (mMask & OPM_Y)
	{
		retVal += "y";
	}

	if (mMask & OPM_Z)
	{
		retVal += "z";
	}

	if (mMask & OPM_W)
	{
		retVal += "w";
	}

	return retVal;
}

//-----------------------------------------------------------------------------
FunctionAtom::FunctionAtom()
{
	mGroupExecutionOrder   = -1;
	mInteralExecutionOrder = -1;
}

//-----------------------------------------------------------------------------
int FunctionAtom::getGroupExecutionOrder() const
{
	return mGroupExecutionOrder;
}

//-----------------------------------------------------------------------------
int	FunctionAtom::getInternalExecutionOrder() const
{
	return mInteralExecutionOrder;
}


String FunctionInvocation::Type = "FunctionInvocation";

//-----------------------------------------------------------------------
FunctionInvocation::FunctionInvocation(const String& functionName, 
									   int groupOrder, int internalOrder, String returnType)
{
	mFunctionName = functionName;
	mGroupExecutionOrder = groupOrder;
	mInteralExecutionOrder = internalOrder;
	mReturnType = returnType;
}

//-----------------------------------------------------------------------
void FunctionInvocation::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
	// Write function name.
	os << mFunctionName << "(";

	// Write parameters.
	for (OperandVector::const_iterator it = mOperands.begin(); it != mOperands.end(); )
	{
		os << (*it).toString();

		++it;

		if (it != mOperands.end())
		{
			os << ", ";
		}
	}

	// Write function call closer.
	os << ");";
}

//-----------------------------------------------------------------------
void FunctionInvocation::pushOperand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask)
{
	mOperands.push_back(Operand(parameter, opSemantic, opMask));
}

}
}