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
    mType(PT_RENDERQUAD)
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
    mMaterial.identifier = id;
}
//-----------------------------------------------------------------------
uint32 CompositionPass::getIdentifier() const
{
    return mMaterial.identifier;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterial(const MaterialPtr& mat)
{
    mMaterial.material = mat;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterialName(const String &name)
{
    mMaterial.material = MaterialManager::getSingleton().getByName(name);
}
//-----------------------------------------------------------------------
const MaterialPtr& CompositionPass::getMaterial() const
{
    return mMaterial.material;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearBuffers(uint32 val)
{
    mClear.buffers = val;
}
//-----------------------------------------------------------------------
uint32 CompositionPass::getClearBuffers() const
{
    return mClear.buffers;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearColour(const ColourValue &val)
{
    mClear.colour = val;
}
//-----------------------------------------------------------------------
const ColourValue &CompositionPass::getClearColour() const
{
    return mClear.colour;
}
//-----------------------------------------------------------------------
void CompositionPass::setAutomaticColour(bool val)
{
    mClear.automaticColour = val;
}
//-----------------------------------------------------------------------
bool CompositionPass::getAutomaticColour() const
{
    return mClear.automaticColour;
}
//-----------------------------------------------------------------------
void CompositionPass::setInput(size_t id, const String &input, size_t mrtIndex)
{
    assert(id < OGRE_MAX_TEXTURE_LAYERS);
    mMaterial.inputs[id] = InputTex(input, mrtIndex);
}
//-----------------------------------------------------------------------
const CompositionPass::InputTex &CompositionPass::getInput(size_t id) const
{
    assert(id < OGRE_MAX_TEXTURE_LAYERS);
    return mMaterial.inputs[id];
}
//-----------------------------------------------------------------------
size_t CompositionPass::getNumInputs() const
{
    size_t count = 0;
    for(size_t x = 0; x < OGRE_MAX_TEXTURE_LAYERS; ++x)
    {
        if(!mMaterial.inputs[x].name.empty())
            count = x+1;
    }
    return count;
}
//-----------------------------------------------------------------------
void CompositionPass::clearAllInputs()
{
    for(auto& input : mMaterial.inputs)
    {
        input.name.clear();
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
    mRenderScene.firstRenderQueue = id;
}
//-----------------------------------------------------------------------
uint8 CompositionPass::getFirstRenderQueue() const
{
    return mRenderScene.firstRenderQueue;
}
//-----------------------------------------------------------------------
void CompositionPass::setLastRenderQueue(uint8 id)
{
    mRenderScene.lastRenderQueue = id;
}
//-----------------------------------------------------------------------
uint8 CompositionPass::getLastRenderQueue() const
{
    return mRenderScene.lastRenderQueue ;
}
//-----------------------------------------------------------------------
void CompositionPass::setMaterialScheme(const String& schemeName)
{
    mRenderScene.materialScheme = schemeName;
}
//-----------------------------------------------------------------------
const String& CompositionPass::getMaterialScheme(void) const
{
    return mRenderScene.materialScheme;
}
//-----------------------------------------------------------------------
void CompositionPass::setClearDepth(float depth)
{
    mClear.depth = depth;
}
float CompositionPass::getClearDepth() const
{
    return mClear.depth;
}
void CompositionPass::setClearStencil(uint16 value)
{
    mClear.stencil = value;
}
uint16 CompositionPass::getClearStencil() const
{
    return mClear.stencil;
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
void CompositionPass::setQuadFarCorners(bool farCorners, bool farCornersViewSpace)
{
    mQuad.farCorners = farCorners;
    mQuad.farCornersViewSpace = farCornersViewSpace;
}
bool CompositionPass::getQuadFarCorners() const
{
    return mQuad.farCorners;
}
bool CompositionPass::getQuadFarCornersViewSpace() const
{
    return mQuad.farCornersViewSpace;
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
        if (!mMaterial.material)
        {
            return false;
        }

        mMaterial.material->load();
        if (mMaterial.material->getSupportedTechniques().empty())
        {
            return false;
        }
    }

    return true;
}

}
