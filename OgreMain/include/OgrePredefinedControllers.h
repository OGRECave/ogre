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
#ifndef __PredefinedControllers_H__
#define __PredefinedControllers_H__

#include "OgrePrerequisites.h"

#include "OgreCommon.h"
#include "OgreController.h"
#include "OgreFrameListener.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */
    //-----------------------------------------------------------------------
    // Controller Values
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting the latest frame time.
    */
    class _OgreExport FrameTimeControllerValue : public ControllerValue<float>, private FrameListener
    {
    private:
        Real mFrameTime;
        Real mTimeFactor;
        Real mElapsedTime;
        Real mFrameDelay;

        bool frameStarted(const FrameEvent &evt) override;
    public:
        /// @deprecated use create()
        FrameTimeControllerValue();

        static ControllerValueRealPtr create() { return std::make_shared<FrameTimeControllerValue>(); }

        float getValue(void) const override { return mFrameTime; }
        void setValue(float value) override { /* Do nothing - value is set from frame listener */ }
        Real getTimeFactor(void) const { return mTimeFactor; }
        /// @copydoc ControllerManager::setTimeFactor
        void setTimeFactor(Real tf);
        Real getFrameDelay(void) const { return mFrameDelay; }
        /// @copydoc ControllerManager::setFrameDelay
        void setFrameDelay(Real fd);
        Real getElapsedTime(void) const { return mElapsedTime; }
        void setElapsedTime(Real elapsedTime) { mElapsedTime = elapsedTime; }
    };

    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting the frame number of a texture layer
    */
    class _OgreExport TextureFrameControllerValue : public ControllerValue<float>
    {
    private:
        TextureUnitState* mTextureLayer;
    public:
        /// @deprecated use create()
        TextureFrameControllerValue(TextureUnitState* t);

        static ControllerValueRealPtr create(TextureUnitState* t)
        {
            return std::make_shared<TextureFrameControllerValue>(t);
        }

        /** Gets the frame number as a parametric value in the range [0,1]
        */
        float getValue(void) const override;
        /** Sets the frame number as a parametric value in the range [0,1]; the actual frame number is (value * numFrames) % numFrames).
        */
        void setValue(float value) override;

    };
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting a texture coordinate modifications (scales and translates).

            Effects can be applied to the scale or the offset of the u or v coordinates, or both. If separate
            modifications are required to u and v then 2 instances are required to control both independently, or 4
            if you want separate u and v scales as well as separate u and v offsets.
        @par
            Because of the nature of this value, it can accept values outside the 0..1 parametric range.
    */
    class _OgreExport TexCoordModifierControllerValue : public ControllerValue<float>
    {
    private:
        bool mTransU, mTransV;
        bool mScaleU, mScaleV;
        bool mRotate;
        TextureUnitState* mTextureLayer;
    public:
        /// @deprecated use create
        TexCoordModifierControllerValue(TextureUnitState* t, bool translateU = false, bool translateV = false,
            bool scaleU = false, bool scaleV = false, bool rotate = false );

        /** Constructor.
            @param
                t TextureUnitState to apply the modification to.
            @param
                translateU If true, the u coordinates will be translated by the modification.
            @param
                translateV If true, the v coordinates will be translated by the modification.
            @param
                scaleU If true, the u coordinates will be scaled by the modification.
            @param
                scaleV If true, the v coordinates will be scaled by the modification.
            @param
                rotate If true, the texture will be rotated by the modification.
        */
        static ControllerValueRealPtr create(TextureUnitState* t, bool translateU = false, bool translateV = false,
                                             bool scaleU = false, bool scaleV = false, bool rotate = false)
        {
            return std::make_shared<TexCoordModifierControllerValue>(t, translateU, translateV, scaleU, scaleV, rotate);
        }

        float getValue(void) const override;
        void setValue(float value) override;

    };

    //-----------------------------------------------------------------------
    /** Predefined controller value for setting a single floating-
        point value in a constant parameter of a vertex or fragment program.

        Any value is accepted, it is propagated into the 'x'
        component of the constant register identified by the index. If you
        need to use named parameters, retrieve the index from the param
        object before setting this controller up.
    @note
        Retrieving a value from the program parameters is not currently
        supported, therefore do not use this controller value as a source,
        only as a target.
    */
    class _OgreExport FloatGpuParameterControllerValue : public ControllerValue<float>
    {
    private:
        /// The parameters to access
        GpuProgramParametersSharedPtr mParams;
        /// The index of the parameter to be read or set
        size_t mParamIndex;
    public:
        /// @deprecated use create()
        FloatGpuParameterControllerValue(const GpuProgramParametersSharedPtr& params, size_t index);

        /** Constructor.
            @param
                params The parameters object to access
            @param
                index The index of the parameter to be set
        */
        static ControllerValueRealPtr create(const GpuProgramParametersSharedPtr& params, size_t index)
        {
            return std::make_shared<FloatGpuParameterControllerValue>(params, index);
        }

        float getValue(void) const override;
        void setValue(float value) override;

    };
    //-----------------------------------------------------------------------
    // Controller functions
    //-----------------------------------------------------------------------

    /** Predefined controller function which just passes through the original source
    directly to dest.
    */
    class _OgreExport PassthroughControllerFunction : public ControllerFunction<float>
    {
    public:
        /// @deprecated use create()
        PassthroughControllerFunction(bool deltaInput = false);

        /// @copydoc ControllerFunction::ControllerFunction
        static ControllerFunctionRealPtr create(bool deltaInput = false)
        {
            return std::make_shared<PassthroughControllerFunction>(deltaInput);
        }

        float calculate(float source) override;
    };

    /** Predefined controller function for dealing with animation.
    */
    class _OgreExport AnimationControllerFunction : public ControllerFunction<float>
    {
    private:
        Real mSeqTime;
        Real mTime;
    public:
        /// @deprecated use create()
        AnimationControllerFunction(Real sequenceTime, Real timeOffset = 0.0f);

        /** Constructor.
            @param
                sequenceTime The amount of time in seconds it takes to loop through the whole animation sequence.
            @param
                timeOffset The offset in seconds at which to start (default is start at 0)
        */
        static ControllerFunctionRealPtr create(Real sequenceTime, Real timeOffset = 0.0f)
        {
            return std::make_shared<AnimationControllerFunction>(sequenceTime, timeOffset);
        }

        float calculate(float source) override;

        /** Set the time value manually. */
        void setTime(Real timeVal);
        /** Set the sequence duration value manually. */
        void setSequenceTime(Real seqVal);
    };

    //-----------------------------------------------------------------------
    /** Predefined controller function which simply scales an input to an output value.
    */
    class _OgreExport ScaleControllerFunction : public ControllerFunction<float>
    {
    private:
        Real mScale;
    public:
        /// @deprecated use create()
        ScaleControllerFunction(Real scalefactor, bool deltaInput);

        /** Constructor, requires a scale factor.
            @param
                scalefactor The multiplier applied to the input to produce the output.
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                 add it to an internal counter before calculating the output.
        */
        static ControllerFunctionRealPtr create(Real scalefactor, bool deltaInput = false)
        {
            return std::make_shared<ScaleControllerFunction>(scalefactor, deltaInput);
        }

        float calculate(float source) override;
    };

    //-----------------------------------------------------------------------
    /** Predefined controller function based on a waveform.

            A waveform function translates parametric input to parametric output based on a wave.
        @par
            Note that for simplicity of integration with the rest of the controller insfrastructure, the output of
            the wave is parametric i.e. 0..1, rather than the typical wave output of [-1,1]. To compensate for this, the
            traditional output of the wave is scaled by the following function before output:
        @par
            output = (waveoutput + 1) * 0.5
        @par
            Hence a wave output of -1 becomes 0, a wave output of 1 becomes 1, and a wave output of 0 becomes 0.5.
    */
    class _OgreExport WaveformControllerFunction : public ControllerFunction<float>
    {
    private:
        WaveformType mWaveType;
        Real mBase;
        Real mFrequency;
        Real mPhase;
        Real mAmplitude;
        Real mDutyCycle;

        /** Overridden from ControllerFunction. */
        Real getAdjustedInput(Real input);

    public:
        /// @deprecated use create()
        WaveformControllerFunction(WaveformType wType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1, bool deltaInput = true, Real dutyCycle = 0.5);

        /** Default constructor, requires at least a wave type, other parameters can be defaulted unless required.
            @param wType the shape of the wave
            @param base the base value of the output from the wave
            @param frequency the speed of the wave in cycles per second
            @param phase the offset of the start of the wave, e.g. 0.5 to start half-way through the wave
            @param amplitude scales the output so that instead of lying within [0,1] it lies within [0,1] * amplitude
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                add it to an internal counter before calculating the output.
            @param
                dutyCycle Used in PWM mode to specify the pulse width.
        */
        static ControllerFunctionRealPtr create(WaveformType wType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1, bool deltaInput = true, Real dutyCycle = 0.5)
        {
            return std::make_shared<WaveformControllerFunction>(wType, base, frequency, phase, amplitude, deltaInput, dutyCycle);
        }

        float calculate(float source) override;
    };

    //-----------------------------------------------------------------------
    /** Predefined controller function based on linear function interpolation.
    */
    class _OgreExport LinearControllerFunction : public ControllerFunction<float> {
        Real mFrequency;
        std::vector<Real> mKeys;
        std::vector<Real> mValues;
    public:
        /// @deprecated use create()
        LinearControllerFunction(const std::vector<Real>& keys, const std::vector<Real>& values, Real frequency = 1, bool deltaInput = true);

        /** Constructor, requires keys and values of the function to interpolate

            For simplicity and compatibility with the predefined ControllerValue classes the function domain must be [0,1].
            However, you can use the frequency parameter to rescale the domain to a different range.
            @param
                keys the x-values of the function sampling points. Value range is [0,1]. Must include at least the keys 0 and 1.
            @param
                values the function values f(x) of the function. order must match keys
            @param frequency the speed of the evaluation in cycles per second
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                 add it to an internal counter before calculating the output.
            @note
                there must be the same amount of keys and values
        */
        static ControllerFunctionRealPtr create(const std::vector<Real>& keys, const std::vector<Real>& values, Real frequency = 1, bool deltaInput = true)
        {
            return std::make_shared<LinearControllerFunction>(keys, values, frequency, deltaInput);
        }

        float calculate(float source) override;
    };
    //-----------------------------------------------------------------------
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
