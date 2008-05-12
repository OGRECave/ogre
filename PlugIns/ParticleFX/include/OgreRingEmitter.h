/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright ) 2002 Tels <http://bloodgate.com> based on BoxEmitter
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
#ifndef __RingEmitter_H__
#define __RingEmitter_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreAreaEmitter.h"
#include "OgreMath.h"

namespace Ogre {

    /** Particle emitter which emits particles randomly from points inside a ring (e.g. a tube).
    @remarks
        This particle emitter emits particles from a ring-shaped area.
        The initial direction of these particles can either be a single
        direction (i.e. a line), a random scattering inside a cone, or a random
        scattering in all directions, depending the 'angle' parameter, which
        is the angle across which to scatter the particles either side of the
        base direction of the emitter. 
    */
    class _OgreParticleFXExport RingEmitter : public AreaEmitter
    {
    public:
        // See AreaEmitter
        /** Command object for inner size (see ParamCommand).*/
        class CmdInnerX : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for inner size (see ParamCommand).*/
        class CmdInnerY : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        RingEmitter(ParticleSystem* psys);

        /** See ParticleEmitter. */
        void _initParticle(Particle* pParticle);

        /** Sets the size of the clear space inside the area from where NO particles are emitted.
        @param x,y,z
            Parametric values describing the proportion of the shape which is hollow in each direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSize(Real x, Real y);

        /** Sets the x component of the area inside the ellipsoid which doesn't emit particles. 
        @param x
            Parametric value describing the proportion of the shape which is hollow in this direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSizeX(Real x);
        /** Sets the y component of the area inside the ellipsoid which doesn't emit particles. 
        @param y
            Parametric value describing the proportion of the shape which is hollow in this direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSizeY(Real y);
        /** Gets the x component of the area inside the ellipsoid which doesn't emit particles. */
        Real getInnerSizeX(void) const;
        /** Gets the y component of the area inside the ellipsoid which doesn't emit particles. */
        Real getInnerSizeY(void) const;

    protected:
        // See ParticleEmitter
        static CmdInnerX msCmdInnerX;
        static CmdInnerY msCmdInnerY;

        /// Size of 'clear' center area (> 0 and < 1.0)
        Real mInnerSizex;
        Real mInnerSizey;



    };

}

#endif

