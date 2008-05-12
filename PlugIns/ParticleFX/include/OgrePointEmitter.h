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
#ifndef __PointEmitter_H__
#define __PointEmitter_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleEmitter.h"

namespace Ogre {

    /** Particle emitter which emits particles from a single point.
    @remarks
        This basic particle emitter emits particles from a single point in space. The
        initial direction of these particles can either be a single direction (i.e. a line),
        a random scattering inside a cone, or a random scattering in all directions, 
        depending the 'angle' parameter, which is the angle across which to scatter the 
        particles either side of the base direction of the emitter. 
    */
    class _OgreParticleFXExport PointEmitter : public ParticleEmitter
    {
    public:
        PointEmitter(ParticleSystem* psys);

        /** See ParticleEmitter. */
        void _initParticle(Particle* pParticle);

        /** See ParticleEmitter. */
        unsigned short _getEmissionCount(Real timeElapsed);




    };

}

#endif
