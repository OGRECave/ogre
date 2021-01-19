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
#ifndef __DeflectorPlaneAffector_H__
#define __DeflectorPlaneAffector_H__

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
    /** This affector defines a plane which deflects particles which collide with it.
    */
    class _OgreParticleFXExport DeflectorPlaneAffector : public ParticleAffector
    {
    public:
        /** Command object for plane point (see ParamCommand).*/
        class CmdPlanePoint : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for plane normal (see ParamCommand).*/
        class CmdPlaneNormal : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for bounce (see ParamCommand).*/
        class CmdBounce : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Default constructor
        DeflectorPlaneAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

        /** Sets the plane point of the deflector plane. */
        void setPlanePoint(const Vector3& pos);

        /** Gets the plane point of the deflector plane. */
        Vector3 getPlanePoint(void) const;

        /** Sets the plane normal of the deflector plane. */
        void setPlaneNormal(const Vector3& normal);

        /** Gets the plane normal of the deflector plane. */
        Vector3 getPlaneNormal(void) const;

        /** Sets the bounce value of the deflection. */
        void setBounce(Real bounce);

        /** Gets the bounce value of the deflection. */
        Real getBounce(void) const;

        /// Command objects
        static CmdPlanePoint msPlanePointCmd;
        static CmdPlaneNormal msPlaneNormalCmd;
        static CmdBounce msBounceCmd;

    protected:
        /// deflector plane point
        Vector3 mPlanePoint;
        /// deflector plane normal vector
        Vector3 mPlaneNormal;

        /// bounce factor (0.5 means 50 percent)
        Real mBounce;
    };
    /** @} */
    /** @} */
}

#endif
