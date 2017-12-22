/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreShaderFFPAlphaTest.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderFunctionAtom.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramSet.h"
#include "OgrePass.h"
#include "OgreMaterialSerializer.h"
#include "OgreShaderFFPRenderState.h"


namespace Ogre {
	namespace RTShader {

		String FFPAlphaTest::Type = "FFP_Alpha_Test";
		

		//-----------------------------------------------------------------------
		const Ogre::String& FFPAlphaTest::getType() const
		{
			return Type;
		}


		//-----------------------------------------------------------------------
		bool FFPAlphaTest::resolveParameters(ProgramSet* programSet)
		{
			Program* psProgram  = programSet->getCpuFragmentProgram();
			Function* psMain = psProgram->getEntryPointFunction();
			  
			mPSAlphaRef = psProgram->resolveAutoParameterReal(GpuProgramParameters::ACT_SURFACE_ALPHA_REJECTION_VALUE, 0);
			mPSAlphaFunc = psProgram->resolveParameter(GCT_FLOAT1,-1, (uint16)GPV_GLOBAL, "gAlphaFunc");
			
			mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
			
			return true;
		}



		//-----------------------------------------------------------------------
		bool FFPAlphaTest::resolveDependencies(ProgramSet* programSet)
		{
			Program* psProgram = programSet->getCpuFragmentProgram();
			psProgram->addDependency(FFP_LIB_ALPHA_TEST);
			return true;
		}
	
		//-----------------------------------------------------------------------
	
		void FFPAlphaTest::copyFrom( const SubRenderState& rhs )
		{

		}

		bool FFPAlphaTest::addFunctionInvocations( ProgramSet* programSet )
		{
			Program* psProgram = programSet->getCpuFragmentProgram();
			Function* psMain = psProgram->getEntryPointFunction();

			FunctionInvocation *curFuncInvocation;

			//Fragment shader invocations
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ALPHA_TEST, FFP_PS_ALPHA_TEST);
			curFuncInvocation->pushOperand(mPSAlphaFunc, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSAlphaRef, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN);

			psMain->addAtomInstance(curFuncInvocation);	

			return true;
		}

		int FFPAlphaTest::getExecutionOrder() const
		{
			return FFP_ALPHA_TEST;
		}

		bool FFPAlphaTest::preAddToRenderState( const RenderState* renderState, Pass* srcPass, Pass* dstPass )
		{
			return srcPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS;
		}

		void FFPAlphaTest::updateGpuProgramsParams( Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList )
		{
			mPSAlphaFunc->setGpuParameter((float)pass->getAlphaRejectFunction());
		}

		//----------------------Factory Implementation---------------------------
		//-----------------------------------------------------------------------
		const String& FFPAlphaTestFactory ::getType() const
		{
			return FFPAlphaTest::Type;
		}

		//-----------------------------------------------------------------------
		SubRenderState*	FFPAlphaTestFactory::createInstanceImpl()
		{
			return OGRE_NEW FFPAlphaTest;
		}
	}
}
#endif


