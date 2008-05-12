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
#include "OgreHollowEllipsoidEmitter.h"
#include "OgreParticle.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreMath.h"

/* Implements an Emitter whose emitting points all lie inside an ellipsoid.
   See <http://mathworld.wolfram.com/Ellipsoid.html> for mathematical details.

  If the lengths of two axes of an ellipsoid are the same, the figure is
  called a 'spheroid' (depending on whether c < a or c > a, an 'oblate
  spheroid' or 'prolate spheroid', respectively), and if all three are the
  same, it is a 'sphere' (ball).
*/

namespace Ogre {

    HollowEllipsoidEmitter::CmdInnerX HollowEllipsoidEmitter::msCmdInnerX;
    HollowEllipsoidEmitter::CmdInnerY HollowEllipsoidEmitter::msCmdInnerY;
    HollowEllipsoidEmitter::CmdInnerZ HollowEllipsoidEmitter::msCmdInnerZ;


    //-----------------------------------------------------------------------
    HollowEllipsoidEmitter::HollowEllipsoidEmitter(ParticleSystem* psys)
        : EllipsoidEmitter(psys)
    {
        if (initDefaults("HollowEllipsoid"))
        {
            // Add custom parameters
            ParamDictionary* pDict = getParamDictionary();

            pDict->addParameter(ParameterDef("inner_width", "Parametric value describing the proportion of the "
                "shape which is hollow.", PT_REAL), &msCmdInnerX);
            pDict->addParameter(ParameterDef("inner_height", "Parametric value describing the proportion of the "
                "shape which is hollow.", PT_REAL), &msCmdInnerY);
            pDict->addParameter(ParameterDef("inner_depth", "Parametric value describing the proportion of the "
                "shape which is hollow.", PT_REAL), &msCmdInnerZ);
        }
        // default is half empty
        setInnerSize(0.5,0.5,0.5);
    }
    //-----------------------------------------------------------------------
    void HollowEllipsoidEmitter::_initParticle(Particle* pParticle)
    {
        Real a, b, c, x, y, z;

        // Init dimensions
        pParticle->resetDimensions();

        // create two random angles alpha and beta
        // with these two angles, we are able to select any point on an
        // ellipsoid's surface
        Radian alpha ( Math::RangeRandom(0,Math::TWO_PI) );
        Radian beta  ( Math::RangeRandom(0,Math::PI) );

        // create three random radius values that are bigger than the inner
        // size, but smaller/equal than/to the outer size 1.0 (inner size is
        // between 0 and 1)
        a = Math::RangeRandom(mInnerSize.x,1.0);
        b = Math::RangeRandom(mInnerSize.y,1.0);
        c = Math::RangeRandom(mInnerSize.z,1.0);

        // with a,b,c we have defined a random ellipsoid between the inner
        // ellipsoid and the outer sphere (radius 1.0)
        // with alpha and beta we select on point on this random ellipsoid
        // and calculate the 3D coordinates of this point
		Real sinbeta ( Math::Sin(beta) );
        x = a * Math::Cos(alpha) * sinbeta;
        y = b * Math::Sin(alpha) * sinbeta;
        z = c * Math::Cos(beta);

        // scale the found point to the ellipsoid's size and move it
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
    void HollowEllipsoidEmitter::setInnerSize(Real x, Real y, Real z)
    {
        assert((x > 0) && (x < 1.0) &&
            (y > 0) && (y < 1.0) &&
            (z > 0) && (z < 1.0));

        mInnerSize.x = x;
        mInnerSize.y = y;
        mInnerSize.z = z;
    }
    //-----------------------------------------------------------------------
    void HollowEllipsoidEmitter::setInnerSizeX(Real x)
    {
        assert(x > 0 && x < 1.0);

        mInnerSize.x = x;
    }
    //-----------------------------------------------------------------------
    void HollowEllipsoidEmitter::setInnerSizeY(Real y)
    {
        assert(y > 0 && y < 1.0);

        mInnerSize.y = y;
    }
    //-----------------------------------------------------------------------
    void HollowEllipsoidEmitter::setInnerSizeZ(Real z)
    {
        assert(z > 0 && z < 1.0);

        mInnerSize.z = z;
    }
    //-----------------------------------------------------------------------
    Real HollowEllipsoidEmitter::getInnerSizeX(void) const
    {
        return mInnerSize.x;
    }
    //-----------------------------------------------------------------------
    Real HollowEllipsoidEmitter::getInnerSizeY(void) const
    {
        return mInnerSize.y;
    }
    //-----------------------------------------------------------------------
    Real HollowEllipsoidEmitter::getInnerSizeZ(void) const
    {
        return mInnerSize.z;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String HollowEllipsoidEmitter::CmdInnerX::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const HollowEllipsoidEmitter*>(target)->getInnerSizeX() );
    }
    void HollowEllipsoidEmitter::CmdInnerX::doSet(void* target, const String& val)
    {
        static_cast<HollowEllipsoidEmitter*>(target)->setInnerSizeX(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String HollowEllipsoidEmitter::CmdInnerY::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const HollowEllipsoidEmitter*>(target)->getInnerSizeY() );
    }
    void HollowEllipsoidEmitter::CmdInnerY::doSet(void* target, const String& val)
    {
        static_cast<HollowEllipsoidEmitter*>(target)->setInnerSizeY(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String HollowEllipsoidEmitter::CmdInnerZ::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const HollowEllipsoidEmitter*>(target)->getInnerSizeZ() );
    }
    void HollowEllipsoidEmitter::CmdInnerZ::doSet(void* target, const String& val)
    {
        static_cast<HollowEllipsoidEmitter*>(target)->setInnerSizeZ(StringConverter::parseReal(val));
    }


}


