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
// Original author: Tels <http://bloodgate.com>, released as public domain
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

        pParticle->mPosition = mPosition +
         + x * mXRange + y * mYRange + z * mZRange;

        // Generate complex data by reference
        genEmissionColour(pParticle->mColour);
        genEmissionDirection( pParticle->mPosition, pParticle->mDirection );
        genEmissionVelocity(pParticle->mDirection);

        // Generate simpler data
        pParticle->mTimeToLive = pParticle->mTotalTimeToLive = genEmissionTTL();
        
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


