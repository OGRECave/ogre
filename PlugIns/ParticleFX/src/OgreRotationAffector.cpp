/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreRotationAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
	RotationAffector::CmdRotationSpeedRangeStart	RotationAffector::msRotationSpeedRangeStartCmd;
    RotationAffector::CmdRotationSpeedRangeEnd		RotationAffector::msRotationSpeedRangeEndCmd;
    RotationAffector::CmdRotationRangeStart			RotationAffector::msRotationRangeStartCmd;
    RotationAffector::CmdRotationRangeEnd			RotationAffector::msRotationRangeEndCmd;
    
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
        pParticle->rotationSpeed =
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

			NewRotation = p->rotation + (ds * p->rotationSpeed);
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



