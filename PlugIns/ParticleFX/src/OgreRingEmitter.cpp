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
#include "OgreRingEmitter.h"
#include "OgreParticle.h"
#include "OgreException.h"
#include "OgreStringConverter.h"


/* Implements an Emitter whose emitting points all lie inside a ring.
*/

namespace Ogre {

    RingEmitter::CmdInnerX RingEmitter::msCmdInnerX;
    RingEmitter::CmdInnerY RingEmitter::msCmdInnerY;

    //-----------------------------------------------------------------------
    RingEmitter::RingEmitter(ParticleSystem* psys)
        : AreaEmitter(psys)
    {
        if (initDefaults("Ring"))
        {
            // Add custom parameters
            ParamDictionary* pDict = getParamDictionary();

            pDict->addParameter(ParameterDef("inner_width", "Parametric value describing the proportion of the "
                "shape which is hollow.", PT_REAL), &msCmdInnerX);
            pDict->addParameter(ParameterDef("inner_height", "Parametric value describing the proportion of the "
                "shape which is hollow.", PT_REAL), &msCmdInnerY);
        }
        // default is half empty
        setInnerSize(0.5,0.5);
    }
    //-----------------------------------------------------------------------
    void RingEmitter::_initParticle(Particle* pParticle)
    {
        Real a, b, x, y, z;

        // Call superclass
        AreaEmitter::_initParticle(pParticle);
        // create a random angle from 0 .. PI*2
        Radian alpha ( Math::RangeRandom(0,Math::TWO_PI) );
  
        // create two random radius values that are bigger than the inner size
        a = Math::RangeRandom(mInnerSizex,1.0);
        b = Math::RangeRandom(mInnerSizey,1.0);

        // with a and b we have defined a random ellipse inside the inner
        // ellipse and the outer circle (radius 1.0)
        // with alpha, and a and b we select a random point on this ellipse
        // and calculate it's coordinates
        x = a * Math::Sin(alpha);
        y = b * Math::Cos(alpha);
        // the height is simple -1 to 1
        z = Math::SymmetricRandom();     

        // scale the found point to the ring's size and move it
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
    //-----------------------------------------------------------------------
    void RingEmitter::setInnerSize(Real x, Real y)
    {
        // TODO: should really throw some exception
        if ((x > 0) && (x < 1.0) &&
            (y > 0) && (y < 1.0))
            {
            mInnerSizex = x;
            mInnerSizey = y;
            }
    }
    //-----------------------------------------------------------------------
    void RingEmitter::setInnerSizeX(Real x)
    {
        assert(x > 0 && x < 1.0);

        mInnerSizex = x;
    }
    //-----------------------------------------------------------------------
    void RingEmitter::setInnerSizeY(Real y)
    {
        assert(y > 0 && y < 1.0);

        mInnerSizey = y;
    }
    //-----------------------------------------------------------------------
    Real RingEmitter::getInnerSizeX(void) const
    {
        return mInnerSizex;
    }
    //-----------------------------------------------------------------------
    Real RingEmitter::getInnerSizeY(void) const
    {
        return mInnerSizey;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String RingEmitter::CmdInnerX::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RingEmitter*>(target)->getInnerSizeX() );
    }
    void RingEmitter::CmdInnerX::doSet(void* target, const String& val)
    {
        static_cast<RingEmitter*>(target)->setInnerSizeX(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String RingEmitter::CmdInnerY::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RingEmitter*>(target)->getInnerSizeY() );
    }
    void RingEmitter::CmdInnerY::doSet(void* target, const String& val)
    {
        static_cast<RingEmitter*>(target)->setInnerSizeY(StringConverter::parseReal(val));
    }

}


