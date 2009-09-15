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
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTransform::Type = "FFP_Transform";

//-----------------------------------------------------------------------
const String& FFPTransform::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	FFPTransform::getExecutionOrder() const
{
	return FFP_TRANSFORM;
}

//-----------------------------------------------------------------------
bool FFPTransform::createCpuSubPrograms(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	
	// Resolve World View Projection Matrix.
	Parameter* wvpMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX, 0);
	if (wvpMatrix == NULL)	
		return false;
		
	Function* vsEntry = vsProgram->getEntryPointFunction();
	assert(vsEntry != NULL);

	// Resolve input position parameter.
	Parameter* positionIn = vsEntry->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);	
	if (positionIn == NULL)	
		return false;
	

	// Resolve output position parameter.
	Parameter* positionOut = vsEntry->resolveOutputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
	if (positionOut == NULL)	
		return false;
	

	// Add dependency.
	vsProgram->addDependency(FFP_LIB_TRANSFORM);

	FunctionInvocation* transformFunc = new FunctionInvocation(FFP_FUNC_TRANSFORM,  FFP_VS_TRANSFORM, 0); 

	transformFunc->getParameterList().push_back(wvpMatrix->getName());
	transformFunc->getParameterList().push_back(positionIn->getName());
	transformFunc->getParameterList().push_back(positionOut->getName());

	vsEntry->addAtomInstace(transformFunc);

	return true;
}


//-----------------------------------------------------------------------
void FFPTransform::copyFrom(const SubRenderState& rhs)
{

}

//-----------------------------------------------------------------------
const String& FFPTransformFactory::getType() const
{
	return FFPTransform::Type;
}


//-----------------------------------------------------------------------
SubRenderState*	FFPTransformFactory::createInstanceImpl()
{
	return new FFPTransform;
}


}
}

