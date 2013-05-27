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
#ifndef __ParticleEmitter_H__
#define __ParticleEmitter_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreVector3.h"
#include "OgreColourValue.h"
#include "OgreStringInterface.h"
#include "OgreParticleEmitterCommands.h"
#include "OgreParticle.h"
#include "OgreHeaderPrefix.h"


namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Abstract class defining the interface to be implemented by particle emitters.
    @remarks
        Particle emitters are the sources of particles in a particle system. 
        This class defines the ParticleEmitter interface, and provides a basic implementation 
        for tasks which most emitters will do (these are of course overridable).
        Particle emitters can be  grouped into types, e.g. 'point' emitters, 'box' emitters etc; each type will 
        create particles with a different starting point, direction and velocity (although
        within the types you can configure the ranges of these parameters). 
    @par
        Because there are so many types of emitters you could use, OGRE chooses not to dictate
        the available types. It comes with some in-built, but allows plugins or applications to extend the emitter types available.
        This is done by subclassing ParticleEmitter to have the appropriate emission behaviour you want,
        and also creating a subclass of ParticleEmitterFactory which is responsible for creating instances 
        of your new emitter type. You register this factory with the ParticleSystemManager using
        addEmitterFactory, and from then on emitters of this type can be created either from code or through
        text particle scripts by naming the type.
    @par
        This same approach is used for ParticleAffectors (which modify existing particles per frame).
        This means that OGRE is particularly flexible when it comes to creating particle system effects,
        with literally infinite combinations of emitter and affector types, and parameters within those
        types.
    */
    class _OgreExport ParticleEmitter : public StringInterface, public Particle
    {
    protected:

        // Command object for setting / getting parameters
        static EmitterCommands::CmdAngle msAngleCmd;
        static EmitterCommands::CmdColour msColourCmd;
        static EmitterCommands::CmdColourRangeStart msColourRangeStartCmd;
        static EmitterCommands::CmdColourRangeEnd msColourRangeEndCmd;
        static EmitterCommands::CmdDirection msDirectionCmd;
        static EmitterCommands::CmdUp msUpCmd;
		static EmitterCommands::CmdDirPositionRef msDirPositionRefCmd;
        static EmitterCommands::CmdEmissionRate msEmissionRateCmd;
        static EmitterCommands::CmdMaxTTL msMaxTTLCmd;
        static EmitterCommands::CmdMaxVelocity msMaxVelocityCmd;
        static EmitterCommands::CmdMinTTL msMinTTLCmd;
        static EmitterCommands::CmdMinVelocity msMinVelocityCmd;
        static EmitterCommands::CmdPosition msPositionCmd;
        static EmitterCommands::CmdTTL msTTLCmd;
        static EmitterCommands::CmdVelocity msVelocityCmd;
        static EmitterCommands::CmdDuration msDurationCmd;
        static EmitterCommands::CmdMinDuration msMinDurationCmd;
        static EmitterCommands::CmdMaxDuration msMaxDurationCmd;
        static EmitterCommands::CmdRepeatDelay msRepeatDelayCmd;
        static EmitterCommands::CmdMinRepeatDelay msMinRepeatDelayCmd;
        static EmitterCommands::CmdMaxRepeatDelay msMaxRepeatDelayCmd;
		static EmitterCommands::CmdName msNameCmd;
		static EmitterCommands::CmdEmittedEmitter msEmittedEmitterCmd;


        /// Parent particle system
        ParticleSystem* mParent;
        /// Position relative to the center of the ParticleSystem
        Vector3 mPosition;
        /// Rate in particles per second at which this emitter wishes to emit particles
        Real mEmissionRate;
        /// Name of the type of emitter, MUST be initialised by subclasses
        String mType;
        /// Base direction of the emitter, may not be used by some emitters
        Vector3 mDirection;
        /// Notional up vector, used to speed up generation of variant directions, and also to orient some emitters.
        Vector3 mUp;
		/// When true, mDirPositionRef is used instead of mDirection to generate particles
		bool mUseDirPositionRef;
		/* Center position to tell in which direction will particles be emitted according to their position,
            useful for explosions & implosions, some emitters (i.e. point emitter) may not need it. */
        Vector3 mDirPositionRef;
        /// Angle around direction which particles may be emitted, internally radians but angleunits for interface
        Radian mAngle;
        /// Min speed of particles
        Real mMinSpeed;
        /// Max speed of particles
        Real mMaxSpeed;
        /// Initial time-to-live of particles (min)
        Real mMinTTL;
        /// Initial time-to-live of particles (max)
        Real mMaxTTL;
        /// Initial colour of particles (range start)
        ColourValue mColourRangeStart;
        /// Initial colour of particles (range end)
        ColourValue mColourRangeEnd;

        /// Whether this emitter is currently enabled (defaults to true)
        bool mEnabled;

        /// Start time (in seconds from start of first call to ParticleSystem to update)
        Real mStartTime;
        /// Minimum length of time emitter will run for (0 = forever)
        Real mDurationMin;
        /// Maximum length of time the emitter will run for (0 = forever)
        Real mDurationMax;
        /// Current duration remainder
        Real mDurationRemain;

        /// Time between each repeat
        Real mRepeatDelayMin;
        Real mRepeatDelayMax;
        /// Repeat delay left
        Real mRepeatDelayRemain;

		// Fractions of particles wanted to be emitted last time
		Real mRemainder;

        /// The name of the emitter. The name is optional unless it is used as an emitter that is emitted itself.
        String mName;

		/// The name of the emitter to be emitted (optional)
        String mEmittedEmitter;

		// If 'true', this emitter is emitted by another emitter.
		// NB. That doesn't imply that the emitter itself emits other emitters (that could or could not be the case)
		bool mEmitted;

		// NB Method below here are to help out people implementing emitters by providing the
        // most commonly used approaches as piecemeal methods

        /** Internal utility method for generating particle exit direction
        @param destVector Reference to vector to complete with new direction (normalised)
        */
        virtual void genEmissionDirection( const Vector3 &particlePos, Vector3& destVector );

        /** Internal utility method to apply velocity to a particle direction.
        @param destVector The vector to scale by a randomly generated scale between min and max speed.
            Assumed normalised already, and likely already oriented in the right direction.
        */
        virtual void genEmissionVelocity(Vector3& destVector);

        /** Internal utility method for generating a time-to-live for a particle. */
        virtual Real genEmissionTTL(void);

        /** Internal utility method for generating a colour for a particle. */
        virtual void genEmissionColour(ColourValue& destColour);

        /** Internal utility method for generating an emission count based on a constant emission rate. */
        virtual unsigned short genConstantEmissionCount(Real timeElapsed);

        /** Internal method for setting up the basic parameter definitions for a subclass. 
        @remarks
            Because StringInterface holds a dictionary of parameters per class, subclasses need to
            call this to ask the base class to add it's parameters to their dictionary as well.
            Can't do this in the constructor because that runs in a non-virtual context.
        @par
            The subclass must have called it's own createParamDictionary before calling this method.
        */
        void addBaseParameters(void);

        /** Internal method for initialising the duration & repeat of an emitter. */
        void initDurationRepeat(void);


    public:
        ParticleEmitter(ParticleSystem* psys);
        /** Virtual destructor essential. */
        virtual ~ParticleEmitter();

		/** Sets the position of this emitter relative to the particle system center. */
        virtual void setPosition(const Vector3& pos);

        /** Returns the position of this emitter relative to the center of the particle system. */
        virtual const Vector3& getPosition(void) const;

        /** Sets the direction of the emitter.
        @remarks
            Most emitters will have a base direction in which they emit particles (those which
            emit in all directions will ignore this parameter). They may not emit exactly along this
            vector for every particle, many will introduce a random scatter around this vector using 
            the angle property.
        @note 
			This resets the up vector.
        @param direction
            The base direction for particles emitted.
        */
        virtual void setDirection(const Vector3& direction);

        /** Returns the base direction of the emitter. */
        virtual const Vector3& getDirection(void) const;

        /** Sets the notional up vector of the emitter
        @remarks
			Many emitters emit particles from within a region, and for some that region is not
			circularly symmetric about the emitter direction. The up vector allows such emitters
			to be orientated about the direction vector.
        @param up
            The base direction for particles emitted. It must be perpendicular to the direction vector.
        */
        virtual void setUp(const Vector3& up);

        /** Returns the up vector of the emitter. */
        virtual const Vector3& getUp(void) const;

		/** Sets the direction of the emitter.
			Some particle effects need to emit particles in many random directions, but still
			following some rules; like not having them collide against each other. Very useful
			for explosions and implosions (when velocity is negative)
		@note
			Although once enabled mDirPositionRef will supersede mDirection; calling setDirection()
			may still be needed to setup a custom up vector.
		@param position
            The reference position in which the direction of the particles will be calculated from,
			also taking into account the particle's position at the time of emission.
		@param enable
            True to use mDirPositionRef, false to use the default behaviour with mDirection
		*/
		virtual void setDirPositionReference( const Vector3& position, bool enable );

		/** Returns the position reference to generate direction of emitted particles */
		virtual const Vector3& getDirPositionReference() const;

		/** Returns whether direction or position reference is used */
		virtual bool getDirPositionReferenceEnabled() const;

        /** Sets the maximum angle away from the emitter direction which particle will be emitted.
        @remarks
            Whilst the direction property defines the general direction of emission for particles, 
            this property defines how far the emission angle can deviate away from this base direction.
            This allows you to create a scatter effect - if set to 0, all particles will be emitted
            exactly along the emitters direction vector, whereas if you set it to 180 degrees or more,
            particles will be emitted in a sphere, i.e. in all directions.
        @param angle
            Maximum angle which initial particle direction can deviate from the emitter base direction vector.
        */
        virtual void setAngle(const Radian& angle);

        /** Returns the maximum angle which the initial particle direction can deviate from the emitters base direction. */
        virtual const Radian& getAngle(void) const;

        /** Sets the initial velocity of particles emitted.
        @remarks
            This method sets a constant speed for emitted particles. See the alternate version
            of this method which takes 2 parameters if you want a variable speed. 
        @param
            speed The initial speed in world units per second which every particle emitted starts with.
        */
        virtual void setParticleVelocity(Real speed);


        /** Sets the initial velocity range of particles emitted.
        @remarks
            This method sets the range of starting speeds for emitted particles. 
            See the alternate version of this method which takes 1 parameter if you want a 
            constant speed. This emitter will randomly choose a speed between the minimum and 
            maximum for each particle.
        @param max The maximum speed in world units per second for the initial particle speed on emission.
        @param min The minimum speed in world units per second for the initial particle speed on emission.
        */
        virtual void setParticleVelocity(Real min, Real max);
        /** Returns the minimum particle velocity. */
        virtual void setMinParticleVelocity(Real min);
        /** Returns the maximum particle velocity. */
        virtual void setMaxParticleVelocity(Real max);

        /** Returns the initial velocity of particles emitted. */
        virtual Real getParticleVelocity(void) const;

        /** Returns the minimum particle velocity. */
        virtual Real getMinParticleVelocity(void) const;

        /** Returns the maximum particle velocity. */
        virtual Real getMaxParticleVelocity(void) const;

        /** Sets the emission rate for this emitter.
        @remarks
            This method tells the emitter how many particles per second should be emitted. The emitter
            subclass does not have to emit these in a continuous burst - this is a relative parameter
            and the emitter may choose to emit all of the second's worth of particles every half-second
            for example. This is controlled by the emitter's getEmissionCount method.
        @par
            Also, if the ParticleSystem's particle quota is exceeded, not all the particles requested
            may be actually emitted.
        @param
            particlesPerSecond The number of particles to be emitted every second.
        */
        virtual void setEmissionRate(Real particlesPerSecond);

        /** Returns the emission rate set for this emitter. */
        virtual Real getEmissionRate(void) const;

        /** Sets the lifetime of all particles emitted.
        @remarks
            The emitter initialises particles with a time-to-live (TTL), the number of seconds a particle
            will exist before being destroyed. This method sets a constant TTL for all particles emitted.
            Note that affectors are able to modify the TTL of particles later.
        @par
            Also see the alternate version of this method which takes a min and max TTL in order to 
            have the TTL vary per particle.
        @param ttl The number of seconds each particle will live for.
        */
        virtual void setTimeToLive(Real ttl);
        /** Sets the range of lifetime for particles emitted.
        @remarks
            The emitter initialises particles with a time-to-live (TTL), the number of seconds a particle
            will exist before being destroyed. This method sets a range for the TTL for all particles emitted;
            the ttl may be randomised between these 2 extremes or will vary some other way depending on the
            emitter.
            Note that affectors are able to modify the TTL of particles later.
        @par
            Also see the alternate version of this method which takes a single TTL in order to 
            set a constant TTL for all particles.
        @param minTtl The minimum number of seconds each particle will live for.
        @param maxTtl The maximum number of seconds each particle will live for.
        */
        virtual void setTimeToLive(Real minTtl, Real maxTtl);

        /** Sets the minimum time each particle will live for. */
        virtual void setMinTimeToLive(Real min);
        /** Sets the maximum time each particle will live for. */
        virtual void setMaxTimeToLive(Real max);
        
        /** Gets the time each particle will live for. */
        virtual Real getTimeToLive(void) const;

        /** Gets the minimum time each particle will live for. */
        virtual Real getMinTimeToLive(void) const;
        /** Gets the maximum time each particle will live for. */
        virtual Real getMaxTimeToLive(void) const;

        /** Sets the initial colour of particles emitted.
        @remarks
            Particles have an initial colour on emission which the emitter sets. This method sets
            this colour. See the alternate version of this method which takes 2 colours in order to establish 
            a range of colours to be assigned to particles.
        @param colour The colour which all particles will be given on emission.
        */
        virtual void setColour(const ColourValue& colour);
        /** Sets the range of colours for emitted particles.
        @remarks
            Particles have an initial colour on emission which the emitter sets. This method sets
            the range of this colour. See the alternate version of this method which takes a single colour
            in order to set a constant colour for all particles. Emitters may choose to randomly assign
            a colour in this range, or may use some other method to vary the colour.
        @param colourStart The start of the colour range
        @param colourEnd The end of the colour range
        */
        virtual void setColour(const ColourValue& colourStart, const ColourValue& colourEnd);
        /** Sets the minimum colour of particles to be emitted. */
        virtual void setColourRangeStart(const ColourValue& colour);
        /** Sets the maximum colour of particles to be emitted. */
        virtual void setColourRangeEnd(const ColourValue& colour);
        /** Gets the colour of particles to be emitted. */
        virtual const ColourValue& getColour(void) const;
        /** Gets the minimum colour of particles to be emitted. */
        virtual const ColourValue& getColourRangeStart(void) const;
        /** Gets the maximum colour of particles to be emitted. */
        virtual const ColourValue& getColourRangeEnd(void) const;

        /** Gets the number of particles which this emitter would like to emit based on the time elapsed.
        @remarks
            For efficiency the emitter does not actually create new Particle instances (these are reused
            by the ParticleSystem as existing particles 'die'). The implementation for this method must
            return the number of particles the emitter would like to emit given the number of seconds which
            have elapsed (passed in as a parameter).
        @par
            Based on the return value from this method, the ParticleSystem class will call 
            _initParticle once for each particle it chooses to allow to be emitted by this emitter.
            The emitter should not track these _initParticle calls, it should assume all emissions
            requested were made (even if they could not be because of particle quotas).
        */
        virtual unsigned short _getEmissionCount(Real timeElapsed) = 0;

        /** Initialises a particle based on the emitter's approach and parameters.
        @remarks
            See the _getEmissionCount method for details of why there is a separation between
            'requested' emissions and actual initialised particles.
        @param
            pParticle Pointer to a particle which must be initialised based on how this emitter
            starts particles. This is passed as a pointer rather than being created by the emitter so the
            ParticleSystem can reuse Particle instances, and can also set defaults itself.
        */
        virtual void _initParticle(Particle* pParticle) {
            // Initialise size in case it's been altered
            pParticle->resetDimensions();
        }


        /** Returns the name of the type of emitter. 
        @remarks
            This property is useful for determining the type of emitter procedurally so another
            can be created.
        */
        const String &getType(void) const { return mType; }

        /** Sets whether or not the emitter is enabled.
        @remarks
            You can turn an emitter off completely by setting this parameter to false.
        */
        virtual void setEnabled(bool enabled);

        /** Gets the flag indicating if this emitter is enabled or not. */
        virtual bool getEnabled(void) const;

        /** Sets the 'start time' of this emitter.
        @remarks
            By default an emitter starts straight away as soon as a ParticleSystem is first created,
            or also just after it is re-enabled. This parameter allows you to set a time delay so
            that the emitter does not 'kick in' until later.
        @param startTime The time in seconds from the creation or enabling of the emitter.
        */
        virtual void setStartTime(Real startTime);
        /** Gets the start time of the emitter. */
        virtual Real getStartTime(void) const;

        /** Sets the duration of the emitter.
        @remarks
            By default emitters run indefinitely (unless you manually disable them). By setting this
            parameter, you can make an emitter turn off on it's own after a set number of seconds. It
            will then remain disabled until either setEnabled(true) is called, or if the 'repeatAfter' parameter
            has been set it will also repeat after a number of seconds.
        @par
            Also see the alternative version of this method which allows you to set a min and max duration for
            a random variable duration.
        @param duration The duration in seconds.
        */
        virtual void setDuration(Real duration);

        /** Gets the duration of the emitter from when it is created or re-enabled. */
        virtual Real getDuration(void) const;

        /** Sets the range of random duration for this emitter. 
        @remarks
            By default emitters run indefinitely (unless you manually disable them). By setting this
            parameter, you can make an emitter turn off on it's own after a random number of seconds. It
            will then remain disabled until either setEnabled(true) is called, or if the 'repeatAfter' parameter
            has been set it will also repeat after a number of seconds.
        @par
            Also see the alternative version of this method which allows you to set a constant duration.
        @param min The minimum duration in seconds.
        @param max The minimum duration in seconds.
        */
        virtual void setDuration(Real min, Real max);
        /** Sets the minimum duration of this emitter in seconds (see setDuration for more details) */
        virtual void setMinDuration(Real min);
        /** Sets the maximum duration of this emitter in seconds (see setDuration for more details) */
        virtual void setMaxDuration(Real max);
        /** Gets the minimum duration of this emitter in seconds (see setDuration for more details) */
        virtual Real getMinDuration(void) const;
        /** Gets the maximum duration of this emitter in seconds (see setDuration for more details) */
        virtual Real getMaxDuration(void) const;

        /** Sets the time between repeats of the emitter.
        @remarks
            By default emitters run indefinitely (unless you manually disable them). However, if you manually
            disable the emitter (by calling setEnabled(false), or it's duration runs out, it will cease to emit
        @par
            Also see the alternative version of this method which allows you to set a min and max duration for
            a random variable duration.
        @param duration The duration in seconds.
        */
        virtual void setRepeatDelay(Real duration);

        /** Gets the duration of the emitter from when it is created or re-enabled. */
        virtual Real getRepeatDelay(void) const;

        /** Sets the range of random duration for this emitter. 
        @remarks
            By default emitters run indefinitely (unless you manually disable them). By setting this
            parameter, you can make an emitter turn off on it's own after a random number of seconds. It
            will then remain disabled until either setEnabled(true) is called, or if the 'repeatAfter' parameter
            has been set it will also repeat after a number of seconds.
        @par
            Also see the alternative version of this method which allows you to set a constant duration.
        @param min The minimum duration in seconds.
        @param max The minimum duration in seconds.
        */
        virtual void setRepeatDelay(Real min, Real max);
        /** Sets the minimum duration of this emitter in seconds (see setRepeatDelay for more details) */
        virtual void setMinRepeatDelay(Real min);
        /** Sets the maximum duration of this emitter in seconds (see setRepeatDelay for more details) */
        virtual void setMaxRepeatDelay(Real max);
        /** Gets the minimum duration of this emitter in seconds (see setRepeatDelay for more details) */
        virtual Real getMinRepeatDelay(void) const;
        /** Gets the maximum duration of this emitter in seconds (see setRepeatDelay for more details) */
        virtual Real getMaxRepeatDelay(void) const;

		/** Returns the name of the emitter */
		const String &getName(void) const;

		/** Sets the name of the emitter */
		virtual void setName(const String& newName);

		/** Returns the name of the emitter to be emitted */
		const String &getEmittedEmitter(void) const;

		/** Sets the name of the emitter to be emitted*/
		virtual void setEmittedEmitter(const String& emittedEmitter);

		/** Return true if the emitter is emitted by another emitter */
		virtual bool isEmitted(void) const;

		/** Set the indication (true/false) to indicate that the emitter is emitted by another emitter */
		virtual void setEmitted(bool emitted);


    };
	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif

