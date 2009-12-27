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
#ifndef __ShaderProgramWriterCG_H__
#define __ShaderProgramWriterCG_H__

#include "OgreShaderProgramWriterManager.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** CG target language writer implementation.
@see ProgramWriter.
*/
class CGProgramWriter : public ProgramWriter
{

// Interface.
public:

	/** Class constructor. 
	@param language The target shader language.
	*/
	CGProgramWriter	();

	/** Class destructor */
	virtual ~CGProgramWriter ();

	/** 
	@see ProgramWriter::writeSourceCode.
	*/
	virtual void			writeSourceCode			(std::ostream& os, Program* program);

	/** 
	@see ProgramWriter::getTargetLanguage.
	*/
	virtual const String&  getTargetLanguage			() const { return TargetLanguage; }

	static String TargetLanguage;

// Protected methods.
protected:

	/** Initialize string maps. */
	void		initializeStringMaps		();

	/** Write the program dependencies. */
	void		writeProgramDependencies	(std::ostream& os, Program* program);
	
	/** Write a uniform parameter. */
	void		writeUniformParameter		(std::ostream& os, ParameterPtr parameter);

	/** Write a function parameter. */
	void		writeFunctionParameter		(std::ostream& os, ParameterPtr parameter, const String & overrideType = "");

	/** Write a local parameter. */
	void		writeLocalParameter			(std::ostream& os, ParameterPtr parameter);

	/** Write a function declaration. */
	void		writeFunctionDeclaration	(std::ostream& os, Function* function);

	/** Write function atom instance. */
	void		writeAtomInstance			(std::ostream& os, FunctionAtom* atom);


protected:
	typedef		map<GpuConstantType, const char*>::type		GpuConstTypeToStringMap;
	typedef		map<Parameter::Semantic, const char*>::type	ParamSemanticToStringMap;

// Attributes.
protected:
	GpuConstTypeToStringMap		mGpuConstTypeMap;		// Map between GPU constant type to string value.
	ParamSemanticToStringMap	mParamSemanticMap;		// Map between parameter semantic to string value.
};

/** CG program writer factory implementation.
@see ProgramWriterFactory
*/
class ShaderProgramWriterCGFactory : public ProgramWriterFactory
{
public:
	ShaderProgramWriterCGFactory()
	{
		mLanguage = "cg";
	}
	virtual ~ShaderProgramWriterCGFactory() {}

	/** 
	@see ProgramWriterFactory::getTargetLanguage
	*/
	virtual const String& getTargetLanguage(void) const
	{
		return mLanguage;
	}

	/** 
	@see ProgramWriterFactory::create
	*/
	virtual ProgramWriter* create(void)
	{
		return OGRE_NEW CGProgramWriter();
	}

private:
	String mLanguage;

};

/** @} */
/** @} */
}
}

#endif