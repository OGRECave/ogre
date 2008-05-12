/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright ) 2002 Tels <http://bloodgate.com> based on EllipsoidEmitter
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
#include "OgreCylinderEmitter.h"
#include "OgreParticle.h"
#include "OgreQuaternion.h"
#include "OgreException.h"
#include "OgreStringConverter.h"


/* Implements an Emitter whose emitting points all lie inside a cylinder.
*/

namespace Ogre {


    //-----------------------------------------------------------------------
    CylinderEmitter::CylinderEmitter(ParticleSystem* psys)
        : AreaEmitter(psys)
    {
        initDefaults("Cylinder");
    }
    //-----------------------------------------------------------------------
    void CylinderEmitter::_initParticle(Particle* pParticle)
    {
        Real x, y, z;

        // Call superclass
        AreaEmitter::_initParticle(pParticle);

        // First we create a random point inside a bounding cylinder with a
        // radius and height of 1 (this is easy to do). The distance of the
        // point from 0,0,0 must be <= 1 (== 1 means on the surface and we
        // count this as inside, too).

        while (true)
        {
/* ClearSpace not yet implemeted

*/
                // three random values for one random point in 3D space
                x = Math::SymmetricRandom();
                y = Math::SymmetricRandom();
                z = Math::SymmetricRandom();

                // the distance of x,y from 0,0 is sqrt(x*x+y*y), but
                // as usual we can omit the sqrt(), since sqrt(1) == 1 and we
                // use the 1 as boundary. z is not taken into account, since
                // all values in the z-direction are inside the cylinder:
                if ( x*x + y*y <= 1)
                {
                        break;          // found one valid point inside
                }
        }       

        // scale the found point to the cylinder's size and move it
        // relatively to the center of the emitter point

        pParticle->position = mPosition + 
         + x * mXRange + y * mYRange + z * mZRange;

        // Generate complex data by reference
        genEmissionColour(pParticle->colour);
        genEmissionDirection(pParticle->direction);
        genEmissionVelocity(pParticle->direction);

        // Generate simpler data
        pParticle->timeToLive = pParticle->totalTimeToLive = genEmissionTTL();
        
    }

}


