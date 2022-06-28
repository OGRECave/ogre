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
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

namespace Ogre {

CompositionTargetPass::CompositionTargetPass(CompositionTechnique *parent):
    mParent(parent),
    mInputMode(IM_NONE),
    mOnlyInitial(false),
    mVisibilityMask(0xFFFFFFFF),
    mLodBias(1.0f),
    mMaterialScheme(MaterialManager::DEFAULT_SCHEME_NAME), 
    mShadowsEnabled(true),
    mOutputSlice(0)
{
    if (Root::getSingleton().getRenderSystem())
    {
        mMaterialScheme = Root::getSingleton().getRenderSystem()->_getDefaultViewportMaterialScheme();
    }
}
//-----------------------------------------------------------------------
CompositionTargetPass::~CompositionTargetPass()
{
    removeAllPasses();
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setInputMode(InputMode mode)
{
    mInputMode = mode;
}
//-----------------------------------------------------------------------
CompositionTargetPass::InputMode CompositionTargetPass::getInputMode() const
{
    return mInputMode;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setOutputName(const String &out)
{
    mOutputName = out;
}
//-----------------------------------------------------------------------
const String &CompositionTargetPass::getOutputName() const
{
    return mOutputName;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setOnlyInitial(bool value)
{
    mOnlyInitial = value;
}
//-----------------------------------------------------------------------
bool CompositionTargetPass::getOnlyInitial()
{
    return mOnlyInitial;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setVisibilityMask(uint32 mask)
{
    mVisibilityMask = mask;
}
//-----------------------------------------------------------------------
uint32 CompositionTargetPass::getVisibilityMask()
{
    return mVisibilityMask;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setLodBias(float bias)
{
    mLodBias = bias;
}
//-----------------------------------------------------------------------
float CompositionTargetPass::getLodBias()
{
    return mLodBias;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setMaterialScheme(const String& schemeName)
{
    mMaterialScheme = schemeName;
}
//-----------------------------------------------------------------------
const String& CompositionTargetPass::getMaterialScheme(void) const
{
    return mMaterialScheme;
}
//-----------------------------------------------------------------------
void CompositionTargetPass::setShadowsEnabled(bool enabled)
{
    mShadowsEnabled = enabled;
}
//-----------------------------------------------------------------------
bool CompositionTargetPass::getShadowsEnabled(void) const
{
    return mShadowsEnabled;
}
//-----------------------------------------------------------------------
CompositionPass *CompositionTargetPass::createPass(CompositionPass::PassType type)
{
    CompositionPass *t = OGRE_NEW CompositionPass(this);
    t->setType(type);
    mPasses.push_back(t);
    return t;
}
//-----------------------------------------------------------------------

void CompositionTargetPass::removePass(size_t index)
{
    assert (index < mPasses.size() && "Index out of bounds.");
    Passes::iterator i = mPasses.begin() + index;
    OGRE_DELETE (*i);
    mPasses.erase(i);
}
//-----------------------------------------------------------------------
void CompositionTargetPass::removeAllPasses()
{
    Passes::iterator i, iend;
    iend = mPasses.end();
    for (i = mPasses.begin(); i != iend; ++i)
    {
        OGRE_DELETE (*i);
    }
    mPasses.clear();
}
//-----------------------------------------------------------------------
CompositionTargetPass::PassIterator CompositionTargetPass::getPassIterator(void)
{
    return PassIterator(mPasses.begin(), mPasses.end());
}

//-----------------------------------------------------------------------
CompositionTechnique *CompositionTargetPass::getParent()
{
    return mParent;
}

//-----------------------------------------------------------------------
bool CompositionTargetPass::_isSupported(void)
{
    // A target pass is supported if all passes are supported

    Passes::const_iterator passi = mPasses.begin();
    for (;passi != mPasses.end(); ++passi)
    {
        CompositionPass* pass = *passi;
        if (!pass->_isSupported())
        {
            return false;
        }
    }

    return true;
}

}
