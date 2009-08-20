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
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTransform::Type = "FFP_Transform";

//-----------------------------------------------------------------------
FFPTransform::FFPTransform()
{

}

//-----------------------------------------------------------------------
FFPTransform::~FFPTransform()
{

}

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

