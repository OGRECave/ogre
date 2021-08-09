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
#include "OgreScaleAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
    ScaleAffector::CmdScaleAdjust ScaleAffector::msScaleCmd;

    static SimpleParamCommand<ScaleAffector, const Vector2&, &ScaleAffector::getScaleRange,
                              &ScaleAffector::setScaleRange>
        msScaleRangeCmd;

    //-----------------------------------------------------------------------
    ScaleAffector::ScaleAffector(ParticleSystem* psys)
        :ParticleAffector(psys), mScaleRange(Vector2::UNIT_SCALE)
    {
        mScaleAdj = 0;
        mType = "Scaler";

        // Init parameters
        if (createParamDictionary("ScaleAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter("rate", &msScaleCmd);
            dict->addParameter("scale_range", &msScaleRangeCmd);
        }
    }
    //-----------------------------------------------------------------------
    void ScaleAffector::_initParticle(Particle* p)
    {
        float w = p->getOwnWidth();
        float h = p->getOwnHeight();
        float s = Math::RangeRandom(mScaleRange[0], mScaleRange[1]);
        p->setDimensions(s * w, s * h);
    }
    //-----------------------------------------------------------------------
    void ScaleAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        // Scale adjustments by time
        float ds = mScaleAdj * timeElapsed;

        for (auto p : pSystem->_getActiveParticles())
        {
            float w = std::max(0.0f, p->getOwnWidth() + ds);
            float h = std::max(0.0f, p->getOwnHeight() + ds);
            p->setDimensions(w, h);
        }
    }
    //-----------------------------------------------------------------------
    void ScaleAffector::setAdjust( Real rate )
    {
        mScaleAdj = rate;
    }
    //-----------------------------------------------------------------------
    Real ScaleAffector::getAdjust(void) const
    {
        return mScaleAdj;
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ScaleAffector::CmdScaleAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ScaleAffector*>(target)->getAdjust() );
    }
    void ScaleAffector::CmdScaleAdjust::doSet(void* target, const String& val)
    {
        static_cast<ScaleAffector*>(target)->setAdjust(
            StringConverter::parseReal(val));
    }

}



