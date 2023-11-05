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
#include "OgreControllerManager.h"

#include "OgrePredefinedControllers.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> ControllerManager* Singleton<ControllerManager>::msSingleton = 0;
    ControllerManager* ControllerManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    ControllerManager& ControllerManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    ControllerManager::ControllerManager()
        : mFrameTimeController(OGRE_NEW FrameTimeControllerValue())
        , mPassthroughFunction(OGRE_NEW PassthroughControllerFunction())
        , mLastFrameNumber(0)
    {

    }
    //-----------------------------------------------------------------------
    ControllerManager::~ControllerManager()
    {
        clearControllers();
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createController(
        const ControllerValueRealPtr& src, const ControllerValueRealPtr& dest,
        const ControllerFunctionRealPtr& func)
    {
        ControllerFloat* c = OGRE_NEW ControllerFloat(src, dest, func);

        mControllers.push_back(c);
        return c;
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createFrameTimePassthroughController(
        const ControllerValueRealPtr& dest)
    {
        return createController(getFrameTimeSource(), dest, getPassthroughControllerFunction());
    }
    //-----------------------------------------------------------------------
    void ControllerManager::updateAllControllers(void)
    {
        // Only update once per frame
        unsigned long thisFrameNumber = Root::getSingleton().getNextFrameNumber();
        if (thisFrameNumber != mLastFrameNumber)
        {
            for (auto *ci : mControllers)
            {
                ci->update();
            }
            mLastFrameNumber = thisFrameNumber;
        }
    }
    //-----------------------------------------------------------------------
    void ControllerManager::clearControllers(void)
    {
        for (auto *ci : mControllers)
        {
            OGRE_DELETE ci;
        }
        mControllers.clear();
    }
    //-----------------------------------------------------------------------
    const ControllerValueRealPtr& ControllerManager::getFrameTimeSource(void) const
    {
        return mFrameTimeController;
    }
    //-----------------------------------------------------------------------
    const ControllerFunctionRealPtr& ControllerManager::getPassthroughControllerFunction(void) const
    {
        return mPassthroughFunction;
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureAnimator(TextureUnitState* layer, Real sequenceTime)
    {
        return createController(mFrameTimeController, TextureFrameControllerValue::create(layer),
                                AnimationControllerFunction::create(sequenceTime));
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureUVScroller(TextureUnitState* layer, Real speed)
    {
        ControllerFloat* ret = 0;

        if (speed != 0)
        {
            // We do both scrolls with a single controller
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            ret = createController(mFrameTimeController, TexCoordModifierControllerValue::create(layer, true, true),
                                   ScaleControllerFunction::create(-speed, true));
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureUScroller(TextureUnitState* layer, Real uSpeed)
    {
        ControllerFloat* ret = 0;

        if (uSpeed != 0)
        {
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            ret = createController(mFrameTimeController, TexCoordModifierControllerValue::create(layer, true),
                                   ScaleControllerFunction::create(-uSpeed, true));
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureVScroller(TextureUnitState* layer, Real vSpeed)
    {
        ControllerFloat* ret = 0;

        if (vSpeed != 0)
        {
            // Set up a second controller for v scroll
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            ret = createController(mFrameTimeController, TexCoordModifierControllerValue::create(layer, false, true),
                                   ScaleControllerFunction::create(-vSpeed, true));
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureRotater(TextureUnitState* layer, Real speed)
    {
        // Target value is texture coord rotation
        // Function is simple scale (seconds * speed)
        // Use -speed since altering texture coords has the reverse visible effect
        return createController(mFrameTimeController,
                                TexCoordModifierControllerValue::create(layer, false, false, false, false, true),
                                ScaleControllerFunction::create(-speed, true));
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createTextureWaveTransformer(TextureUnitState* layer,
        TextureUnitState::TextureTransformType ttype, WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        ControllerValueRealPtr val;

        switch (ttype)
        {
        case TextureUnitState::TT_TRANSLATE_U:
            // Target value is a u scroll
            val = TexCoordModifierControllerValue::create(layer, true);
            break;
        case TextureUnitState::TT_TRANSLATE_V:
            // Target value is a v scroll
            val = TexCoordModifierControllerValue::create(layer, false, true);
            break;
        case TextureUnitState::TT_SCALE_U:
            // Target value is a u scale
            val = TexCoordModifierControllerValue::create(layer, false, false, true);
            break;
        case TextureUnitState::TT_SCALE_V:
            // Target value is a v scale
            val = TexCoordModifierControllerValue::create(layer, false, false, false, true);
            break;
        case TextureUnitState::TT_ROTATE:
            // Target value is texture coord rotation
            val = TexCoordModifierControllerValue::create(layer, false, false, false, false, true);
            break;
        }
        // Create new wave function for alterations
        return createController(mFrameTimeController, val,
                                WaveformControllerFunction::create(waveType, base, frequency, phase, amplitude, true));
    }
    //-----------------------------------------------------------------------
    ControllerFloat* ControllerManager::createGpuProgramTimerParam(
        GpuProgramParametersSharedPtr params, size_t paramIndex, Real timeFactor)
    {
        return createController(mFrameTimeController, FloatGpuParameterControllerValue::create(params, paramIndex),
                                ScaleControllerFunction::create(timeFactor, true));
    }
    //-----------------------------------------------------------------------
    void ControllerManager::destroyController(ControllerFloat* controller)
    {
        ControllerList::iterator i = std::find(mControllers.begin(), mControllers.end(), controller);
        if (i != mControllers.end())
        {
            std::swap(*i, mControllers.back());
            mControllers.pop_back();
            OGRE_DELETE controller;
        }
    }
    //-----------------------------------------------------------------------
    Real ControllerManager::getTimeFactor(void) const {
        return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getTimeFactor();
    }
    //-----------------------------------------------------------------------
    void ControllerManager::setTimeFactor(Real tf) {
        static_cast<FrameTimeControllerValue*>(mFrameTimeController.get())->setTimeFactor(tf);
    }
    //-----------------------------------------------------------------------
    Real ControllerManager::getFrameDelay(void) const {
        return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getFrameDelay();
    }
    //-----------------------------------------------------------------------
    void ControllerManager::setFrameDelay(Real fd) {
        static_cast<FrameTimeControllerValue*>(mFrameTimeController.get())->setFrameDelay(fd);
    }
    //-----------------------------------------------------------------------
    Real ControllerManager::getElapsedTime(void) const
    {
        return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getElapsedTime();
    }
    //-----------------------------------------------------------------------
    void ControllerManager::setElapsedTime(Real elapsedTime)
    {
        static_cast<FrameTimeControllerValue*>(mFrameTimeController.get())->setElapsedTime(elapsedTime);
    }
}
