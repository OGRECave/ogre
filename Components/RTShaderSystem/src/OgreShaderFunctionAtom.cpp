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

#include "OgreShaderFunctionAtom.h"
#include "OgreRoot.h"

namespace Ogre {
namespace RTShader {
//-----------------------------------------------------------------------------
Operand::Operand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask, ushort indirectionLevel) : mParameter(parameter), mSemantic(opSemantic), mMask(opMask), mIndirectionLevel(indirectionLevel)
{
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
		mIndirectionLevel = other.mIndirectionLevel;
	}		
	return *this;
}
//-----------------------------------------------------------------------------
Operand::~Operand()
{
	// nothing todo
}
//-----------------------------------------------------------------------------
String Operand::getMaskAsString(int mask)
{
	String retVal = "";

	if (mask & ~OPM_ALL) 
	{
		if (mask & OPM_X)
		{
			retVal += "x";
		}

		if (mask & OPM_Y)
		{
			retVal += "y";
		}

		if (mask & OPM_Z)
		{
			retVal += "z";
		}

		if (mask & OPM_W)
		{
			retVal += "w";
		}
	}

	return retVal;
}

//-----------------------------------------------------------------------------
int Operand::getFloatCount(int mask)
{
	int floatCount = 0;

	while (mask != 0)
	{
		if ((mask & Operand::OPM_X) != 0)
		{
			floatCount++;

		}			
		mask = mask >> 1;
	}

	return floatCount;
}

//-----------------------------------------------------------------------------
GpuConstantType	Operand::getGpuConstantType(int mask)
{
	int floatCount = getFloatCount(mask);
	GpuConstantType type;

	switch (floatCount)
	{

	case 1:
		type = GCT_FLOAT1;
		break;

	case 2:
		type = GCT_FLOAT2;
		break;

	case 3:
		type = GCT_FLOAT3;
		break;

	case 4:
		type = GCT_FLOAT4;
		break;

	default:
		type = GCT_UNKNOWN;
		break;
	}

	return type;
}

//-----------------------------------------------------------------------------
String Operand::toString() const
{
	String retVal = mParameter->toString();
	if ((mMask & OPM_ALL) || ((mMask & OPM_X) && (mMask & OPM_Y) && (mMask & OPM_Z) && (mMask & OPM_W)))
	{
		return retVal;
	}

	retVal += "." + getMaskAsString(mMask);

	return retVal;
}

//-----------------------------------------------------------------------------
FunctionAtom::FunctionAtom()
{
	mGroupExecutionOrder   = -1;
	mInternalExecutionOrder = -1;
}

//-----------------------------------------------------------------------------
int FunctionAtom::getGroupExecutionOrder() const
{
	return mGroupExecutionOrder;
}

//-----------------------------------------------------------------------------
int	FunctionAtom::getInternalExecutionOrder() const
{
	return mInternalExecutionOrder;
}


String FunctionInvocation::Type = "FunctionInvocation";

//-----------------------------------------------------------------------
FunctionInvocation::FunctionInvocation(const String& functionName, 
									   int groupOrder, int internalOrder, String returnType) :
    mFunctionName(functionName), mReturnType(returnType)
{
	mGroupExecutionOrder = groupOrder;
	mInternalExecutionOrder = internalOrder;
}

//-----------------------------------------------------------------------------
FunctionInvocation::FunctionInvocation(const FunctionInvocation& other) :
    mFunctionName(other.mFunctionName), mReturnType(other.mReturnType)
{
	mGroupExecutionOrder = other.mGroupExecutionOrder;
	mInternalExecutionOrder = other.mInternalExecutionOrder;
    
    for ( OperandVector::const_iterator it = other.mOperands.begin(); it != other.mOperands.end(); ++it)
        mOperands.push_back(Operand(*it));
}

//-----------------------------------------------------------------------
void FunctionInvocation::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
	// Write function name.
	os << mFunctionName << "(";

	// Write parameters.
	ushort curIndLevel = 0;
	for (OperandVector::const_iterator it = mOperands.begin(); it != mOperands.end(); )
	{
		os << (*it).toString();
		++it;

		ushort opIndLevel = 0;
		if (it != mOperands.end())
		{
			opIndLevel = (*it).getIndirectionLevel();
		}

		if (curIndLevel < opIndLevel)
		{
			while (curIndLevel < opIndLevel)
			{
				++curIndLevel;
				os << "[";
			}
		}
		else //if (curIndLevel >= opIndLevel)
		{
			while (curIndLevel > opIndLevel)
			{
				--curIndLevel;
				os << "]";
			}
			if (opIndLevel != 0)
			{
				os << "][";
			}
			else if (it != mOperands.end())
			{
				os << ", ";
			}
		}
	}

	// Write function call closer.
	os << ");";
}

//-----------------------------------------------------------------------
void FunctionInvocation::pushOperand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask, int indirectionLevel)
{
	mOperands.push_back(Operand(parameter, opSemantic, opMask, indirectionLevel));
}

//-----------------------------------------------------------------------
bool FunctionInvocation::operator == ( const FunctionInvocation& rhs ) const
{
    return FunctionInvocationCompare()(*this, rhs);
}

//-----------------------------------------------------------------------
bool FunctionInvocation::operator != ( const FunctionInvocation& rhs ) const
{
    return !(*this == rhs);
}

//-----------------------------------------------------------------------
bool FunctionInvocation::operator < ( const FunctionInvocation& rhs ) const
{
    return FunctionInvocationLessThan()(*this, rhs);
}

