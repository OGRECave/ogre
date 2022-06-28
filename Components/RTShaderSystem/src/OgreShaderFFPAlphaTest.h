/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rightsA
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
#ifndef _OgreShaderExAlphaTest_
#define _OgreShaderExAlphaTest_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderSubRenderState.h"

namespace Ogre {
namespace RTShader {

/**
A factory that enables creation of LayeredBlending instances.
@remarks Sub class of SubRenderStateFactory
*/
	class FFPAlphaTest : public SubRenderState
	{

	private:
	UniformParameterPtr mPSAlphaRef;
	UniformParameterPtr mPSAlphaFunc;
	ParameterPtr mPSOutDiffuse;



	protected:

    /**
    @see SubRenderState::resolveParameters.
    */
    virtual bool resolveParameters(ProgramSet* programSet);

    /**
    @see SubRenderState::resolveDependencies.
    */
    virtual bool resolveDependencies(ProgramSet* programSet);

    /**
    @see SubRenderState::addFunctionInvocations.
    */
    virtual bool addFunctionInvocations(ProgramSet* programSet);

public:

	/// The type.
	static String Type;
    /**
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

    /**
    @see SubRenderState::getExecutionOrder.
    */
    virtual int getExecutionOrder() const;

    /**
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState (const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    /**
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);

    /**
    @see SubRenderState::updateGpuProgramsParams.
    */
    virtual void updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

	};

class FFPAlphaTestFactory : public SubRenderStateFactory
{

public:
static String Type;

	/**
	@see SubRenderStateFactory::getType.
	*/
	virtual const String& getType() const;

protected:

	/**
	@see SubRenderStateFactory::createInstanceImpl.
	*/
	virtual SubRenderState* createInstanceImpl();
};

} // namespace RTShader
} // namespace Ogre

#endif
#endif
