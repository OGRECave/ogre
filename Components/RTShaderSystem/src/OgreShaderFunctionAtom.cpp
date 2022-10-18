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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {
//-----------------------------------------------------------------------------
Operand::Operand(ParameterPtr parameter, OpSemantic opSemantic, OpMask opMask, ushort indirectionLevel) : mParameter(parameter), mSemantic(opSemantic), mMask(opMask), mIndirectionLevel(indirectionLevel)
{
    // delay null check until FunctionInvocation
    if(parameter)
        parameter->setUsed(true);
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
    // nothing to do
}

void Operand::setMaskToParamType()
{
    switch (mParameter->getType())
    {
    case GCT_FLOAT1:
        mMask = OPM_X;
        break;
    case GCT_FLOAT2:
        mMask = OPM_XY;
        break;
    case GCT_FLOAT3:
        mMask = OPM_XYZ;
        break;
    default:
        mMask = OPM_ALL;
        break;
    }
}

//-----------------------------------------------------------------------------
static void writeMask(std::ostream& os, int mask)
{
    if (mask != Operand::OPM_ALL)
    {
        os << '.';
        if (mask & Operand::OPM_X)
        {
            os << 'x';
        }

        if (mask & Operand::OPM_Y)
        {
            os << 'y';
        }

        if (mask & Operand::OPM_Z)
        {
            os << 'z';
        }

        if (mask & Operand::OPM_W)
        {
            os << 'w';
        }
    }
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
void Operand::write(std::ostream& os) const
{
    os << mParameter->toString();
    writeMask(os, mMask);
}

//-----------------------------------------------------------------------------
FunctionAtom::FunctionAtom()
{
    mGroupExecutionOrder   = -1;
}

//-----------------------------------------------------------------------------
int FunctionAtom::getGroupExecutionOrder() const
{
    return mGroupExecutionOrder;
}

//-----------------------------------------------------------------------
FunctionInvocation::FunctionInvocation(const String& functionName, int groupOrder,
                                       const String& returnType)
    : mReturnType(returnType)
{
    mFunctionName = functionName;
    mGroupExecutionOrder = groupOrder;
}

//-----------------------------------------------------------------------------
FunctionInvocation::FunctionInvocation(const FunctionInvocation& other) :
    mReturnType(other.mReturnType)
{
    mFunctionName = other.mFunctionName;
    mGroupExecutionOrder = other.mGroupExecutionOrder;
    
    for ( OperandVector::const_iterator it = other.mOperands.begin(); it != other.mOperands.end(); ++it)
        mOperands.push_back(Operand(*it));
}

//-----------------------------------------------------------------------
void FunctionInvocation::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
    // Write function name.
    os << mFunctionName << "(";
    writeOperands(os, mOperands.begin(), mOperands.end());
    // Write function call closer.
    os << ");";
}

//-----------------------------------------------------------------------
static String parameterNullMsg(const String& name, size_t pos)
{
    return StringUtil::format("%s: parameter #%zu is NULL", name.c_str(), pos);
}

void FunctionAtom::pushOperand(ParameterPtr parameter, Operand::OpSemantic opSemantic, Operand::OpMask opMask, int indirectionLevel)
{
    if (!parameter)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, parameterNullMsg(mFunctionName, mOperands.size()));
    mOperands.push_back(Operand(parameter, opSemantic, opMask, indirectionLevel));
}

void FunctionAtom::setOperands(const OperandVector& ops)
{
    for (size_t i = 0; i < ops.size(); i++)
        if(!ops[i].getParameter())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, parameterNullMsg(mFunctionName, i));

    mOperands = ops;
}

