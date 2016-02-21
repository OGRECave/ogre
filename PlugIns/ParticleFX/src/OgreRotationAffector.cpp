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
#include "OgreRotationAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
    RotationAffector::CmdRotationSpeedRangeStart    RotationAffector::msRotationSpeedRangeStartCmd;
    RotationAffector::CmdRotationSpeedRangeEnd      RotationAffector::msRotationSpeedRangeEndCmd;
    RotationAffector::CmdRotationRangeStart         RotationAffector::msRotationRangeStartCmd;
    RotationAffector::CmdRotationRangeEnd           RotationAffector::msRotationRangeEndCmd;
    
    //-----------------------------------------------------------------------
    RotationAffector::RotationAffector(ParticleSystem* psys) :
        ParticleAffector(psys),
        mRotationSpeedRangeStart(0),
        mRotationSpeedRangeEnd(0),
        mRotationRangeStart(0),
        mRotationRangeEnd(0)
    {
        mType = "Rotator";

        // Init parameters
        if (createParamDictionary("RotationAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("rotation_speed_range_start", 
                "The start of a range of rotation speed to be assigned to emitted particles.", PT_REAL),
                &msRotationSpeedRangeStartCmd);

            dict->addParameter(ParameterDef("rotation_speed_range_end", 
                "The end of a range of rotation speed to be assigned to emitted particles.", PT_REAL),
                &msRotationSpeedRangeEndCmd);

            dict->addParameter(ParameterDef("rotation_range_start", 
                "The start of a range of rotation angles to be assigned to emitted particles.", PT_REAL),
                &msRotationRangeStartCmd);

            dict->addParameter(ParameterDef("rotation_range_end", 
                "The end of a range of rotation angles to be assigned to emitted particles.", PT_REAL),
                &msRotationRangeEndCmd);
        }
    }

    //-----------------------------------------------------------------------
    void RotationAffector::_initParticle(Particle* pParticle)
    {
        pParticle->setRotation(
            mRotationRangeStart + 
            (Math::UnitRandom() * 
                (mRotationRangeEnd - mRotationRangeStart)));
        pParticle->mRotationSpeed =
            mRotationSpeedRangeStart + 
            (Math::UnitRandom() * 
                (mRotationSpeedRangeEnd - mRotationSpeedRangeStart));
        
    }
    //-----------------------------------------------------------------------
    void RotationAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;
        Real ds;

        // Rotation adjustments by time
        ds = timeElapsed;

        Radian NewRotation;

        while (!pi.end())
        {
            p = pi.getNext();

            NewRotation = p->mRotation + (ds * p->mRotationSpeed);
            p->setRotation( NewRotation );
        }

    }
    //-----------------------------------------------------------------------
    const Radian& RotationAffector::getRotationSpeedRangeStart(void) const
    {
        return mRotationSpeedRangeStart;
    }
    //-----------------------------------------------------------------------
    const Radian& RotationAffector::getRotationSpeedRangeEnd(void) const
    {
        return mRotationSpeedRangeEnd;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationSpeedRangeStart(const Radian& val)
    {
        mRotationSpeedRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationSpeedRangeEnd(const Radian& val )
    {
        mRotationSpeedRangeEnd = val;
    }
    //-----------------------------------------------------------------------
    const Radian& RotationAffector::getRotationRangeStart(void) const
    {
        return mRotationRangeStart;
    }
    //-----------------------------------------------------------------------
    const Radian& RotationAffector::getRotationRangeEnd(void) const
    {
        return mRotationRangeEnd;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationRangeStart(const Radian& val)
    {
        mRotationRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationRangeEnd(const Radian& val )
    {
        mRotationRangeEnd = val;
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationSpeedRangeEnd::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationSpeedRangeEnd() );
    }
    void RotationAffector::CmdRotationSpeedRangeEnd::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationSpeedRangeEnd(StringConverter::parseAngle(val));
    }
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationSpeedRangeStart::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationSpeedRangeStart() );
    }
    void RotationAffector::CmdRotationSpeedRangeStart::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationSpeedRangeStart(StringConverter::parseAngle(val));
    }
    
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationRangeEnd::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationRangeEnd() );
    }
    void RotationAffector::CmdRotationRangeEnd::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationRangeEnd(StringConverter::parseAngle(val));
    }
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationRangeStart::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationRangeStart() );
    }
    void RotationAffector::CmdRotationRangeStart::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationRangeStart(StringConverter::parseAngle(val));
    }
}



