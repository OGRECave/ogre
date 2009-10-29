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
class FunctionAtom
{
// Interface.
public:

	/** Get the group execution order of this function atom. */
	int						getGroupExecutionOrder		() const;
	
	/** Get an internal execution order within a group of this function atom. */
	int						getInternalExecutionOrder	() const;
	
	/** Abstract method that writes a source code to the given output stream in the target shader language. */
	virtual void			writeSourceCode				(std::ostream& os, const String& targetLanguage) const = 0;
	
protected:
	/** Class default constructor */
	FunctionAtom		();

// Attributes.
protected:
	int			mGroupExecutionOrder;		// The owner group execution order.	
	int			mInteralExecutionOrder;		// The execution order within the group.		
};

/** A class that represents a function operand (its the combination of a parameter the in/out semantic and the used fields)
*/
class Operand
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
		OPM_ALL = 1,
		OPM_X = 2,
		OPM_Y = 4,
		OPM_Z = 8,
		OPM_W = 16
	};

	/** Class constructor 
	@param parameter A function parameter.
	@param opSemantic The in/out semantic of the parameter.
	@param opMask The field mask of the parameter.
	*/
	Operand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask = Operand::OPM_ALL);

	/** Copy constructor */
	Operand(const Operand& rhs);

	/** Copy the given Operand to this Operand.
	@param rhs The other Operand to copy to this state.
	*/
	Operand& operator= (const Operand & rhs);

	/** Class destructor */
	~Operand();

	/** Returns the parameter object as weak reference */
	const ParameterPtr& getParameter	()	const { return mParameter; }

	/** Returns true if not all fields used. (usage is described through semantic)*/
	bool				hasFreeFields	()	const { return ((mMask & ~OPM_ALL) && ((mMask & ~OPM_X) || (mMask & ~OPM_Y) || (mMask & ~OPM_Z) || (mMask & ~OPM_W))); }
	
	/** Returns the mask bitfield. */
	int					getMask			()	const { return mMask; }

	/** Returns the operand semantic (do we read/write or both with the parameter). */
	OpSemantic			getSemantic		()	const { return mSemantic; }

	/** Returns the parameter name and the usage mask like this 'color.xyz' */
	String				toString		()	const;

protected:
	ParameterPtr	mParameter;
	OpSemantic		mSemantic;
	int				mMask;
};

/** A class that represents function invocation code from shader based program function.
*/
class FunctionInvocation : public FunctionAtom
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

	/** 
	@see FunctionAtom::writeSourceCode
	*/
	virtual void			writeSourceCode	(std::ostream& os, const String& targetLanguage) const;

	/** Get a list of parameters this function invocation will use in the function call as arguments. */
	OperandVector&			getOperandList	() { return mOperands; }
	
	/** Push a new operand (on the end) to the function. 
	@param parameter A function parameter.
	@param opSemantic The in/out semantic of the parameter.
	@param opMask The field mask of the parameter.
	*/
	void					pushOperand(ParameterPtr parameter, Operand::OpSemantic opSemantic, int opMask = Operand::OPM_ALL);

	/** Return the function name */
	const String&			getFunctionName	() const {return mFunctionName; }

	/** Return the return type */
	const String&			getReturnType	() const {return mReturnType; }

	// Attributes.
protected:	
	String				mFunctionName;
	String				mReturnType;
	OperandVector		mOperands;	
};

typedef std::vector<FunctionAtom*> 					FunctionAtomInstanceList;
typedef FunctionAtomInstanceList::iterator 			FunctionAtomInstanceIterator;
typedef FunctionAtomInstanceList::const_iterator	FunctionAtomInstanceConstIterator;

/** @} */
/** @} */

}
}

#endif

