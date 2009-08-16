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
#include "OgrePredefinedControllers.h"

#include "OgreRoot.h"
#include "OgreMath.h"
#include "OgreLogManager.h"
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
        return (mTextureLayer->getCurrentFrame() / numFrames);
    }
    //-----------------------------------------------------------------------
    void TextureFrameControllerValue::setValue(Real value)
    {
        int numFrames = mTextureLayer->getNumFrames();
        mTextureLayer->setCurrentFrame((int)(value * numFrames) % numFrames);
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
        // Use looped subtract rather than divide / round
        while (input >= 1.0)
            input -= 1.0;
        while (input < 0.0)
            input += 1.0;

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
                output = 1.0 - ((input - 0.25) * 4);
            else
                output = ((input - 0.75) * 4) - 1.0;

            break;
        case WFT_SQUARE:
            if (input <= 0.5)
                output = 1.0;
            else
                output = -1.0;
            break;
        case WFT_SAWTOOTH:
            output = (input * 2) - 1;
            break;
        case WFT_INVERSE_SAWTOOTH:
            output = -((input * 2) - 1);
            break;
		case WFT_PWM:
			if( input <= mDutyCycle )
				output = 1.0;
			else
				output = -1.0;
			break;
        }

        // Scale output into 0..1 range and then by base + amplitude
        return mBase + ((output + 1.0) * 0.5 * mAmplitude);


    }
}

