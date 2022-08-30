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
#ifndef __LinearForceAffector_H__
#define __LinearForceAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreVector.h"


namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup ParticleFX
    *  @{
    */
    /** This affector applies a force vector to all particles to modify their trajectory. Can be used for gravity, wind, or any other linear force.

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
    class LinearForceAffector : public ParticleAffector
    {
    public:
        /** Command object for force vector (see ParamCommand).*/
        class CmdForceVector : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /** Command object for force application (see ParamCommand).*/
        class CmdForceApp : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
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

        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed) override;


        /** Sets the force vector to apply to the particles in a system. */
        void setForceVector(const Vector3& force);

        /** Gets the force vector to apply to the particles in a system. */
        Vector3 getForceVector(void) const;

        /** Sets how the force vector is applied to a particle. 

          The default is FA_ADD.
        @param fa A member of the ForceApplication enum.
        */
        void setForceApplication(ForceApplication fa);

        /** Retrieves how the force vector is applied to a particle. 
        @return A member of the ForceApplication enum.
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

    /** @} */
    /** @} */
}


#endif

