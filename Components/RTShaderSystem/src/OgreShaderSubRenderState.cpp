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

#include "OgreShaderSubRenderState.h"
#include "OgreException.h"
#include "OgreShaderGenerator.h"

namespace Ogre {
namespace RTShader {



//-----------------------------------------------------------------------
SubRenderState::SubRenderState()
{
	
}

//-----------------------------------------------------------------------
SubRenderState::~SubRenderState()
{
	if (mOtherAccessor.isNull() == false)
	{
		mOtherAccessor->removeSubRenderStateInstance(this);
	}
}

//-----------------------------------------------------------------------
SubRenderStateFactory::~SubRenderStateFactory()
{
	assert(mSubRenderStateList.size() == 0 &&
		"SubRenderStateFactory::~SubRenderStateFactory -> Sub render states still exists !!!");
}

//-----------------------------------------------------------------------
SubRenderState*	SubRenderStateFactory::createInstance()
{
	SubRenderState*	subRenderState = createInstanceImpl();

	mSubRenderStateList.insert(subRenderState);

	return subRenderState;
}

//-----------------------------------------------------------------------
void SubRenderStateFactory::destroyInstance(SubRenderState* subRenderState)
{
	SubRenderStateSetIterator it = mSubRenderStateList.find(subRenderState);

	if (it != mSubRenderStateList.end())
	{
		OGRE_DELETE *it;
		mSubRenderStateList.erase(it);
	}	
}

//-----------------------------------------------------------------------
void SubRenderStateFactory::destroyAllInstances()
{
	SubRenderStateSetIterator it;

	for (it = mSubRenderStateList.begin(); it != mSubRenderStateList.end(); ++it)
	{		
		OGRE_DELETE *it;			
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

	SubRenderStateAccessorPtr rhsAccessor = rhs.getAccessor();

	rhsAccessor->addSubRenderStateInstance(this);
	mOtherAccessor = rhsAccessor;

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

//-----------------------------------------------------------------------
SubRenderStateAccessorPtr SubRenderState::getAccessor()
{
	if (mThisAccessor.isNull())
	{
		SubRenderStateAccessor* accessor = OGRE_NEW_T(SubRenderStateAccessor, MEMCATEGORY_GENERAL)(this);
		
		mThisAccessor.bind(accessor, SPFM_DELETE_T);
	}

	return mThisAccessor;
}

//-----------------------------------------------------------------------
SubRenderStateAccessorPtr SubRenderState::getAccessor() const
{
	if (mThisAccessor.isNull())
	{
		SubRenderStateAccessor* accessor = OGRE_NEW_T(SubRenderStateAccessor, MEMCATEGORY_GENERAL)(this);

		mThisAccessor.bind(accessor, SPFM_DELETE_T);
	}

	return mThisAccessor;
}



}
}

