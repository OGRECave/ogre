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
#ifndef __DirectionRandomiserAffector_H__
#define __DirectionRandomiserAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreVector3.h"


namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup ParticleFX
    *  @{
    */
    /** This class defines a ParticleAffector which applies randomness to the movement of the particles.
    @remarks
        This affector (see ParticleAffector) applies randomness to the movement of the particles by
        changing the direction vectors.
    @par
        The most important parameter to control the effect is randomness. It controls the range in which changes
        are applied to each axis of the direction vector.
        The parameter scope can be used to limit the effect to a certain percentage of the particles.
    */
    class _OgreParticleFXExport DirectionRandomiserAffector : public ParticleAffector
    {
    public:
        /** Command object for randomness (see ParamCommand).*/
        class CmdRandomness : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for scope (see ParamCommand).*/
        class CmdScope : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for keep_velocity (see ParamCommand).*/
        class CmdKeepVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Default constructor
        DirectionRandomiserAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);


        /** Sets the randomness to apply to the particles in a system. */
        void setRandomness(Real force);
        /** Sets the scope (percentage of particles which are randomised). */
        void setScope(Real force);
        /** Set flag which detemines whether particle speed is changed. */
        void setKeepVelocity(bool keepVelocity);

        /** Gets the randomness to apply to the particles in a system. */
        Real getRandomness(void) const;
        /** Gets the scope (percentage of particles which are randomised). */
        Real getScope(void) const;
        /** Gets flag which detemines whether particle speed is changed. */
        bool getKeepVelocity(void) const;

        /// Command objects
        static CmdRandomness msRandomnessCmd;
        static CmdScope msScopeCmd;
        static CmdKeepVelocity msKeepVelocityCmd;

    protected:
        Real mRandomness;
        Real mScope;
        bool mKeepVelocity;

    };

}

#endif
