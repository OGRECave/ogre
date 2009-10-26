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
#ifndef _ShaderProgramWriter_
#define _ShaderProgramWriter_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreString.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that perform the actual writing operation of a given CPU program to stream.
The main usage of this class is to generate the actual shader source code from a CPU program representation.
*/
class OGRE_RTSHADERSYSTEM_INTERNAL ProgramWriter
{
// Interface.
public:

	/** Class constructor. 
	@param language The target shader language.
	*/
	ProgramWriter	(const String& language);

	/** Class destructor */
	~ProgramWriter	();
	

	/** Write the program shader source code.
	@param os The output stream to write to code into.
	@param program The source CPU program for the GPU program code.
	*/
	void			writeSourceCode				(std::ostream& os, Program* program);

	
// Protected methods.
protected:
	/** Get the target language of this writer. */
	const String&	getTargetLanguage		() { return mTargetLanguage; }

	/** Initialize string maps. */
	void		initializeStringMaps		();

	/** Write the program title. */
	void		writeProgramTitle			(std::ostream& os, Program* program);

	/** Write the program dependencies. */
	void		writeProgramDependencies	(std::ostream& os, Program* program);

	/** Write the uniform parameters title. */
	void		writeUniformParametersTitle	(std::ostream& os, Program* program);

	/** Write a function title. */
	void		writeFunctionTitle			(std::ostream& os, Function* function);

	/** Write a uniform parameter. */
	void		writeUniformParameter		(std::ostream& os, ParameterPtr parameter);

	/** Write a function parameter. */
	void		writeFunctionParameter		(std::ostream& os, ParameterPtr parameter);

	/** Write a local parameter. */
	void		writeLocalParameter			(std::ostream& os, ParameterPtr parameter);

	/** Write a function declaration. */
	void		writeFunctionDeclaration	(std::ostream& os, Function* function);

	/** Write function atom instance. */
	void		writeAtomInstance			(std::ostream& os, FunctionAtom* atom);

	
protected:
	typedef		std::map<GpuConstantType, const char*>		GpuConstTypeToStringMap;
	typedef		std::map<Parameter::Semantic, const char*>	ParamSemanticToStringMap;

// Attributes.
protected:
	String						mTargetLanguage;		// The target language.
	GpuConstTypeToStringMap		mGpuConstTypeMap;		// Map between GPU constant type to string value.
	ParamSemanticToStringMap	mParamSemanticMap;		// Map between parameter semantic to string value.
};

/** @} */
/** @} */

}
}

#endif
