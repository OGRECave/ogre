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
#include "OgreRenderQueue.h"

namespace Ogre {

CompositionPass::CompositionPass(CompositionTargetPass *parent):
    mParent(parent),
    mType(PT_RENDERQUAD),
    mIdentifier(0),
    mFirstRenderQueue(RENDER_QUEUE_BACKGROUND),
    mLastRenderQueue(RENDER_QUEUE_SKIES_LATE),
    mClearBuffers(FBT_COLOUR|FBT_DEPTH),
    mClearColour(ColourValue::ZERO),
    mAutomaticColour(false),
    mClearDepth(1.0f),
    mClearStencil(0),
    mStencilReadBackAsTexture(false),
    mQuadCornerModified(false),
    mQuad(-1, 1, 1, -1),
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
void CompositionPass::setClearColour(const ColourValue &val)
{
    mClearColour = val;
}
//-----------------------------------------------------------------------
const ColourValue &CompositionPass::getClearColour() const
{
    return mClearColour;
}
//-----------------------------------------------------------------------
void CompositionPass::setAutomaticColour(bool val)
{
    mAutomaticColour = val;
}
//-----------------------------------------------------------------------
bool CompositionPass::getAutomaticColour() const
{
    return mAutomaticColour;
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
    mStencilState.enabled = value;
}
bool CompositionPass::getStencilCheck() const
{
    return mStencilState.enabled;
}
void CompositionPass::setStencilFunc(CompareFunction value)
{
    mStencilState.compareOp = value;
}
CompareFunction CompositionPass::getStencilFunc() const
{
    return mStencilState.compareOp;
} 
void CompositionPass::setStencilRefValue(uint32 value)
{
    mStencilState.referenceValue = value;
}
uint32 CompositionPass::getStencilRefValue() const
{
    return mStencilState.referenceValue;
}
void CompositionPass::setStencilMask(uint32 value)
{
    mStencilState.compareMask = value;
}
uint32 CompositionPass::getStencilMask() const
{
    return mStencilState.compareMask ;
}
void CompositionPass::setStencilFailOp(StencilOperation value)
{
    mStencilState.stencilFailOp = value;
}
StencilOperation CompositionPass::getStencilFailOp() const
{
    return mStencilState.stencilFailOp;
}
void CompositionPass::setStencilDepthFailOp(StencilOperation value)
{
    mStencilState.depthFailOp = value;
}
StencilOperation CompositionPass::getStencilDepthFailOp() const
{
    return mStencilState.depthFailOp;
}
void CompositionPass::setStencilPassOp(StencilOperation value)
{
    mStencilState.depthStencilPassOp = value;
}
StencilOperation CompositionPass::getStencilPassOp() const
{
    return mStencilState.depthStencilPassOp;
}
void CompositionPass::setStencilTwoSidedOperation(bool value)
{
    mStencilState.twoSidedOperation = value;
}
bool CompositionPass::getStencilTwoSidedOperation() const
{
    return mStencilState.twoSidedOperation;
}
void CompositionPass::setStencilReadBackAsTextureOperation(bool value)
{
    mStencilReadBackAsTexture = value;
}
bool CompositionPass::getStencilReadBackAsTextureOperation() const
{
    return mStencilReadBackAsTexture;
}

void CompositionPass::setQuadCorners(Real left,Real top,Real right,Real bottom)
{
    mQuadCornerModified=true;
    mQuad.left = left;
    mQuad.top = top;
    mQuad.right = right;
    mQuad.bottom = bottom;
}
bool CompositionPass::getQuadCorners(Real & left,Real & top,Real & right,Real & bottom) const
{
    left = mQuad.left;
    top = mQuad.top;
    right = mQuad.right;
    bottom = mQuad.bottom;
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
        if (!mMaterial)
        {
            return false;
        }

        mMaterial->compile();
        if (mMaterial->getSupportedTechniques().empty())
        {
            return false;
        }
    }

    return true;
}

}
