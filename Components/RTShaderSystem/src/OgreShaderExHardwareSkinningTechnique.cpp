/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreShaderExHardwareSkinningTechnique.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExDualQuaternionSkinning.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMaterial.h"
#include "OgreSubMesh.h"
#include "OgreShaderGenerator.h"

#define HS_DATA_BIND_NAME "HS_SRS_DATA"


namespace Ogre {

namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
HardwareSkinningTechnique::HardwareSkinningTechnique() :
	mBoneCount(0),
	mWeightCount(0),
	mCorrectAntipodalityHandling(false),
	mScalingShearingSupport(false),
	mDoBoneCalculations(false)
{
}

//-----------------------------------------------------------------------
HardwareSkinningTechnique::~HardwareSkinningTechnique()
{
}

//-----------------------------------------------------------------------
void HardwareSkinningTechnique::setHardwareSkinningParam(ushort boneCount, ushort weightCount, bool correctAntipodalityHandling, bool scalingShearingSupport)
{
	mBoneCount = std::min<ushort>(boneCount, 256);
	mWeightCount = std::min<ushort>(weightCount, 4);
	mCorrectAntipodalityHandling = correctAntipodalityHandling;
	mScalingShearingSupport = scalingShearingSupport;
}

//-----------------------------------------------------------------------
void HardwareSkinningTechnique::setDoBoneCalculations(bool doBoneCalculations)
{
	mDoBoneCalculations = doBoneCalculations;
}

//-----------------------------------------------------------------------
ushort HardwareSkinningTechnique::getBoneCount()
{
	return mBoneCount;
}

//-----------------------------------------------------------------------
ushort HardwareSkinningTechnique::getWeightCount()
{
	return mWeightCount;
}

//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::hasCorrectAntipodalityHandling()
{
	return mCorrectAntipodalityHandling;
}

//-----------------------------------------------------------------------
bool HardwareSkinningTechnique::hasScalingShearingSupport()
{
	return mScalingShearingSupport;
}

//-----------------------------------------------------------------------
Operand::OpMask HardwareSkinningTechnique::indexToMask(int index)
{
	switch(index)
	{
	case 0: return Operand::OPM_X;
	case 1: return Operand::OPM_Y;
	case 2: return Operand::OPM_Z;
	case 3: return Operand::OPM_W;
	default: OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Illegal value", "HardwareSkinningTechnique::indexToMask");
	}
}

//-----------------------------------------------------------------------
void HardwareSkinningTechnique::copyFrom(const HardwareSkinningTechnique* hardSkin)
{
	mWeightCount = hardSkin->mWeightCount;
	mBoneCount = hardSkin->mBoneCount;
	mDoBoneCalculations = hardSkin->mDoBoneCalculations;
	mCorrectAntipodalityHandling = hardSkin->mCorrectAntipodalityHandling;
	mScalingShearingSupport = hardSkin->mScalingShearingSupport;
}

}
}

#endif


