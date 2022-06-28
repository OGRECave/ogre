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
#include "OgreDirectionRandomiserAffector.h"
#include "OgreParticleSystem.h"
#include "OgreParticle.h"
#include "OgreStringConverter.h"


namespace Ogre {

    // Instantiate statics
    DirectionRandomiserAffector::CmdRandomness DirectionRandomiserAffector::msRandomnessCmd;
    DirectionRandomiserAffector::CmdScope DirectionRandomiserAffector::msScopeCmd;
    DirectionRandomiserAffector::CmdKeepVelocity DirectionRandomiserAffector::msKeepVelocityCmd;

    //-----------------------------------------------------------------------
    DirectionRandomiserAffector::DirectionRandomiserAffector(ParticleSystem* psys)
       : ParticleAffector(psys)
    {
        mType = "DirectionRandomiser";

        // defaults
        mRandomness = 1.0;
        mScope = 1.0;
        mKeepVelocity = false;

        // Set up parameters
        if (createParamDictionary("DirectionRandomiserAffector"))
        {
            addBaseParameters();
            // Add extra parameters
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("randomness",
                "The amount of randomness (chaos) to apply to the particle movement.",
                PT_REAL), &msRandomnessCmd);
            dict->addParameter(ParameterDef("scope",
                "The percentage of particles which is affected.",
                PT_REAL), &msScopeCmd);
            dict->addParameter(ParameterDef("keep_velocity",
                "Determines whether the velocity of the particles is changed.",
                PT_BOOL), &msKeepVelocityCmd);
        }
    }
    //-----------------------------------------------------------------------
    void DirectionRandomiserAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        Real length = 0;

        for (auto p : pSystem->_getActiveParticles())
        {
            if (mScope > Math::UnitRandom())
            {
                if (!p->mDirection.isZeroLength())
                {
                    if (mKeepVelocity)
                    {
                        length = p->mDirection.length();
                    }

                    p->mDirection += Vector3(Math::RangeRandom(-mRandomness, mRandomness) * timeElapsed,
                        Math::RangeRandom(-mRandomness, mRandomness) * timeElapsed,
                        Math::RangeRandom(-mRandomness, mRandomness) * timeElapsed);

                    if (mKeepVelocity)
                    {
                        p->mDirection *= length / p->mDirection.length();
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void DirectionRandomiserAffector::setRandomness(Real force)
    {
        mRandomness = force;
    }
    //-----------------------------------------------------------------------
    void DirectionRandomiserAffector::setScope(Real scope)
    {
        mScope = scope;
    }
    //-----------------------------------------------------------------------
    void DirectionRandomiserAffector::setKeepVelocity(bool keepVelocity)
    {
        mKeepVelocity = keepVelocity;
    }
    //-----------------------------------------------------------------------
    Real DirectionRandomiserAffector::getRandomness(void) const
    {
        return mRandomness;
    }
    //-----------------------------------------------------------------------
    Real DirectionRandomiserAffector::getScope(void) const
    {
        return mScope;
    }
    //-----------------------------------------------------------------------
    bool DirectionRandomiserAffector::getKeepVelocity(void) const
    {
        return mKeepVelocity;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String DirectionRandomiserAffector::CmdRandomness::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DirectionRandomiserAffector*>(target)->getRandomness() );
    }
    void DirectionRandomiserAffector::CmdRandomness::doSet(void* target, const String& val)
    {
        static_cast<DirectionRandomiserAffector*>(target)->setRandomness(StringConverter::parseReal(val));
    }

    String DirectionRandomiserAffector::CmdScope::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DirectionRandomiserAffector*>(target)->getScope() );
    }
    void DirectionRandomiserAffector::CmdScope::doSet(void* target, const String& val)
    {
        static_cast<DirectionRandomiserAffector*>(target)->setScope(StringConverter::parseReal(val));
    }
    String DirectionRandomiserAffector::CmdKeepVelocity::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DirectionRandomiserAffector*>(target)->getKeepVelocity() );
    }
    void DirectionRandomiserAffector::CmdKeepVelocity::doSet(void* target, const String& val)
    {
        static_cast<DirectionRandomiserAffector*>(target)->setKeepVelocity(StringConverter::parseBool(val));
    }

}
