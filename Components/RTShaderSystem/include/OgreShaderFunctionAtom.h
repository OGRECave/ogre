/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _ShaderFunctionAtom_
#define _ShaderFunctionAtom_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"
#include "OgreShaderParameter.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that represents an atomic code section of shader based program function.
*/
class _OgreRTSSExport FunctionAtom : public RTShaderSystemAlloc
{
// Interface.
public:
	/** Class default constructor. */
	FunctionAtom();

	/** Class default destructor. */
	virtual ~FunctionAtom() {}

	/** Get the group execution order of this function atom. */
	int getGroupExecutionOrder() const;
	
	/** Get an internal execution order within a group of this function atom. */
	int getInternalExecutionOrder() const;
	
	/** Abstract method that writes a source code to the given output stream in the target shader language. */
	virtual void writeSourceCode(std::ostream& os, const String& targetLanguage) const = 0;
	
	/** Return the type of this atom instance implementation. */
	virtual const String& getFunctionAtomType() = 0;

// Attributes.
protected:
	// The owner group execution order.	
	int mGroupExecutionOrder;
	// The execution order within the group.		
	int mInternalExecutionOrder;
};

/** A class that represents a function operand (its the combination of a parameter the in/out semantic and the used fields)
*/
class _OgreRTSSExport Operand : public RTShaderSystemAlloc
{
public:

	// InOut semantic
	enum OpSemantic
	{
		/// The parameter is a input parameter
		OPS_IN, 
		/// The parameter is a output parameter
		OPS_OUT,
		/// The parameter is a input/output parameter
		OPS_INOUT
	};

	// Used field mask
	enum OpMask
	{
		OPM_ALL		= 0x0001,
		OPM_X		= 0x0002,
		OPM_Y		= 0x0004,
		OPM_Z		= 0x0008,
		OPM_W		= 0x0010,
		OPM_XY		= OPM_X | OPM_Y,
		OPM_XZ		= OPM_X | OPM_Z,
		OPM_XW		= OPM_X | OPM_W,
		OPM_YZ		= OPM_Y | OPM_Z,
		OPM_YW		= OPM_Y | OPM_W,
		OPM_ZW		= OPM_Z | OPM_W,
		OPM_XYZ		= OPM_X | OPM_Y | OPM_Z,
		OPM_XYW		= OPM_X | OPM_Y | OPM_W,
		OPM_XZW		= OPM_X | OPM_Z | OPM_W,
		OPM_YZW		= OPM_Y | OPM_Z | OPM_W,
		OPM_XYZW	= OPM_X | OPM_Y | OPM_Z | OPM_W
	};

	/** Class constructor 
	@param parameter A function parameter.
	@param opSemantic The in/out semantic of the parameter.
	@param opMask The field mask of the parameter.
	*/
	Operand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask = Operand::OPM_ALL, ushort indirectionLevel = 0);

	/** Copy constructor */
	Operand(const Operand& rhs);

	/** Copy the given Operand to this Operand.
	@param rhs The other Operand to copy to this state.
	*/
	Operand& operator= (const Operand & rhs);

	/** Class destructor */
	~Operand();

	/** Returns the parameter object as weak reference */
	const ParameterPtr& getParameter()	const { return mParameter; }

	/** Returns true if not all fields used. (usage is described through semantic)*/
	bool hasFreeFields()	const { return ((mMask & ~OPM_ALL) && ((mMask & ~OPM_X) || (mMask & ~OPM_Y) || (mMask & ~OPM_Z) || (mMask & ~OPM_W))); }
	
	/** Returns the mask bitfield. */
	int getMask()	const { return mMask; }

	/** Returns the operand semantic (do we read/write or both with the parameter). */
	OpSemantic getSemantic()	const { return mSemantic; }

	/** Returns the level of indirection. 
	The greater the indirection level the more the parameter needs to be nested in brackets.
	For example given 4 parameters x1...x4 with the indirections levels 0,1,1,2 
	respectively. The parameters should form the following string: x1[x2][x3[x4]].
	*/
	ushort getIndirectionLevel()	const { return mIndirectionLevel; }

	/** Returns the parameter name and the usage mask like this 'color.xyz' */
	String toString()	const;

	/** Returns the given mask as string representation. */
	static String getMaskAsString(int mask);

	/** Return the float count of the given mask. */
	static int getFloatCount(int mask);

	/** Return the gpu constant type of the given mask. */
	static GpuConstantType getGpuConstantType(int mask);

protected:
	/// The parameter being carried by the operand
	ParameterPtr mParameter;
	/// Tells if the parameter is of type input,output or both
	OpSemantic mSemantic;
	/// Which part of the parameter should be passed (x,y,z,w)
	int mMask;
	/// The level of indirection. @see getIndirectionLevel
	ushort mIndirectionLevel;
};

/** A class that represents function invocation code from shader based program function.
*/
class _OgreRTSSExport FunctionInvocation : public FunctionAtom
{
	// Interface.
public:	
	typedef vector<Operand>::type OperandVector;

	/** Class constructor 
	@param functionName The name of the function to invoke.
	@param groupOrder The group order of this invocation.
	@param internalOrder The internal order of this invocation.
	@param returnType The return type of the used function.
	*/
	FunctionInvocation(const String& functionName, int groupOrder, int internalOrder, String returnType = "void");

    /** Copy constructor */
	FunctionInvocation(const FunctionInvocation& rhs);

	/** 
	@see FunctionAtom::writeSourceCode
	*/
	virtual void writeSourceCode(std::ostream& os, const String& targetLanguage) const;

	/** 
	@see FunctionAtom::getFunctionAtomType
	*/
	virtual const String& getFunctionAtomType() { return Type; }

	/** Get a list of parameters this function invocation will use in the function call as arguments. */
	OperandVector& getOperandList() { return mOperands; }
	
	/** Push a new operand (on the end) to the function. 
	@param parameter A function parameter.
	@param opSemantic The in/out semantic of the parameter.
	@param opMask The field mask of the parameter.
	@param indirectionLevel The level of nesting inside brackets
	*/
	void pushOperand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask = Operand::OPM_ALL, int indirectionLevel = 0);

	/** Return the function name */
	const String& getFunctionName() const { return mFunctionName; }

	/** Return the return type */
	const String& getReturnType() const { return mReturnType; }

    /** Determines if the current object is equal to the compared one. */
    bool operator == ( const FunctionInvocation& rhs ) const;

    /** Determines if the current object is not equal to the compared one. */
    bool operator != ( const FunctionInvocation& rhs ) const;

    /** Determines if the current object is less than the compared one. */
    bool operator <  ( const FunctionInvocation& rhs ) const;

    /** Comparator function to be used for sorting.
        Implemented as a struct to make it easier for the compiler to inline
    */
    struct FunctionInvocationLessThan
    {
        bool operator()(FunctionInvocation const& lhs, FunctionInvocation const& rhs) const;
    };

    /** Comparator function to be used for comparisons.
        Implemented as a struct to make it easier for the compiler to inline
    */
    struct FunctionInvocationCompare
    {
        bool operator()(FunctionInvocation const& lhs, FunctionInvocation const& rhs) const;
    };

	/// The type of this class.
	static String Type;

	// Attributes.
protected:	
	String mFunctionName;
	String mReturnType;
	OperandVector mOperands;	
};

typedef vector<FunctionAtom*>::type					FunctionAtomInstanceList;
typedef FunctionAtomInstanceList::iterator 			FunctionAtomInstanceIterator;
typedef FunctionAtomInstanceList::const_iterator	FunctionAtomInstanceConstIterator;

/** @} */
/** @} */

}
}

#endif
