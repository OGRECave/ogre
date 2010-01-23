/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
// Original author: Tels <http://bloodgate.com>, released as public domain
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


