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
#ifndef __DeflectorPlaneAffector_H__
#define __DeflectorPlaneAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreVector3.h"


namespace Ogre {

    /** This class defines a ParticleAffector which deflects particles.
    @remarks
        This affector (see ParticleAffector) offers a simple (and inaccurate) physical deflection.
        All particles which hit the plane are reflected.
    @par
        The plane is defined by a point (plane_point) and the normal (plane_normal).
        In addition it is possible to change the strenght of the recoil by using the bounce parameter.
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

}

#endif
