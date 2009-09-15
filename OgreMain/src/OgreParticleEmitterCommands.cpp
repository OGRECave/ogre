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
#include "OgreStableHeaders.h"
#include "OgreParticleEmitterCommands.h"
#include "OgreParticleEmitter.h"
#include "OgreStringConverter.h"


namespace Ogre {

    namespace EmitterCommands {

        //-----------------------------------------------------------------------
        String CmdAngle::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getAngle() );
        }
        void CmdAngle::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setAngle(StringConverter::parseAngle(val));
        }
        //-----------------------------------------------------------------------
        String CmdColour::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getColour() );
        }
        void CmdColour::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColour(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdColourRangeStart::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getColourRangeStart() );
        }
        void CmdColourRangeStart::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColourRangeStart(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdColourRangeEnd::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getColourRangeEnd() );
        }
        void CmdColourRangeEnd::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColourRangeEnd(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdDirection::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getDirection() );
        }
        void CmdDirection::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setDirection(StringConverter::parseVector3(val));
        }
        //-----------------------------------------------------------------------
        String CmdEmissionRate::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getEmissionRate() );
        }
        void CmdEmissionRate::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setEmissionRate(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxTTL::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMaxTimeToLive() );
        }
        void CmdMaxTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinTTL::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMinTimeToLive() );
        }
        void CmdMinTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxVelocity::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMaxParticleVelocity() );
        }
        void CmdMaxVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinVelocity::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMinParticleVelocity() );
        }
        void CmdMinVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdPosition::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getPosition() );
        }
        void CmdPosition::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setPosition(StringConverter::parseVector3(val));
        }
        //-----------------------------------------------------------------------
        String CmdTTL::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getTimeToLive() );
        }
        void CmdTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdVelocity::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getParticleVelocity() );
        }
        void CmdVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdDuration::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getDuration() );
        }
        void CmdDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinDuration::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMinDuration() );
        }
        void CmdMinDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxDuration::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMaxDuration() );
        }
        void CmdMaxDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdRepeatDelay::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getRepeatDelay() );
        }
        void CmdRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setRepeatDelay(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinRepeatDelay::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMinRepeatDelay() );
        }
        void CmdMinRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinRepeatDelay(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxRepeatDelay::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getMaxRepeatDelay() );
        }
        void CmdMaxRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxRepeatDelay(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdName::doGet(const void* target) const
        {
            return 
                static_cast<const ParticleEmitter*>(target)->getName();
        }
        void CmdName::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setName(val);
        }
        //-----------------------------------------------------------------------
        String CmdEmittedEmitter::doGet(const void* target) const
        {
            return 
                static_cast<const ParticleEmitter*>(target)->getEmittedEmitter();
        }
        void CmdEmittedEmitter::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setEmittedEmitter(val);
        }
 

    
    }
}

