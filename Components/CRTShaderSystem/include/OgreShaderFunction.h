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
#ifndef _ShaderProgramFunction_
#define _ShaderProgramFunction_

#include "OgrePrerequisites.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunctionAtom.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class Function
{
// Interface.
public:
	const String&					getName					() const { return m_name; }
	const String&					getDescription			() const { return m_description; }

	Parameter*						resolveInputParameter	(Parameter::Semantic semantic, int index, GpuConstantType type);
	Parameter*						resolveOutputParameter	(Parameter::Semantic semantic, int index, GpuConstantType type);
	Parameter*						resolveLocalParameter	(Parameter::Semantic semantic, int index, GpuConstantType type, const String& name);
	
	
	const ShaderParameterList&		getInputParameters		() const { return mInputParameters; }	
	const ShaderParameterList&		getOutputParameters		() const { return mOutputParameters; }
	const ShaderParameterList&		getLocalParameters		() const { return mLocalParameters; }	

	Parameter*						getParameterByName		(const ShaderParameterList& parameterList, const String& name);
	Parameter*						getParameterBySemantic	(const ShaderParameterList& parameterList, const Parameter::Semantic semantic, int index);
	Parameter*						getParameterByType		(const ShaderParameterList& parameterList, GpuConstantType type);

	
	void							addAtomInstace			(FunctionAtom* atomInstance);
	bool							deleteAtomInstance		(FunctionAtom* atomInstance);

	void							sortAtomInstances		();
	const FunctionAtomInstanceList&	getAtomInstances		() const { return mAtomInstances; }

protected:
	Function			(const String& name, const String& desc);
	~Function			();


	void						addInputParameter			(Parameter* parameter);
	void						addOutputParameter			(Parameter* parameter);
	void						deleteInputParameter		(Parameter* parameter);
	void						deleteOutputParameter		(Parameter* parameter);

	void						addParameter				(ShaderParameterList& parameterList, Parameter* parameter);
	void						deleteParameter				(ShaderParameterList& parameterList, Parameter* parameter);


	static int					sAtomInstanceCompare		(const void * p0, const void *p1);

protected:
	String						m_name;						// Function name.
	String						m_description;				// Function description.
	ShaderParameterList			mInputParameters;			// Input parameters.
	ShaderParameterList			mOutputParameters;			// Output parameters.
	ShaderParameterList			mLocalParameters;			// Local parameters.
	FunctionAtomInstanceList	mAtomInstances;				// Atom instances composing this function.
	
private:
	friend class Program;
};

typedef std::vector<Function*> 						ShaderFunctionList;
typedef ShaderFunctionList::iterator 				ShaderFunctionIterator;
typedef ShaderFunctionList::const_iterator			ShaderFunctionConstIterator;


}
}

#endif
