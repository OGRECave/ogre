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
#ifndef _ShaderProgram_
#define _ShaderProgram_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"
#include "OgreShaderFunctionAtom.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that represents a shader based program.
*/
class _OgreRTSSExport Program : public RTShaderSystemAlloc
{

// Interface.
public:	
	/** Get the type of this program. */
	GpuProgramType				getType						() const;

	/** Resolve uniform auto constant parameter with associated real data of this program.
	@param autoType The auto type of the desired parameter.	
	@param data The data to associate with the auto parameter.
	Return parameter instance in case of that resolve operation succeeded.	
	*/
	ParameterPtr		resolveAutoParameterReal	(GpuProgramParameters::AutoConstantType autoType, Real data);

	/** Resolve uniform auto constant parameter with associated int data of this program.
	@param autoType The auto type of the desired parameter.	
	@param data The data to associate with the auto parameter.
	Return parameter instance in case of that resolve operation succeeded.	
	*/
	ParameterPtr		resolveAutoParameterInt		(GpuProgramParameters::AutoConstantType autoType, size_t data);

	/** Resolve uniform parameter of this program.
	@param type The type of the desired parameter.
	@param index The index of the desired parameter.
	@param suggestedName The suggested name for the parameter in case new one should be create.	
	@param variability How this parameter varies (bitwise combination of GpuProgramVariability).
	Return parameter instance in case of that resolve operation succeeded.
	@remarks Pass -1 as index parameter to create a new parameter with the desired type and index.
	*/
	ParameterPtr		resolveParameter			(GpuConstantType type, int index, uint16 variability, const String& suggestedName);
	
	/** Get parameter by a given name.	
	@param name The name of the parameter to search for.
	@remarks Return NULL if no matching parameter found.
	*/
	ParameterPtr		getParameterByName			(const String& name);

	/** Get parameter by a given auto constant type.	
	@param autoType The auto type of the parameter to search for.
	@remarks Return NULL if no matching parameter found.
	*/
	ParameterPtr		getParameterByAutoType		(GpuProgramParameters::AutoConstantType autoType);

	/** Get parameter by a given type and index.	
	@param type The type of the parameter to search for.
	@param index The index of the parameter to search for.
	@remarks Return NULL if no matching parameter found.
	*/
	ParameterPtr		getParameterByType			(GpuConstantType type, int index);

	/** Get the list of uniform parameters of this program.
	*/
	const ShaderParameterList&	getParameters		() const { return mParameters; };

	/** Create new function in this program. Return the newly created function instance.
	@param name The name of the function to create.
	@param desc The description of the function.
	*/
	Function*					createFunction				(const String& name, const String& desc, const Function::FunctionType functionType);

	/** Get a function by a given name. Return NULL if no matching function found.
	@param name The name of the function to search for.
	*/
	Function*					getFunctionByName			(const String& name);

	/** Get the function list of this program.
	*/
	const ShaderFunctionList&	getFunctions				() const { return mFunctions; };

	/** Set the entry point function.
	@param function The function that will use as entry point of this program.
	*/
	void						setEntryPointFunction		(Function* function) { mEntryPointFunction = function; }

	/** Get the entry point function of this program.*/
	Function*					getEntryPointFunction		()					 { return mEntryPointFunction; }

	/** Add dependency for this program. Basically a filename that will be included in this
	program and provide predefined shader functions code.
	One should verify that the given library file he provides can be reached by the resource manager.
	This step can be achieved using the ResourceGroupManager::addResourceLocation method.
	*/
	void						addDependency				(const String& libFileName);

	/** Get the number of external libs this program depends on */
	size_t						getDependencyCount			() const;

	/** Get the library name of the given index dependency.
	@param index The index of the dependecy.
	*/
	const String&				getDependency				(unsigned int index) const;
	

// Protected methods.
protected:

	/** Class constructor.
	@param type The type of this program.
	*/
	Program			(GpuProgramType type);

	/** Class destructor */
	~Program		();

	/** Destroy all parameters of this program. */
	void						destroyParameters	();

	/** Destroy all functions of this program. */
	void						destroyFunctions	();

	/** Add parameter to this program. */
	void						addParameter				(ParameterPtr parameter);
		
	/** Remove parameter from this program. */
	void						removeParameter				(ParameterPtr parameter);


// Attributes.
protected:
	GpuProgramType					mType;						// Program type. (Vertex, Fragment, Geometry).
	ShaderParameterList				mParameters;				// Program global parameters.	
	ShaderFunctionList				mFunctions;					// Function list.
	Function*						mEntryPointFunction;		// Entry point function for this program.	
	StringVector					mDependencies;				// Program dependencies.

private:
	friend class ProgramManager;
};

/** @} */
/** @} */

}
}

#endif

