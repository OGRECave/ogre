/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreCompositionPass.h"
#include "OgreMaterialManager.h"

namespace Ogre {

CompositionPass::CompositionPass(CompositionTargetPass *parent):
    mParent(parent),
    mType(PT_RENDERQUAD),
	mIdentifier(0),
	mFirstRenderQueue(RENDER_QUEUE_BACKGROUND),
	mLastRenderQueue(RENDER_QUEUE_SKIES_LATE),
    mClearBuffers(FBT_COLOUR|FBT_DEPTH),
    mClearColour(0.0,0.0,0.0,0.0),
	mClearDepth(1.0f),
	mClearStencil(0),
    mStencilCheck(false),
    mStencilFunc(CMPF_ALWAYS_PASS),
    mStencilRefValue(0),
    mStencilMask(0xFFFFFFFF),
    mStencilFailOp(SOP_KEEP),
    mStencilDepthFailOp(SOP_KEEP),
    mStencilPassOp(SOP_KEEP),
    mStencilTwoSidedOperation(false),
    mQuadCornerModified(false),
    mQuadLeft(-1),
    mQuadTop(1),
    mQuadRight(1),
    mQuadBottom(-1),
	mQuadFarCorners(false),
	mQuadFarCornersViewSpace(false)
{
}
//-----------------------------------------------------------------------
CompositionPass::~CompositionPass()
{
}
//-----------------------------------------------------------------------
void CompositionPass::setType(CompositionPass::PassType type)
{
    mType = type;
}
//-----------------------------------------------------------------------
CompositionPass::PassType CompositionPass::getType() const
{
    return mType;
}
//-----------------------------------------------------------------------
void CompositionPass::setIdentifier(uint32 id)
{
    mIdentifier = id;
}
//-----------------------------------------------------------------------
uint32 CompositionPass::getIdentifier() const
{
    return mIdentifier;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterial(const MaterialPtr& mat)
{
    mMaterial = mat;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterialName(const String &name)
{
    mMaterial = MaterialManager::getSingleton().getByName(name);
}
//-----------------------------------------------------------------------
const MaterialPtr& CompositionPass::getMaterial() const
{
    return mMaterial;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearBuffers(uint32 val)
{
    mClearBuffers = val;
}
//-----------------------------------------------------------------------
uint32 CompositionPass::getClearBuffers()
{
    return mClearBuffers;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearColour(ColourValue val)
{
    mClearColour = val;
}
//-----------------------------------------------------------------------
const ColourValue &CompositionPass::getClearColour()
{
    return mClearColour;
}
//-----------------------------------------------------------------------
void CompositionPass::setInput(size_t id, const String &input, size_t mrtIndex)
{
    assert(id<OGRE_MAX_TEXTURE_LAYERS);
    mInputs[id] = InputTex(input, mrtIndex);
}
//-----------------------------------------------------------------------
const CompositionPass::InputTex &CompositionPass::getInput(size_t id)
{
    assert(id<OGRE_MAX_TEXTURE_LAYERS);
    return mInputs[id];
}
//-----------------------------------------------------------------------
size_t CompositionPass::getNumInputs()
{
    size_t count = 0;
    for(size_t x=0; x<OGRE_MAX_TEXTURE_LAYERS; ++x)
    {
        if(!mInputs[x].name.empty())
            count = x+1;
    }
    return count;
}
//-----------------------------------------------------------------------
void CompositionPass::clearAllInputs()
{
    for(size_t x=0; x<OGRE_MAX_TEXTURE_LAYERS; ++x)
    {
        mInputs[x].name.clear();
    }
}
//-----------------------------------------------------------------------
CompositionTargetPass *CompositionPass::getParent()
{
    return mParent;
}
//-----------------------------------------------------------------------
void CompositionPass::setFirstRenderQueue(uint8 id)
{
	mFirstRenderQueue = id;
}
//-----------------------------------------------------------------------
uint8 CompositionPass::getFirstRenderQueue()
{
	return mFirstRenderQueue;
}
//-----------------------------------------------------------------------
void CompositionPass::setLastRenderQueue(uint8 id)
{
	mLastRenderQueue = id;
}
//-----------------------------------------------------------------------
uint8 CompositionPass::getLastRenderQueue()
{
	return mLastRenderQueue;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearDepth(Real depth)
{
	mClearDepth = depth;
}
Real CompositionPass::getClearDepth()
{
	return mClearDepth;
}
void CompositionPass::setClearStencil(uint32 value)
{
	mClearStencil = value;
}
uint32 CompositionPass::getClearStencil()
{
	return mClearStencil;
}

void CompositionPass::setStencilCheck(bool value)
{
	mStencilCheck = value;
}
bool CompositionPass::getStencilCheck()
{
	return mStencilCheck;
}
void CompositionPass::setStencilFunc(CompareFunction value)
{
	mStencilFunc = value;
}
CompareFunction CompositionPass::getStencilFunc()
{
	return mStencilFunc;
} 
void CompositionPass::setStencilRefValue(uint32 value)
{
	mStencilRefValue = value;
}
uint32 CompositionPass::getStencilRefValue()
{
	return mStencilRefValue;
}
void CompositionPass::setStencilMask(uint32 value)
{
	mStencilMask = value;
}
uint32 CompositionPass::getStencilMask()
{
	return mStencilMask;
}
void CompositionPass::setStencilFailOp(StencilOperation value)
{
	mStencilFailOp = value;
}
StencilOperation CompositionPass::getStencilFailOp()
{
	return mStencilFailOp;
}
void CompositionPass::setStencilDepthFailOp(StencilOperation value)
{
	mStencilDepthFailOp = value;
}
StencilOperation CompositionPass::getStencilDepthFailOp()
{
	return mStencilDepthFailOp;
}
void CompositionPass::setStencilPassOp(StencilOperation value)
{
	mStencilPassOp = value;
}
StencilOperation CompositionPass::getStencilPassOp()
{
	return mStencilPassOp;
}
void CompositionPass::setStencilTwoSidedOperation(bool value)
{
	mStencilTwoSidedOperation = value;
}
bool CompositionPass::getStencilTwoSidedOperation()
{
	return mStencilTwoSidedOperation;
}
void CompositionPass::setQuadCorners(Real left,Real top,Real right,Real bottom)
{
    mQuadCornerModified=true;
    mQuadLeft = left;
    mQuadTop = top;
    mQuadRight = right;
    mQuadBottom = bottom;
}
bool CompositionPass::getQuadCorners(Real & left,Real & top,Real & right,Real & bottom) const
{
    left = mQuadLeft;
    top = mQuadTop;
    right = mQuadRight;
    bottom = mQuadBottom;
    return mQuadCornerModified;
}
void CompositionPass::setQuadFarCorners(bool farCorners, bool farCornersViewSpace)
{
	mQuadFarCorners = farCorners;
	mQuadFarCornersViewSpace = farCornersViewSpace;
}
bool CompositionPass::getQuadFarCorners() const
{
	return mQuadFarCorners;
}
bool CompositionPass::getQuadFarCornersViewSpace() const
{
	return mQuadFarCornersViewSpace;
}

//-----------------------------------------------------------------------
bool CompositionPass::_isSupported(void)
{
    // A pass is supported if material referenced have a supported technique

    if (mType == PT_RENDERQUAD)
    {
        if (mMaterial.isNull())
        {
            return false;
        }

        mMaterial->compile();
        if (mMaterial->getNumSupportedTechniques() == 0)
        {
            return false;
        }
    }

    return true;
}

}
