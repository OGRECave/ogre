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
#ifndef _ShaderProgramFunction_
#define _ShaderProgramFunction_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunctionAtom.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that represents a shader based program function.
*/
class _OgreRTSSExport Function : public RTShaderSystemAlloc
{
// Interface.
public:
	enum FunctionType
	{
		// internal function (default)
		FFT_INTERNAL,
		// Vertex program main
		FFT_VS_MAIN,
		// Pixel shader main
		FFT_PS_MAIN,
	};

	/** Get the name of this function */
	const String&				getName					() const { return m_name; }

	/** Get the description of this function */
	const String&				getDescription			() const { return m_description; }

	/** Resolve input parameter of this function
	@param semantic The desired parameter semantic.
	@param index The index of the desired parameter.
	@param content The content of the parameter.
	@param type The type of the desired parameter.
	Return parameter instance in case of that resolve operation succeeded.
	@remarks Pass -1 as index parameter to create a new parameter with the desired semantic and type.
	*/
	ParameterPtr				resolveInputParameter	(Parameter::Semantic semantic, int index,  const Parameter::Content content, GpuConstantType type);


	/** Resolve output parameter of this function
	@param semantic The desired parameter semantic.	
	@param index The index of the desired parameter.
	@param content The content of the parameter.
	@param type The type of the desired parameter.
	Return parameter instance in case of that resolve operation succeeded.
	@remarks Pass -1 as index parameter to create a new parameter with the desired semantic and type.
	*/
	ParameterPtr				resolveOutputParameter	(Parameter::Semantic semantic, int index,  const Parameter::Content content, GpuConstantType type);

	/** Resolve local parameter of this function	
	@param semantic The desired parameter semantic.	
	@param index The index of the desired parameter.
	@param name The name of the parameter.
	@param type The type of the desired parameter.	
	Return parameter instance in case of that resolve operation succeeded.
	*/
	ParameterPtr				resolveLocalParameter	(Parameter::Semantic semantic, int index, const String& name, GpuConstantType type);

	/** Resolve local parameter of this function	
	@param semantic The desired parameter semantic.	
	@param index The index of the desired parameter.
	@param content The content of the parameter.
	@param type The type of the desired parameter.	
	Return parameter instance in case of that resolve operation succeeded.
	*/
	ParameterPtr				resolveLocalParameter	(Parameter::Semantic semantic, int index, const Parameter::Content content, GpuConstantType type);
	

	/** 
	Get parameter by a given name from the given parameter list.
	@param parameterList The parameters list to look in.
	@param name The name of the parameter to search in the list.
	@remarks Return NULL if no matching parameter found.
	*/
	static ParameterPtr				getParameterByName		(const ShaderParameterList& parameterList, const String& name);

	/** 
	Get parameter by a given semantic and index from the given parameter list.
	@param parameterList The parameters list to look in.
	@param semantic The semantic of the parameter to search in the list.
	@param index The index of the parameter to search in the list.
	@remarks Return NULL if no matching parameter found.
	*/
	static ParameterPtr				getParameterBySemantic	(const ShaderParameterList& parameterList, const Parameter::Semantic semantic, int index);


	/** 
	Get parameter by a given content and type from the given parameter list.
	@param parameterList The parameters list to look in.
	@param content The content of the parameter to search in the list.
	@param type The type of the parameter to search in the list.
	@remarks Return NULL if no matching parameter found.
	*/
	ParameterPtr					getParameterByContent	(const ShaderParameterList& parameterList, const Parameter::Content content, GpuConstantType type);

	/** Return a list of input parameters. */
	const ShaderParameterList&		getInputParameters		() const { return mInputParameters; }	

	/** Return a list of output parameters. */
	const ShaderParameterList&		getOutputParameters		() const { return mOutputParameters; }

	/** Return a list of local parameters. */
	const ShaderParameterList&		getLocalParameters		() const { return mLocalParameters; }	
	
	/** Add a function atom instance to this function. 
	@param atomInstance The atom instance to add.
	*/
	void							addAtomInstace			(FunctionAtom* atomInstance);

	/** Delete a function atom instance from this function. 
	@param atomInstance The atom instance to OGRE_DELETE.
	*/
	bool							deleteAtomInstance		(FunctionAtom* atomInstance);

	/** Sort all atom instances of this function. */
	void							sortAtomInstances		();

	/** Return list of atom instances composing this function. */
	FunctionAtomInstanceList&		getAtomInstances		() { return mAtomInstances; }

	/** Return list of atom instances composing this function. (Const version) */
	const FunctionAtomInstanceList&	getAtomInstances		() const { return mAtomInstances; }

	/** Add input parameter to this function. */
	void						addInputParameter			(ParameterPtr parameter);

	/** Add output parameter to this function. */
	void						addOutputParameter			(ParameterPtr parameter);

	/** Delete input parameter from this function. */
	void						deleteInputParameter		(ParameterPtr parameter);

	/** Delete output parameter from this function. */
	void						deleteOutputParameter		(ParameterPtr parameter);

	/** Delete all input parameters from this function. */
	void						deleteAllInputParameters	();

	/** Delete all output parameters from this function. */
	void						deleteAllOutputParameters	();

	/** get function type. */
	FunctionType getFunctionType() const;


protected:

	/** Class constructor.
	@param name The name of this function.
	@param desc The description of this function.
	@remarks This class is allocated via an instance of Program class. 
	*/
	Function			(const String& name, const String& desc, const FunctionType functionType);

	/** Class destructor */
	~Function			();

	/** Add parameter to given list */
	void						addParameter				(ShaderParameterList& parameterList, ParameterPtr parameter);

	/** Delete parameter from a given list */
	void						deleteParameter				(ShaderParameterList& parameterList, ParameterPtr parameter);

	/** Function atom comparison function used to sort atoms. */
	static int					sAtomInstanceCompare		(const void * p0, const void *p1);

protected:
	String						m_name;						// Function name.
	String						m_description;				// Function description.
	ShaderParameterList			mInputParameters;			// Input parameters.
	ShaderParameterList			mOutputParameters;			// Output parameters.
	ShaderParameterList			mLocalParameters;			// Local parameters.
	FunctionAtomInstanceList	mAtomInstances;				// Atom instances composing this function.
	FunctionType				m_functionType;				// Function type
	
private:
	friend class Program;
};

typedef vector<Function*>::type						ShaderFunctionList;
typedef ShaderFunctionList::iterator 				ShaderFunctionIterator;
typedef ShaderFunctionList::const_iterator			ShaderFunctionConstIterator;

/** @} */
/** @} */

}
}

#endif