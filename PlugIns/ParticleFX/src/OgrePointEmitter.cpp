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
#include "OgrePointEmitter2.h"
#include "OgreParticle2.h"



namespace Ogre {

    //-----------------------------------------------------------------------
    PointEmitter2::PointEmitter2(ParticleSystem2* psys)
        :ParticleEmitter2(psys)
    {
        mType = "Point";
        // Set up parameters
        if (createParamDictionary("PointEmitter2"))
        {
            addBaseParameters();
        }
        // No custom parameters
    }
    //-----------------------------------------------------------------------
    void PointEmitter2::_initParticle(Particle2* pParticle)
    {
        // Very simple emitter, uses default implementations with no modification

        // Call superclass
        ParticleEmitter2::_initParticle(pParticle);

        // Point emitter emits from own position
        pParticle->mPosition = mPosition;

        // Generate complex data by reference
        genEmissionColour(pParticle->mColour);
        genEmissionDirection( pParticle->mPosition, pParticle->mDirection );
        genEmissionVelocity(pParticle->mDirection);

        // Generate simpler data
        pParticle->mTimeToLive = pParticle->mTotalTimeToLive = genEmissionTTL();
        
    }

    void PointEmitter2::_initParticles (Particles2& particles, unsigned start, unsigned end)
    {
        genEmissionTTLs (particles, start, end);
        genEmissionTranslations (particles, start, end);
        auto colors = particles.colors;
        for (auto c = start; c < end; ++c)
            genEmissionColour (colors[c]);
    }

    //-----------------------------------------------------------------------
    unsigned short PointEmitter2::_getEmissionCount(Real timeElapsed)
    {
        // Use basic constant emission 
        return genConstantEmissionCount(timeElapsed);
    }

    ParticleEmitter2::Ptr PointEmitter2::clone ()
    {
        return ParticleEmitter2::Ptr{ new PointEmitter2 (*this) };
    }

    ParticleEmitter2::Ptr PointEmitter2::create (ParticleSystem2* s)
    {
        return ParticleEmitter2::Ptr{ new PointEmitter2 (s) };
    }

}