bool FunctionInvocation::FunctionInvocationLessThan::operator ()(FunctionInvocation const& lhs, FunctionInvocation const& rhs) const
{
    // Check the function names first
    // Adding an exception to std::string sorting.  I feel that functions beginning with an underscore should be placed before
    // functions beginning with an alphanumeric character.  By default strings are sorted based on the ASCII value of each character.
    // Underscores have an ASCII value in between capital and lowercase characters.  This is why the exception is needed.
    if (lhs.getFunctionName() < rhs.getFunctionName())
    {
        if(rhs.getFunctionName().at(0) == '_')
            return false;
        else
            return true;
    }
    if (lhs.getFunctionName() > rhs.getFunctionName())
    {
        if(lhs.getFunctionName().at(0) == '_')
            return true;
        else
            return false;
    }

    // Next check the return type
    if (lhs.getReturnType() < rhs.getReturnType())
        return true;
    if (lhs.getReturnType() > rhs.getReturnType())
        return false;

    // Check the number of operands
    if (lhs.mOperands.size() < rhs.mOperands.size())
        return true;
    if (lhs.mOperands.size() > rhs.mOperands.size())
        return false;

    // Now that we've gotten past the two quick tests, iterate over operands
    // Check the semantic and type.  The operands must be in the same order as well.
    OperandVector::const_iterator itLHSOps = lhs.mOperands.begin();
    OperandVector::const_iterator itRHSOps = rhs.mOperands.begin();

    for ( ; ((itLHSOps != lhs.mOperands.end()) && (itRHSOps != rhs.mOperands.end())); ++itLHSOps, ++itRHSOps)
    {
        if (itLHSOps->getSemantic() < itRHSOps->getSemantic())
            return true;
        if (itLHSOps->getSemantic() > itRHSOps->getSemantic())
            return false;

        GpuConstantType leftType    = itLHSOps->getParameter()->getType();
        GpuConstantType rightType   = itRHSOps->getParameter()->getType();
        
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != String::npos)
        {
            if (leftType == GCT_SAMPLER1D)
                leftType = GCT_SAMPLER2D;

            if (rightType == GCT_SAMPLER1D)
                rightType = GCT_SAMPLER2D;
        }

        // If a swizzle mask is being applied to the parameter, generate the GpuConstantType to
        // perform the parameter type comparison the way that the compiler will see it.
        if ((itLHSOps->getFloatCount(itLHSOps->getMask()) > 0) ||
           (itRHSOps->getFloatCount(itRHSOps->getMask()) > 0))
        {
            if (itLHSOps->getFloatCount(itLHSOps->getMask()) > 0)
            {
                leftType = (GpuConstantType)((itLHSOps->getParameter()->getType() - itLHSOps->getParameter()->getType()) +
                                             itLHSOps->getFloatCount(itLHSOps->getMask()));
            }
            if (itRHSOps->getFloatCount(itRHSOps->getMask()) > 0)
            {
                rightType = (GpuConstantType)((itRHSOps->getParameter()->getType() - itRHSOps->getParameter()->getType()) +
                                             itRHSOps->getFloatCount(itRHSOps->getMask()));
            }
        }

        if (leftType < rightType)
            return true;
        if (leftType > rightType)
            return false;
    }

    return false;
}

bool FunctionInvocation::FunctionInvocationCompare::operator ()(FunctionInvocation const& lhs, FunctionInvocation const& rhs) const
{
    // Check the function names first
    if (lhs.getFunctionName() != rhs.getFunctionName())
        return false;

    // Next check the return type
    if (lhs.getReturnType() != rhs.getReturnType())
        return false;

    // Check the number of operands
    if (lhs.mOperands.size() != rhs.mOperands.size())
        return false;

    // Now that we've gotten past the two quick tests, iterate over operands
    // Check the semantic and type.  The operands must be in the same order as well.
    OperandVector::const_iterator itLHSOps = lhs.mOperands.begin();
    OperandVector::const_iterator itRHSOps = rhs.mOperands.begin();
    for ( ; ((itLHSOps != lhs.mOperands.end()) && (itRHSOps != rhs.mOperands.end())); ++itLHSOps, ++itRHSOps)
    {
        if (itLHSOps->getSemantic() != itRHSOps->getSemantic())
            return false;

        GpuConstantType leftType    = itLHSOps->getParameter()->getType();
        GpuConstantType rightType   = itRHSOps->getParameter()->getType();
        
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != String::npos)
        {
            if (leftType == GCT_SAMPLER1D)
                leftType = GCT_SAMPLER2D;

            if (rightType == GCT_SAMPLER1D)
                rightType = GCT_SAMPLER2D;
        }

        // If a swizzle mask is being applied to the parameter, generate the GpuConstantType to
        // perform the parameter type comparison the way that the compiler will see it.
        if ((itLHSOps->getFloatCount(itLHSOps->getMask()) > 0) ||
           (itRHSOps->getFloatCount(itRHSOps->getMask()) > 0))
        {
            if (itLHSOps->getFloatCount(itLHSOps->getMask()) > 0)
            {
                leftType = (GpuConstantType)((itLHSOps->getParameter()->getType() - itLHSOps->getParameter()->getType()) +
                                             itLHSOps->getFloatCount(itLHSOps->getMask()));
            }
            if (itRHSOps->getFloatCount(itRHSOps->getMask()) > 0)
            {
                rightType = (GpuConstantType)((itRHSOps->getParameter()->getType() - itRHSOps->getParameter()->getType()) +
                                             itRHSOps->getFloatCount(itRHSOps->getMask()));
            }
        }

        if (leftType != rightType)
            return false;
    }

    // Passed all tests, they are the same
    return true;
}

}
}
