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
#ifndef _ShaderFunctionAtom_
#define _ShaderFunctionAtom_

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"
#include "OgreShaderParameter.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class FunctionAtom
{
// Interface.
public:	
	int						getGroupExecutionOrder		() const;
	int						getInternalExecutionOrder	() const;
	
	virtual void			writeSourceCode			(std::ostream& os, const String& targetLanguage) = 0;
	

protected:
	FunctionAtom		();

// Attributes.
protected:
	int			mGroupExecutionOrder;		// The owner group execution order.	
	int			mInteralExecutionOrder;		// The execution order within the group.		
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
class FunctionInvocation : public FunctionAtom
{
	// Interface.
public:	
	FunctionInvocation(const String& functionName, int groupOrder, int internalOrder);

	virtual void			writeSourceCode			(std::ostream& os, const String& targetLanguage);

	StringVector&			getParameterList	() { return mParameters; }
	
	// Attributes.
protected:	
	String		mFunctionName;
	StringVector mParameters;	
};


typedef std::vector<FunctionAtom*> 					FunctionAtomInstanceList;
typedef FunctionAtomInstanceList::iterator 			FunctionAtomInstanceIterator;
typedef FunctionAtomInstanceList::const_iterator	FunctionAtomInstanceConstIterator;

}
}

#endif

