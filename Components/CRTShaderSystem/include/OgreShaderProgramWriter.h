/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _ShaderProgramWriter_
#define _ShaderProgramWriter_

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreString.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class ProgramWriter
{
// Interface.
public:
	ProgramWriter	(const String& language);
	~ProgramWriter	();
	

	void			writeSourceCode				(std::ostream& os, Program* program);

	
// Protected methods.
protected:
	const String&	getTargetLanguage		() { return mTargetLanguage; }
	void		initializeStringMaps		();

	void		writeProgramTitle			(std::ostream& os, Program* program);
	void		writeProgramDependencies	(std::ostream& os, Program* program);
	void		writeGlobalParametersTitle	(std::ostream& os, Program* program);
	void		writeFunctionTitle			(std::ostream& os, Function* function);
	void		writeUniformParameter		(std::ostream& os, Parameter* parameter);
	void		writeFunctionParameter		(std::ostream& os, Parameter* parameter);
	void		writeLocalParameter			(std::ostream& os, Parameter* parameter);
	void		writeFunctionDeclaration	(std::ostream& os, Function* function);
	void		writeAtomInstance			(std::ostream& os, FunctionAtom* atom);

	

protected:
	typedef		std::map<GpuConstantType, const char*>		GpuConstTypeToStringMap;
	typedef		std::map<Parameter::Semantic, const char*>	ParamSemanticToStringMap;

// Attributes.
protected:
	String						mTargetLanguage;
	GpuConstTypeToStringMap		mGpuConstTypeMap;
	ParamSemanticToStringMap	mParamSemanticMap;
};

}
}

#endif
