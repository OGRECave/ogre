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
#ifndef _ShaderProgram_
#define _ShaderProgram_

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"
#include "OgreShaderFunctionAtom.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class Program
{

// Interface.
public:	
	// Program information.
	const String&				getName						() const;
	const String&				getDescription				() const;
	GpuProgramType				getType						() const;


	// Parameters handling.
	Parameter*			resolveAutoParameterReal	(GpuProgramParameters::AutoConstantType autoType, Real data);
	Parameter*			resolveAutoParameterInt		(GpuProgramParameters::AutoConstantType autoType, size_t data);
	Parameter*			resolveParameter			(GpuConstantType type, int index, const String& suggestedName);
	
	Parameter*			getParameterByName			(const String& name);
	Parameter*			getParameterByAutoType		(GpuProgramParameters::AutoConstantType autoType);
	Parameter*			getParameterByType			(GpuConstantType type, int index);
	const ShaderParameterList&	getParameters				() const { return mParameters; };

	// Functions handling.
	Function*					createFunction				(const String& name, const String& desc);
	Function*					getFunctionByName			(const String& name);
	const ShaderFunctionList&	getFunctions				() const { return mFunctions; };

	
	void						setEntryPointFunction		(Function* function) { mEntryPointFunction = function; }
	Function*					getEntryPointFunction		()					 { return mEntryPointFunction; }


	void						addDependency				(const String& libFileName);
	size_t						getDependencyCount			() const;
	const String&				getDependency				(unsigned int index) const;
	

// Protected methods.
protected:
	Program			(const String& name, const String& desc, GpuProgramType type);
	~Program		();

	void						destroyParameters	();
	void						destroyFunctions	();

	void						addParameter				(Parameter* parameter);
	void						removeParameter				(Parameter* parameter);


// Attributes.
protected:
	String							mName;						// Program name.
	String							mDescription;				// Program description.
	GpuProgramType					mType;						// Program type. (Vertex, Fragment, Geometry).
	ShaderParameterList				mParameters;				// Program global parameters.	
	ShaderFunctionList				mFunctions;					// Function list.
	Function*						mEntryPointFunction;		// Entry point function for this program.	
	StringVector					mDependencies;				// Program dependencies.

private:
	friend class ProgramManager;
};

}
}

#endif

