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
	mMaterialScheme(StringUtil::BLANK),
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
uint32 CompositionPass::getClearBuffers() const
{
    return mClearBuffers;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearColour(ColourValue val)
{
    mClearColour = val;
}
//-----------------------------------------------------------------------
const ColourValue &CompositionPass::getClearColour() const
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
const CompositionPass::InputTex &CompositionPass::getInput(size_t id) const
{
    assert(id<OGRE_MAX_TEXTURE_LAYERS);
    return mInputs[id];
}
//-----------------------------------------------------------------------
size_t CompositionPass::getNumInputs() const
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
uint8 CompositionPass::getFirstRenderQueue() const
{
	return mFirstRenderQueue;
}
//-----------------------------------------------------------------------
void CompositionPass::setLastRenderQueue(uint8 id)
{
	mLastRenderQueue = id;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterialScheme(const String& schemeName)
{
	mMaterialScheme = schemeName;
}
//-----------------------------------------------------------------------
const String& CompositionPass::getMaterialScheme(void) const
{
	return mMaterialScheme;
}
//-----------------------------------------------------------------------
uint8 CompositionPass::getLastRenderQueue() const
{
	return mLastRenderQueue;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearDepth(Real depth)
{
	mClearDepth = depth;
}
Real CompositionPass::getClearDepth() const
{
	return mClearDepth;
}
void CompositionPass::setClearStencil(uint32 value)
{
	mClearStencil = value;
}
uint32 CompositionPass::getClearStencil() const
{
	return mClearStencil;
}

void CompositionPass::setStencilCheck(bool value)
{
	mStencilCheck = value;
}
bool CompositionPass::getStencilCheck() const
{
	return mStencilCheck;
}
void CompositionPass::setStencilFunc(CompareFunction value)
{
	mStencilFunc = value;
}
CompareFunction CompositionPass::getStencilFunc() const
{
	return mStencilFunc;
} 
void CompositionPass::setStencilRefValue(uint32 value)
{
	mStencilRefValue = value;
}
uint32 CompositionPass::getStencilRefValue() const
{
	return mStencilRefValue;
}
void CompositionPass::setStencilMask(uint32 value)
{
	mStencilMask = value;
}
uint32 CompositionPass::getStencilMask() const
{
	return mStencilMask;
}
void CompositionPass::setStencilFailOp(StencilOperation value)
{
	mStencilFailOp = value;
}
StencilOperation CompositionPass::getStencilFailOp() const
{
	return mStencilFailOp;
}
void CompositionPass::setStencilDepthFailOp(StencilOperation value)
{
	mStencilDepthFailOp = value;
}
StencilOperation CompositionPass::getStencilDepthFailOp() const
{
	return mStencilDepthFailOp;
}
void CompositionPass::setStencilPassOp(StencilOperation value)
{
	mStencilPassOp = value;
}
StencilOperation CompositionPass::getStencilPassOp() const
{
	return mStencilPassOp;
}
void CompositionPass::setStencilTwoSidedOperation(bool value)
{
	mStencilTwoSidedOperation = value;
}
bool CompositionPass::getStencilTwoSidedOperation() const
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
		
void CompositionPass::setCustomType(const String& customType)
{
	mCustomType = customType;
}

const String& CompositionPass::getCustomType() const
{
	return mCustomType;
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
