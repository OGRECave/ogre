/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreGpuProgram.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	//-----------------------------------------------------------------------
    // Controller Values
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting the latest frame time.
    */
    class _OgreExport FrameTimeControllerValue : public ControllerValue<Real>, public FrameListener
    {
    protected:
        Real mFrameTime;
		Real mTimeFactor;
		Real mElapsedTime;
		Real mFrameDelay;

    public:
        FrameTimeControllerValue();
        bool frameEnded(const FrameEvent &evt);
        bool frameStarted(const FrameEvent &evt);
        Real getValue(void) const;
        void setValue(Real value);
		Real getTimeFactor(void) const;
		void setTimeFactor(Real tf);
		Real getFrameDelay(void) const;
		void setFrameDelay(Real fd);
		Real getElapsedTime(void) const;
		void setElapsedTime(Real elapsedTime);
    };

    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting the frame number of a texture layer
    */
    class _OgreExport TextureFrameControllerValue : public ControllerValue<Real>
    {
    protected:
        TextureUnitState* mTextureLayer;
    public:
        TextureFrameControllerValue(TextureUnitState* t);

        /** Gets the frame number as a parametric value in the range [0,1]
        */
        Real getValue(void) const;
        /** Sets the frame number as a parametric value in the range [0,1]; the actual frame number is (value * numFrames) % numFrames).
        */
        void setValue(Real value);

    };
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting a texture coordinate modifications (scales and translates).
        @remarks
            Effects can be applied to the scale or the offset of the u or v coordinates, or both. If separate
            modifications are required to u and v then 2 instances are required to control both independently, or 4
            if you want separate u and v scales as well as separate u and v offsets.
        @par
            Because of the nature of this value, it can accept values outside the 0..1 parametric range.
    */
    class _OgreExport TexCoordModifierControllerValue : public ControllerValue<Real>
    {
    protected:
        bool mTransU, mTransV;
        bool mScaleU, mScaleV;
        bool mRotate;
        TextureUnitState* mTextureLayer;
    public:
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
        TexCoordModifierControllerValue(TextureUnitState* t, bool translateU = false, bool translateV = false,
            bool scaleU = false, bool scaleV = false, bool rotate = false );

        Real getValue(void) const;
        void setValue(Real value);

    };

    //-----------------------------------------------------------------------
    /** Predefined controller value for setting a single floating-
	    point value in a constant parameter of a vertex or fragment program.
    @remarks
		Any value is accepted, it is propagated into the 'x'
		component of the constant register identified by the index. If you
		need to use named parameters, retrieve the index from the param
		object before setting this controller up.
	@note
		Retrieving a value from the program parameters is not currently 
		supported, therefore do not use this controller value as a source,
		only as a target.
    */
    class _OgreExport FloatGpuParameterControllerValue : public ControllerValue<Real>
    {
    protected:
		/// The parameters to access
		GpuProgramParametersSharedPtr mParams;
		/// The index of the parameter to be read or set
		size_t mParamIndex;
    public:
        /** Constructor.
		    @param
				params The parameters object to access
            @param
                index The index of the parameter to be set
        */
        FloatGpuParameterControllerValue(GpuProgramParametersSharedPtr params,
				size_t index );

        ~FloatGpuParameterControllerValue() {}

        Real getValue(void) const;
        void setValue(Real value);

    };
    //-----------------------------------------------------------------------
    // Controller functions
    //-----------------------------------------------------------------------

	/** Predefined controller function which just passes through the original source
	directly to dest.
	*/
	class _OgreExport PassthroughControllerFunction : public ControllerFunction<Real>
	{
	public:
		/** Constructor.
         @param
             deltaInput If true, signifies that the input will be a delta value such that the function should
             add it to an internal counter before calculating the output.
		*/
		PassthroughControllerFunction(bool deltaInput = false);

		/** Overridden function.
		*/
		Real calculate(Real source);
	};

	/** Predefined controller function for dealing with animation.
    */
    class _OgreExport AnimationControllerFunction : public ControllerFunction<Real>
    {
    protected:
        Real mSeqTime;
        Real mTime;
    public:
        /** Constructor.
            @param
                sequenceTime The amount of time in seconds it takes to loop through the whole animation sequence.
            @param
                timeOffset The offset in seconds at which to start (default is start at 0)
        */
        AnimationControllerFunction(Real sequenceTime, Real timeOffset = 0.0f);

        /** Overridden function.
        */
        Real calculate(Real source);

		/** Set the time value manually. */
		void setTime(Real timeVal);
		/** Set the sequence duration value manually. */
		void setSequenceTime(Real seqVal);
    };

	//-----------------------------------------------------------------------
    /** Predefined controller function which simply scales an input to an output value.
    */
    class _OgreExport ScaleControllerFunction : public ControllerFunction<Real>
    {
    protected:
        Real mScale;
    public:
        /** Constructor, requires a scale factor.
            @param
                scalefactor The multiplier applied to the input to produce the output.
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                 add it to an internal counter before calculating the output.
        */
        ScaleControllerFunction(Real scalefactor, bool deltaInput);

        /** Overridden method.
        */
        Real calculate(Real source);

    };

    //-----------------------------------------------------------------------
    /** Predefined controller function based on a waveform.
        @remarks
            A waveform function translates parametric input to parametric output based on a wave. The factors
            affecting the function are:
            - wave type - the shape of the wave
            - base - the base value of the output from the wave
            - frequency - the speed of the wave in cycles per second
            - phase - the offset of the start of the wave, e.g. 0.5 to start half-way through the wave
            - amplitude - scales the output so that instead of lying within [0,1] it lies within [0,1] * amplitude
			- duty cycle - the active width of a PWM signal
        @par
            Note that for simplicity of integration with the rest of the controller insfrastructure, the output of
            the wave is parametric i.e. 0..1, rather than the typical wave output of [-1,1]. To compensate for this, the
            traditional output of the wave is scaled by the following function before output:
        @par
            output = (waveoutput + 1) * 0.5
        @par
            Hence a wave output of -1 becomes 0, a wave output of 1 becomes 1, and a wave output of 0 becomes 0.5.
    */
    class _OgreExport WaveformControllerFunction : public ControllerFunction<Real>
    {
    protected:
        WaveformType mWaveType;
        Real mBase;
        Real mFrequency;
        Real mPhase;
        Real mAmplitude;
		Real mDutyCycle;

        /** Overridden from ControllerFunction. */
        Real getAdjustedInput(Real input);

    public:
        /** Default constructor, requires at least a wave type, other parameters can be defaulted unless required.
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                add it to an internal counter before calculating the output.
			@param
				dutyCycle Used in PWM mode to specify the pulse width.
        */
        WaveformControllerFunction(WaveformType wType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1, bool deltaInput = true, Real dutyCycle = 0.5);

        /** Overridden function.
        */
        Real calculate(Real source);

    };
    //-----------------------------------------------------------------------
	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
