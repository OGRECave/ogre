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
#include "OgreStableHeaders.h"
#include "OgreParticleEmitterCommands.h"
#include "OgreParticleEmitter.h"

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
        String CmdUp::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const ParticleEmitter*>(target)->getUp() );
        }
        void CmdUp::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setUp(StringConverter::parseVector3(val));
        }
        //-----------------------------------------------------------------------
        String CmdDirPositionRef::doGet(const void* target) const
        {
            Vector4 val( static_cast<const ParticleEmitter*>(target)->getDirPositionReference() );
            val.w = static_cast<const ParticleEmitter*>(target)->getDirPositionReferenceEnabled();
            return StringConverter::toString( val );
        }
        void CmdDirPositionRef::doSet(void* target, const String& val)
        {
            const Vector4 parsed = StringConverter::parseVector4(val);
            const Vector3 vPos( parsed.x, parsed.y, parsed.z );
            static_cast<ParticleEmitter*>(target)->setDirPositionReference( vPos, parsed.w != 0 );
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

