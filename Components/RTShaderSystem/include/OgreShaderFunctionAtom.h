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
class OGRE_RTSHADERSYSTEM_API FunctionAtom
{
// Interface.
public:

	/** Get the group execution order of this function atom. */
	int						getGroupExecutionOrder		() const;
	
	/** Get an internal execution order whithin a group of this function atom. */
	int						getInternalExecutionOrder	() const;
	
	/** Abstract method that writes a source code to the given output stream in the target shader language. */
	virtual void			writeSourceCode			(std::ostream& os, const String& targetLanguage) = 0;
	

protected:
	/** Class default constructor */
	FunctionAtom		();

// Attributes.
protected:
	int			mGroupExecutionOrder;		// The owner group execution order.	
	int			mInteralExecutionOrder;		// The execution order within the group.		
};


/** A class that represents function invocation code from shader based program function.
*/
class OGRE_RTSHADERSYSTEM_API FunctionInvocation : public FunctionAtom
{
	// Interface.
public:	
	/** Class constructor 
	@param functionName The name of the function to invoke.
	@param groupOrder The group order of this invocation.
	@param internalOrder The internal order of this invocation.
	*/
	FunctionInvocation(const String& functionName, int groupOrder, int internalOrder);

	/** 
	@see FunctionAtom::writeSourceCode
	*/
	virtual void			writeSourceCode			(std::ostream& os, const String& targetLanguage);

	/** Get a list of parameters this function invocation will use in the function call as arguments. */
	StringVector&			getParameterList	() { return mParameters; }
	
	/** Return the function name */
	const String&			getFunctionName		() const {return mFunctionName; }
	// Attributes.
protected:	
	String		 mFunctionName;
	StringVector mParameters;	
};


typedef std::vector<FunctionAtom*> 					FunctionAtomInstanceList;
typedef FunctionAtomInstanceList::iterator 			FunctionAtomInstanceIterator;
typedef FunctionAtomInstanceList::const_iterator	FunctionAtomInstanceConstIterator;

/** @} */
/** @} */

}
}

#endif

