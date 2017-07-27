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
#ifndef __Particle_H__
#define __Particle_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreColourValue.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    /** Abstract class containing any additional data required to be associated
        with a particle to perform the required rendering. 
    @remarks
        Because you can specialise the way that particles are rendered by supplying
        custom ParticleSystemRenderer classes, you might well need some additional 
        data for your custom rendering routine which is not held on the default particle
        class. If that's the case, then you should define a subclass of this class, 
        and construct it when asked in your custom ParticleSystemRenderer class.
    */
    class _OgreExport ParticleVisualData : public FXAlloc
    {
    public:
        ParticleVisualData() {}
        virtual ~ParticleVisualData() {}

    };

    /** Class representing a single particle instance. */
    class _OgreExport Particle : public FXAlloc
    {
    protected:
        /// Parent ParticleSystem
        ParticleSystem* mParentSystem;
        /// Additional visual data you might want to associate with the Particle
        ParticleVisualData* mVisual;
    public:
        /// Type of particle
        enum ParticleType
        {
            Visual,
            Emitter
        };

        /// Does this particle have it's own dimensions?
        bool mOwnDimensions;
        /// Personal width if mOwnDimensions == true
        Real mWidth;
        /// Personal height if mOwnDimensions == true
        Real mHeight;
        /// Current rotation value
        Radian mRotation;
        // Note the intentional public access to internal variables
        // Accessing via get/set would be too costly for 000's of particles
        /// World position
        Vector3 mPosition;
        /// Direction (and speed) 
        Vector3 mDirection;
        /// Current colour
        ColourValue mColour;
        /// Time to live, number of seconds left of particles natural life
        Real mTimeToLive;
        /// Total Time to live, number of seconds of particles natural life
        Real mTotalTimeToLive;
        /// Speed of rotation in radians/sec
        Radian mRotationSpeed;
        /// Determines the type of particle.
        ParticleType mParticleType;

        Particle()
            : mParentSystem(0), mVisual(0), mOwnDimensions(false), mWidth(0), mHeight(0),
            mRotation(0), mPosition(Vector3::ZERO), mDirection(Vector3::ZERO),
            mColour(ColourValue::White), mTimeToLive(10), mTotalTimeToLive(10),
            mRotationSpeed(0), mParticleType(Visual)
        {
        }

        /** Sets the width and height for this particle.
        @remarks
        Note that it is most efficient for every particle in a ParticleSystem to have the same dimensions. If you
        choose to alter the dimensions of an individual particle the set will be less efficient. Do not call
        this method unless you really need to have different particle dimensions within the same set. Otherwise
        just call the ParticleSystem::setDefaultDimensions method instead.
        */
        void setDimensions(Real width, Real height); 

        /** Returns true if this particle deviates from the ParticleSystem's default dimensions (i.e. if the
        particle::setDimensions method has been called for this instance).
        @see
        particle::setDimensions
        */
        bool hasOwnDimensions(void) const { return mOwnDimensions; }

        /** Retrieves the particle's personal width, if hasOwnDimensions is true. */
        Real getOwnWidth(void) const { return mWidth; }

        /** Retrieves the particle's personal width, if hasOwnDimensions is true. */
        Real getOwnHeight(void) const { return mHeight; }
        
        /** Sets the current rotation */
        void setRotation(const Radian& rad);

        const Radian& getRotation(void) const { return mRotation; }

        /** Internal method for notifying the particle of it's owner.
        */
        void _notifyOwner(ParticleSystem* owner);

        /** Internal method for notifying the particle of it's optional visual data.
        */
        void _notifyVisualData(ParticleVisualData* vis) { mVisual = vis; }

        /// Get the optional visual data associated with the class
        ParticleVisualData* getVisualData(void) const { return mVisual; }

        /// Utility method to reset this particle
        void resetDimensions(void);
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

