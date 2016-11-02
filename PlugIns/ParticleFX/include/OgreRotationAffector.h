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
#ifndef __RotationAffector_H__
#define __RotationAffector_H__

#include "OgreMath.h"
#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"

namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup ParticleFX
    *  @{
    */

    /** This plugin subclass of ParticleAffector allows you to alter the rotation of particles.
    @remarks
        This class supplies the ParticleAffector implementation required to make the particle expand
        or contract in mid-flight.
    */
    class _OgreParticleFXExport RotationAffector : public ParticleAffector
    {
    public:
        /// Command object for particle emitter  - see ParamCommand 
        class CmdRotationSpeedRangeStart : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdRotationSpeedRangeEnd : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdRotationRangeStart : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdRotationRangeEnd : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Default constructor. */
        RotationAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _initParticle(Particle* pParticle);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);



        /** Sets the minimum rotation speed of particles to be emitted. */
        void setRotationSpeedRangeStart(const Radian& angle);
        /** Sets the maximum rotation speed of particles to be emitted. */
        void setRotationSpeedRangeEnd(const Radian& angle);
        /** Gets the minimum rotation speed of particles to be emitted. */
        const Radian& getRotationSpeedRangeStart(void) const;
        /** Gets the maximum rotation speed of particles to be emitted. */
        const Radian& getRotationSpeedRangeEnd(void) const;

        
        /** Sets the minimum rotation angle of particles to be emitted. */
        void setRotationRangeStart(const Radian& angle);
        /** Sets the maximum rotation angle of particles to be emitted. */
        void setRotationRangeEnd(const Radian& angle);
        /** Gets the minimum rotation of particles to be emitted. */
        const Radian& getRotationRangeStart(void) const;
        /** Gets the maximum rotation of particles to be emitted. */
        const Radian& getRotationRangeEnd(void) const;

        static CmdRotationSpeedRangeStart   msRotationSpeedRangeStartCmd;
        static CmdRotationSpeedRangeEnd     msRotationSpeedRangeEndCmd;
        static CmdRotationRangeStart        msRotationRangeStartCmd;
        static CmdRotationRangeEnd          msRotationRangeEndCmd;
        
    protected:
        /// Initial rotation speed of particles (range start)
        Radian mRotationSpeedRangeStart;
        /// Initial rotation speed of particles (range end)
        Radian mRotationSpeedRangeEnd;
        /// Initial rotation angle of particles (range start)
        Radian mRotationRangeStart;
        /// Initial rotation angle of particles (range end)
        Radian mRotationRangeEnd;

    };

    /** @} */
    /** @} */
}


#endif