void FunctionAtom::writeOperands(std::ostream& os, OperandVector::const_iterator begin,
                                       OperandVector::const_iterator end) const
{
    // Write parameters.
    ushort curIndLevel = 0;
    for (OperandVector::const_iterator it = begin; it != end; )
    {
        it->write(os);
        ++it;

        ushort opIndLevel = 0;
        if (it != mOperands.end())
        {
            opIndLevel = it->getIndirectionLevel();
        }

        if (curIndLevel != 0)
        {
            os << ")";
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
            else if (it != end)
            {
                os << ", ";
            }
        }
        if (curIndLevel != 0)
        {
            os << "int("; // required by GLSL
        }
    }
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

static uchar getSwizzledSize(const Operand& op)
{
    auto gct = op.getParameter()->getType();
    if (op.getMask() == Operand::OPM_ALL)
        return GpuConstantDefinition::getElementSize(gct, false);

    return Operand::getFloatCount(op.getMask());
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

        uchar leftType    = getSwizzledSize(*itLHSOps);
        uchar rightType   = getSwizzledSize(*itRHSOps);

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

    // filter indirect operands
    std::vector<const Operand*> lhsOps;
    std::vector<const Operand*> rhsOps;
    for(const Operand& op : lhs.mOperands)
        if(op.getIndirectionLevel() == 0) lhsOps.push_back(&op);
    for(const Operand& op : rhs.mOperands)
        if(op.getIndirectionLevel() == 0) rhsOps.push_back(&op);

    // Check the number of direct operands
    if (lhsOps.size() != rhsOps.size())
        return false;

    // Now that we've gotten past the two quick tests, iterate over operands
    // Check the semantic and type.  The operands must be in the same order as well.
    auto itLHSOps = lhsOps.begin();
    auto itRHSOps = rhsOps.begin();
    for ( ; ((itLHSOps != lhsOps.end()) && (itRHSOps != rhsOps.end())); ++itLHSOps, ++itRHSOps)
    {
        if ((*itLHSOps)->getSemantic() != (*itRHSOps)->getSemantic())
            return false;

        if (getSwizzledSize(**itLHSOps) != getSwizzledSize(**itRHSOps))
            return false;
    }

    // Passed all tests, they are the same
    return true;
}

AssignmentAtom::AssignmentAtom(const Out& lhs, const In& rhs, int groupOrder) {
    // do this backwards for compatibility with FFP_FUNC_ASSIGN calls
    setOperands({rhs, lhs});
    mGroupExecutionOrder = groupOrder;
    mFunctionName = "assign";
}

void AssignmentAtom::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
    OperandVector::const_iterator outOp = mOperands.begin();
    // find the output operand
    while(outOp->getSemantic() != Operand::OPS_OUT)
        outOp++;
    writeOperands(os, outOp, mOperands.end());
    os << "\t=\t";
    writeOperands(os, mOperands.begin(), outOp);
    os << ";";
}

SampleTextureAtom::SampleTextureAtom(const In& sampler, const In& texcoord, const Out& lhs, int groupOrder)
{
    setOperands({sampler, texcoord, lhs});
    mGroupExecutionOrder = groupOrder;
    mFunctionName = "sampleTexture";
}

void SampleTextureAtom::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
    OperandVector::const_iterator outOp = mOperands.begin();
    // find the output operand
    while(outOp->getSemantic() != Operand::OPS_OUT)
        outOp++;
    writeOperands(os, outOp, mOperands.end());
    os << "\t=\t";

    os << "texture";
    const auto& sampler = mOperands.front().getParameter();
    switch(sampler->getType())
    {
    case GCT_SAMPLER1D:
        os << "1D";
        break;
    case GCT_SAMPLER_EXTERNAL_OES:
    case GCT_SAMPLER2D:
        os << "2D";
        break;
    case GCT_SAMPLER3D:
        os << "3D";
        break;
    case GCT_SAMPLERCUBE:
        os << "Cube";
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "unknown sampler");
        break;
    }

    os << "(";
    writeOperands(os, mOperands.begin(), outOp);
    os << ");";
}

BinaryOpAtom::BinaryOpAtom(char op, const In& a, const In& b, const Out& dst, int groupOrder) {
    // do this backwards for compatibility with FFP_FUNC_ASSIGN calls
    setOperands({a, b, dst});
    mGroupExecutionOrder = groupOrder;
    mOp = op;
    mFunctionName = op;
}

void BinaryOpAtom::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
    // find the output operand
    OperandVector::const_iterator outOp = mOperands.begin();
    while(outOp->getSemantic() != Operand::OPS_OUT)
        outOp++;

    // find the second operand
    OperandVector::const_iterator secondOp = ++(mOperands.begin());
    while(outOp->getIndirectionLevel() != 0)
        secondOp++;

    writeOperands(os, outOp, mOperands.end());
    os << "\t=\t";
    writeOperands(os, mOperands.begin(), secondOp);
    os << mOp;
    writeOperands(os, secondOp, outOp);
    os << ";";
}

void BuiltinFunctionAtom::writeSourceCode(std::ostream& os, const String& targetLanguage) const
{
    // find the output operand
    OperandVector::const_iterator outOp = mOperands.begin();
    while(outOp->getSemantic() != Operand::OPS_OUT)
        outOp++;

    writeOperands(os, outOp, mOperands.end());
    os << "\t=\t" << mFunctionName << "(";
    writeOperands(os, mOperands.begin(), outOp);
    os << ");";
}

}
}
