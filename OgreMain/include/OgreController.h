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
#ifndef __Controller_H__
#define __Controller_H__

#include "OgrePrerequisites.h"

#include "OgreSharedPtr.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */
    
    
    /** Subclasses of this class are responsible for performing a function on an input value for a Controller.
        @remarks
            This abstract class provides the interface that needs to be supported for a custom function which
            can be 'plugged in' to a Controller instance, which controls some object value based on an input value.
            For example, the WaveControllerFunction class provided by Ogre allows you to use various waveforms to
            translate an input value to an output value.
        @par
            You are free to create your own subclasses in order to define any function you wish.
    */
    template <typename T>
    class ControllerFunction : public ControllerAlloc
    {
    protected:
        /// If true, function will add input values together and wrap at 1.0 before evaluating
        bool mDeltaInput;
        T mDeltaCount;

        /** Gets the input value as adjusted by any delta.
        */
        T getAdjustedInput(T input)
        {
            if (mDeltaInput)
            {
                // Wrap
                mDeltaCount = std::fmod(mDeltaCount + input, 1.0f);
                return mDeltaCount;
            }
            else
            {
                return input;
            }
        }

    public:
        /** Constructor.
            @param
                deltaInput If true, function will add input values together and wrap at 1.0 before evaluating
        */
        ControllerFunction(bool deltaInput)
        {
            mDeltaInput = deltaInput;
            mDeltaCount = 0;
        }

        virtual ~ControllerFunction() {}

        virtual T calculate(T sourceValue) = 0;
    };


    /** Can either be used as an input or output value.
    */
    template <typename T>
    class ControllerValue : public ControllerAlloc
    {

    public:
        virtual ~ControllerValue() { }
        virtual T getValue(void) const = 0;
        virtual void setValue(T value) = 0;

    };

    /** Instances of this class 'control' the value of another object in the system.
        @remarks
            Controller classes are used to manage the values of object automatically based
            on the value of some input. For example, a Controller could animate a texture
            by controlling the current frame of the texture based on time, or a different Controller
            could change the colour of a material used for a spaceship shield mesh based on the remaining
            shield power level of the ship.
        @par
            The Controller is an intentionally abstract concept - it can generate values
            based on input and a function, which can either be one of the standard ones
            supplied, or a function can be 'plugged in' for custom behaviour - see the ControllerFunction class for details.
            Both the input and output values are via ControllerValue objects, meaning that any value can be both
            input and output of the controller.
        @par
            Whilst this is very flexible, it can be a little bit confusing so to make it simpler the most often used
            controller setups are available by calling methods on the ControllerManager object.
        @see
            ControllerFunction

    */
    template <typename T>
    class Controller : public ControllerAlloc
    {
    protected:
        /// Source value
        SharedPtr< ControllerValue<T> > mSource;
        /// Destination value
        SharedPtr< ControllerValue<T> > mDest;
        /// Function
        SharedPtr< ControllerFunction<T> > mFunc;
        /// Controller is enabled or not
        bool mEnabled;


    public:

        /** Usual constructor.
            @remarks
                Requires source and destination values, and a function object. None of these are destroyed
                with the Controller when it is deleted (they can be shared) so you must delete these as appropriate.
        */
        Controller(const SharedPtr< ControllerValue<T> >& src, 
            const SharedPtr< ControllerValue<T> >& dest, const SharedPtr< ControllerFunction<T> >& func)
            : mSource(src), mDest(dest), mFunc(func)
        {
            mEnabled = true;
        }

        /** Default d-tor.
        */
        virtual ~Controller() {}


        /// Sets the input controller value
        void setSource(const SharedPtr< ControllerValue<T> >& src)
        {
            mSource = src;
        }
        /// Gets the input controller value
        const SharedPtr< ControllerValue<T> >& getSource(void) const
        {
            return mSource;
        }
        /// Sets the output controller value
        void setDestination(const SharedPtr< ControllerValue<T> >& dest)
        {
            mDest = dest;
        }

        /// Gets the output controller value
        const SharedPtr< ControllerValue<T> >& getDestination(void) const
        {
            return mDest;
        }

        /// Returns true if this controller is currently enabled
        bool getEnabled(void) const
        {
            return mEnabled;
        }

        /// Sets whether this controller is enabled
        void setEnabled(bool enabled)
        {
            mEnabled = enabled;
        }

        /** Sets the function object to be used by this controller.
        */
        void setFunction(const SharedPtr< ControllerFunction<T> >& func)
        {
            mFunc = func;
        }

        /** Returns a pointer to the function object used by this controller.
        */
        const SharedPtr< ControllerFunction<T> >& getFunction(void) const
        {
            return mFunc;
        }

        /** Tells this controller to map it's input controller value
            to it's output controller value, via the controller function. 
        @remarks
            This method is called automatically every frame by ControllerManager.
        */
        void update(void)
        {
            if(mEnabled)
                mDest->setValue(mFunc->calculate(mSource->getValue()));
        }

    };

    /** @} */
    /** @} */

}

#endif
