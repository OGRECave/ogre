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
#include "OgrePredefinedControllers.h"
#include "OgreTextureUnitState.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    // FrameTimeControllerValue
    //-----------------------------------------------------------------------
    FrameTimeControllerValue::FrameTimeControllerValue()
    {
        // Register self
        Root::getSingleton().addFrameListener(this);
        mFrameTime = 0;
        mTimeFactor = 1;
        mFrameDelay = 0;
        mElapsedTime = 0;

    }
    //-----------------------------------------------------------------------
    bool FrameTimeControllerValue::frameStarted(const FrameEvent &evt)
    {
        if(mFrameDelay) 
        {
            // Fixed frame time
            mFrameTime = mFrameDelay;
            mTimeFactor =  mFrameDelay / evt.timeSinceLastFrame;
        }
        else 
        {
            // Save the time value after applying time factor
            mFrameTime = mTimeFactor * evt.timeSinceLastFrame;
        }
        // Accumulate the elapsed time
        mElapsedTime += mFrameTime;
        return true;
    }
    //-----------------------------------------------------------------------
    bool FrameTimeControllerValue::frameEnded(const FrameEvent &evt)
    {
        return true;
    }
    //-----------------------------------------------------------------------
    Real FrameTimeControllerValue::getValue() const
    {
        return mFrameTime;
    }
    //-----------------------------------------------------------------------
    void FrameTimeControllerValue::setValue(Real value)
    {
        // Do nothing - value is set from frame listener
    }
    //-----------------------------------------------------------------------
    Real FrameTimeControllerValue::getTimeFactor(void) const {
        return mTimeFactor;
    }
    //-----------------------------------------------------------------------
    void FrameTimeControllerValue::setTimeFactor(Real tf) {
        if(tf >= 0) 
        {
            mTimeFactor = tf;
            mFrameDelay = 0;
        }
    }
    //-----------------------------------------------------------------------
    Real FrameTimeControllerValue::getFrameDelay(void) const {
        return mFrameDelay;
    }
    //-----------------------------------------------------------------------
    void FrameTimeControllerValue::setFrameDelay(Real fd) {
        mTimeFactor = 0;
        mFrameDelay = fd;
    }
    //-----------------------------------------------------------------------
    Real FrameTimeControllerValue::getElapsedTime(void) const
    {
        return mElapsedTime;
    }
    //-----------------------------------------------------------------------
    void FrameTimeControllerValue::setElapsedTime(Real elapsedTime)
    {
        mElapsedTime = elapsedTime;
    }
    //-----------------------------------------------------------------------
    // TextureFrameControllerValue
    //-----------------------------------------------------------------------
    TextureFrameControllerValue::TextureFrameControllerValue(TextureUnitState* t)
    {
        mTextureLayer = t;
    }
    //-----------------------------------------------------------------------
    Real TextureFrameControllerValue::getValue(void) const
    {
        int numFrames = mTextureLayer->getNumFrames();
        return ((Real)mTextureLayer->getCurrentFrame() / (Real)numFrames);
    }
    //-----------------------------------------------------------------------
    void TextureFrameControllerValue::setValue(Real value)
    {
        int numFrames = mTextureLayer->getNumFrames();
        mTextureLayer->setCurrentFrame(numFrames ? (int)(value * numFrames) % numFrames : 0);
    }
    //-----------------------------------------------------------------------
    // TexCoordModifierControllerValue
    //-----------------------------------------------------------------------
    TexCoordModifierControllerValue::TexCoordModifierControllerValue(TextureUnitState* t,
        bool translateU, bool translateV, bool scaleU, bool scaleV, bool rotate )
    {
        mTextureLayer = t;
        mTransU = translateU;
        mTransV = translateV;
        mScaleU = scaleU;
        mScaleV = scaleV;
        mRotate = rotate;
    }
    //-----------------------------------------------------------------------
    Real TexCoordModifierControllerValue::getValue() const
    {
        const Matrix4& pMat = mTextureLayer->getTextureTransform();
        if (mTransU)
        {
            return pMat[0][3];
        }
        else if (mTransV)
        {
            return pMat[1][3];
        }
        else if (mScaleU)
        {
            return pMat[0][0];
        }
        else if (mScaleV)
        {
            return pMat[1][1];
        }
        // Shouldn't get here
        return 0;
    }
    //-----------------------------------------------------------------------
    void TexCoordModifierControllerValue::setValue(Real value)
    {
        if (mTransU)
        {
            mTextureLayer->setTextureUScroll(value);
        }
        if (mTransV)
        {
            mTextureLayer->setTextureVScroll(value);
        }
        if (mScaleU)
        {
            mTextureLayer->setTextureUScale(value);
        }
        if (mScaleV)
        {
            mTextureLayer->setTextureVScale(value);
        }
        if (mRotate)
        {
            mTextureLayer->setTextureRotate(Radian(value * Math::TWO_PI));
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    FloatGpuParameterControllerValue::FloatGpuParameterControllerValue(
            GpuProgramParametersSharedPtr params, size_t index) :
        mParams(params), mParamIndex(index)
    {
    }
    //-----------------------------------------------------------------------
    Real FloatGpuParameterControllerValue::getValue(void) const
    {
        // do nothing, reading from a set of params not supported
        return 0.0f;
    }
    //-----------------------------------------------------------------------
    void FloatGpuParameterControllerValue::setValue(Real val)
    {
        Vector4 v4 = Vector4(0,0,0,0);
        v4.x = val;
        mParams->setConstant(mParamIndex, v4);
    }
    //-----------------------------------------------------------------------
    // PassthroughControllerFunction
    //-----------------------------------------------------------------------
    PassthroughControllerFunction::PassthroughControllerFunction(bool delta) 
        : ControllerFunction<Real>(delta)
    {
    }
    //-----------------------------------------------------------------------
    Real PassthroughControllerFunction::calculate(Real source)
    {
        return getAdjustedInput(source);

    }
    //-----------------------------------------------------------------------
    // AnimationControllerFunction
    //-----------------------------------------------------------------------
    AnimationControllerFunction::AnimationControllerFunction(Real sequenceTime, Real timeOffset) 
        : ControllerFunction<Real>(false)
    {
        mSeqTime = sequenceTime;
        mTime = timeOffset;
    }
    //-----------------------------------------------------------------------
    Real AnimationControllerFunction::calculate(Real source)
    {
        // Assume source is time since last update
        mTime += source;
        // Wrap
        while (mTime >= mSeqTime) mTime -= mSeqTime;
        while (mTime < 0) mTime += mSeqTime;

        // Return parametric
        return mTime / mSeqTime;
    }
    //-----------------------------------------------------------------------
    void AnimationControllerFunction::setTime(Real timeVal)
    {
        mTime = timeVal;
    }
    //-----------------------------------------------------------------------
    void AnimationControllerFunction::setSequenceTime(Real seqVal)
    {
        mSeqTime = seqVal;
    }
    //-----------------------------------------------------------------------
    // ScaleControllerFunction
    //-----------------------------------------------------------------------
    ScaleControllerFunction::ScaleControllerFunction(Real factor, bool delta) : ControllerFunction<Real>(delta)
    {
        mScale = factor;
    }
    //-----------------------------------------------------------------------
    Real ScaleControllerFunction::calculate(Real source)
    {
        return getAdjustedInput(source * mScale);

    }
    //-----------------------------------------------------------------------
    // WaveformControllerFunction
    //-----------------------------------------------------------------------
    WaveformControllerFunction::WaveformControllerFunction(WaveformType wType, Real base,  Real frequency, Real phase, Real amplitude, bool delta, Real dutyCycle)
        :ControllerFunction<Real>(delta)
    {
        mWaveType = wType;
        mBase = base;
        mFrequency = frequency;
        mPhase = phase;
        mAmplitude = amplitude;
        mDeltaCount = phase;
        mDutyCycle = dutyCycle;
    }
    //-----------------------------------------------------------------------
    Real WaveformControllerFunction::getAdjustedInput(Real input)
    {
        Real adjusted = ControllerFunction<Real>::getAdjustedInput(input);

        // If not delta, adjust by phase here
        // (delta inputs have it adjusted at initialisation)
        if (!mDeltaInput)
        {
            adjusted += mPhase;
        }

        return adjusted;
    }
    //-----------------------------------------------------------------------
    Real WaveformControllerFunction::calculate(Real source)
    {
        Real input = getAdjustedInput(source * mFrequency);
        Real output = 0;
        // For simplicity, factor input down to {0,1)
        input = std::fmod(input, Real(1));
        if(input < 0) input += 1;

        // Calculate output in -1..1 range
        switch (mWaveType)
        {
        case WFT_SINE:
            output = Math::Sin(Radian(input * Math::TWO_PI));
            break;
        case WFT_TRIANGLE:
            if (input < 0.25)
                output = input * 4;
            else if (input >= 0.25 && input < 0.75)
                output = 1.0f - ((input - 0.25f) * 4.0f);
            else
                output = ((input - 0.75f) * 4.0f) - 1.0f;

            break;
        case WFT_SQUARE:
            if (input <= 0.5f)
                output = 1.0f;
            else
                output = -1.0f;
            break;
        case WFT_SAWTOOTH:
            output = (input * 2.0f) - 1.0f;
            break;
        case WFT_INVERSE_SAWTOOTH:
            output = -((input * 2.0f) - 1.0f);
            break;
        case WFT_PWM:
            if( input <= mDutyCycle )
                output = 1.0f;
            else
                output = -1.0f;
            break;
        }

        // Scale output into 0..1 range and then by base + amplitude
        return mBase + ((output + 1.0f) * 0.5f * mAmplitude);


    }
    //-----------------------------------------------------------------------
    // LinearControllerFunction
    //-----------------------------------------------------------------------
    LinearControllerFunction::LinearControllerFunction(const std::vector<Real>& keys, const std::vector<Real>& values, Real frequency, bool deltaInput) :
            ControllerFunction<Real>(deltaInput), mFrequency(frequency), mKeys(keys), mValues(values) {
        assert(mKeys.size() == mValues.size());
    }
    //-----------------------------------------------------------------------
    Real LinearControllerFunction::calculate(Real source) {
        Real input = getAdjustedInput(source*mFrequency);

        std::vector<Real>::iterator ifirst = std::lower_bound(mKeys.begin(), mKeys.end(), input);
        size_t idx = ifirst - mKeys.begin() - 1;

        assert(ifirst != mKeys.end());

        Real alpha = (input - mKeys[idx])/(mKeys[idx + 1] - mKeys[idx]);
        return mValues[idx] + alpha * (mValues[idx + 1] - mValues[idx]);
    }
}

