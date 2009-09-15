/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifndef __LinearForceAffector_H__
#define __LinearForceAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreVector3.h"


namespace Ogre {

    /** This class defines a ParticleAffector which applies a linear force to particles in a system.
    @remarks
        This affector (see ParticleAffector) applies a linear force, such as gravity, to a particle system.
        This force can be applied in 2 ways: by taking the average of the particle's current momentum and the 
        force vector, or by adding the force vector to the current particle's momentum. 
    @par
        The former approach is self-stabilising i.e. once a particle's momentum
        is equal to the force vector, no further change is made to it's momentum. It also results in
        a non-linear acceleration of particles.
        The latter approach is simpler and applies a constant acceleration to particles. However,
        it is not self-stabilising and can lead to perpetually increasing particle velocities. 
        You choose the approach by calling the setForceApplication method.
    */
    class _OgreParticleFXExport LinearForceAffector : public ParticleAffector
    {
    public:
        /** Command object for force vector (see ParamCommand).*/
        class CmdForceVector : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for force application (see ParamCommand).*/
        class CmdForceApp : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Choice of how to apply the force vector to particles
        enum ForceApplication
        {
            /// Take the average of the force vector and the particle momentum
            FA_AVERAGE,
            /// Add the force vector to the particle momentum
            FA_ADD
        };
        /// Default constructor
        LinearForceAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);


        /** Sets the force vector to apply to the particles in a system. */
        void setForceVector(const Vector3& force);

        /** Gets the force vector to apply to the particles in a system. */
        Vector3 getForceVector(void) const;

        /** Sets how the force vector is applied to a particle. 
        @remarks
          The default is FA_ADD.
        @param fa A member of the ForceApplication enum.
        */
        void setForceApplication(ForceApplication fa);

        /** Retrieves how the force vector is applied to a particle. 
        @param fa A member of the ForceApplication enum.
        */
        ForceApplication getForceApplication(void) const;

        /// Command objects
        static CmdForceVector msForceVectorCmd;
        static CmdForceApp msForceAppCmd;

    protected:
        /// Force vector
        Vector3 mForceVector;

        /// How to apply force
        ForceApplication mForceApplication;

    };


}


#endif

