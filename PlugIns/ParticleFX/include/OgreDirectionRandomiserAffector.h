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
#ifndef __DirectionRandomiserAffector_H__
#define __DirectionRandomiserAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreVector3.h"


namespace Ogre {

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
