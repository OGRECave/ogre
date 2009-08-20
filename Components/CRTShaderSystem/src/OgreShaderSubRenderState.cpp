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

#include "OgreShaderSubRenderState.h"
#include "OgreException.h"

namespace Ogre {
namespace CRTShader {



//-----------------------------------------------------------------------
SubRenderState::SubRenderState()
{
	mParent = NULL;
}

//-----------------------------------------------------------------------
SubRenderState::~SubRenderState()
{

}

uint32 SubRenderState::getHashCode()
{
	_StringHash H;

	return static_cast<uint32>(H(getType()));
}

//-----------------------------------------------------------------------
SubRenderStateFactory::~SubRenderStateFactory()
{
	destroyAllInstances();
}

//-----------------------------------------------------------------------
SubRenderState*	SubRenderStateFactory::createInstance()
{
	SubRenderState*	subRenderState = createInstanceImpl();

	mSubRenderStateList.push_back(subRenderState);

	return subRenderState;
}

//-----------------------------------------------------------------------
void SubRenderStateFactory::destroyInstance(SubRenderState* subRenderState)
{
	SubRenderStateIterator it;

	for (it = mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{
		if (*it == subRenderState)
		{
			delete *it;
			mSubRenderStateList.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------
void SubRenderStateFactory::destroyAllInstances()
{
	SubRenderStateIterator it;

	for (it = mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{		
		delete *it;			
	}
	mSubRenderStateList.clear();

}

//-----------------------------------------------------------------------
SubRenderState& SubRenderState::operator=(const SubRenderState& rhs)
{
	if (getType() != rhs.getType())
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
			"Can not copy sub render states of different types !!",
			"SubRenderState::operator=");
	}

	copyFrom(rhs);

	return *this;
}

//-----------------------------------------------------------------------
bool SubRenderState::createCpuSubPrograms(ProgramSet* programSet)
{
	bool result;

	// Resolve parameters.
	result = resolveParameters(programSet);
	if (false == result)
		return false;

	// Resolve dependencies.
	result = resolveDependencies(programSet);
	if (false == result)
		return false;

	// Add function invocations.
	result = addFunctionInvocations(programSet);
	if (false == result)
		return false;

	return true;
}

//-----------------------------------------------------------------------
bool SubRenderState::resolveParameters(ProgramSet* programSet)
{
	return true;
}

//-----------------------------------------------------------------------
bool SubRenderState::resolveDependencies(ProgramSet* programSet)
{
	return true;
}

//-----------------------------------------------------------------------
bool SubRenderState::addFunctionInvocations( ProgramSet* programSet )
{
	return true;
}




}
}

