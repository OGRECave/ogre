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

#include "OgreParticleEmitter.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleEmitterCommands.h"

namespace Ogre
{
    // Command object for setting / getting parameters
    static EmitterCommands::CmdAngle msAngleCmd;
    static EmitterCommands::CmdColour msColourCmd;
    static EmitterCommands::CmdColourRangeStart msColourRangeStartCmd;
    static EmitterCommands::CmdColourRangeEnd msColourRangeEndCmd;
    static EmitterCommands::CmdDirection msDirectionCmd;
    static EmitterCommands::CmdUp msUpCmd;
    static EmitterCommands::CmdDirPositionRef msDirPositionRefCmd;
    static EmitterCommands::CmdEmissionRate msEmissionRateCmd;
    static EmitterCommands::CmdMaxTTL msMaxTTLCmd;
    static EmitterCommands::CmdMaxVelocity msMaxVelocityCmd;
    static EmitterCommands::CmdMinTTL msMinTTLCmd;
    static EmitterCommands::CmdMinVelocity msMinVelocityCmd;
    static EmitterCommands::CmdPosition msPositionCmd;
    static EmitterCommands::CmdTTL msTTLCmd;
    static EmitterCommands::CmdVelocity msVelocityCmd;
    static EmitterCommands::CmdDuration msDurationCmd;
    static EmitterCommands::CmdMinDuration msMinDurationCmd;
    static EmitterCommands::CmdMaxDuration msMaxDurationCmd;
    static EmitterCommands::CmdRepeatDelay msRepeatDelayCmd;
    static EmitterCommands::CmdMinRepeatDelay msMinRepeatDelayCmd;
    static EmitterCommands::CmdMaxRepeatDelay msMaxRepeatDelayCmd;
    static EmitterCommands::CmdName msNameCmd;
    static EmitterCommands::CmdEmittedEmitter msEmittedEmitterCmd;


    //-----------------------------------------------------------------------
    ParticleEmitter::ParticleEmitter(ParticleSystem* psys)
      : mParent(psys),
        mUseDirPositionRef(false),
        mDirPositionRef(Vector3::ZERO),
        mStartTime(0),
        mDurationMin(0),
        mDurationMax(0),
        mDurationRemain(0),
        mRepeatDelayMin(0),
        mRepeatDelayMax(0),
        mRepeatDelayRemain(0)
    {

        // Reasonable defaults
        mAngle = 0;
        setDirection(Vector3::UNIT_X);
        mEmissionRate = 10;
        mMaxSpeed = mMinSpeed = 1;
        mMaxTTL = mMinTTL = 5;
        mPosition = Vector3::ZERO;
        mColourRangeStart = mColourRangeEnd = ColourValue::White;
        mEnabled = true;
        mRemainder = 0;
        mName = BLANKSTRING;
        mEmittedEmitter = BLANKSTRING;
        mEmitted = false;
    }
    //-----------------------------------------------------------------------
    ParticleEmitter::~ParticleEmitter() 
    {
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setPosition(const Vector3& pos) 
    { 
        mPosition = pos; 
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleEmitter::getPosition(void) const 
    { 
        return mPosition; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDirection(const Vector3& inDirection) 
    { 
        mDirection = inDirection; 
        mDirection.normalise();
        // Generate a default up vector.
        mUp = mDirection.perpendicular();
        mUp.normalise();
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleEmitter::getDirection(void) const
    { 
        return mDirection; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setUp(const Vector3& inUp) 
    {
        mUp = inUp; 
        mUp.normalise();
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleEmitter::getUp(void) const
    { 
        return mUp; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDirPositionReference( const Vector3& nposition, bool enable )
    { 
    mUseDirPositionRef  = enable;
        mDirPositionRef     = nposition;
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleEmitter::getDirPositionReference() const
    {
        return mDirPositionRef;
    }
    //-----------------------------------------------------------------------
    bool ParticleEmitter::getDirPositionReferenceEnabled() const
    {
        return mUseDirPositionRef;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setAngle(const Radian& angle)
    {
        // Store as radians for efficiency
        mAngle = angle;
    }
    //-----------------------------------------------------------------------
    const Radian& ParticleEmitter::getAngle(void) const
    {
        return mAngle;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setParticleVelocity(Real speed)
    {
        assert(std::isfinite(speed));
        mMinSpeed = mMaxSpeed = speed;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setParticleVelocity(Real min, Real max)
    {
        assert(std::isfinite(min) && std::isfinite(max));
        mMinSpeed = min;
        mMaxSpeed = max;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEmissionRate(Real particlesPerSecond) 
    { 
        mEmissionRate = particlesPerSecond; 
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getEmissionRate(void) const 
    { 
        return mEmissionRate; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setTimeToLive(Real ttl)
    {
        assert (ttl >= 0 && "Time to live can not be negative");
        mMinTTL = mMaxTTL = ttl;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setTimeToLive(Real minTtl, Real maxTtl)
    {
        assert (minTtl >= 0 && "Time to live can not be negative");
        assert (maxTtl >= 0 && "Time to live can not be negative");
        mMinTTL = minTtl;
        mMaxTTL = maxTtl;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColour(const ColourValue& inColour)
    {
        mColourRangeStart = mColourRangeEnd = inColour;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColour(const ColourValue& colourStart, const ColourValue& colourEnd)
    {
        mColourRangeStart = colourStart;
        mColourRangeEnd = colourEnd;
    }
    //-----------------------------------------------------------------------
    const String& ParticleEmitter::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setName(const String& newName)
    {
        mName = newName;
    }
    //-----------------------------------------------------------------------
    const String& ParticleEmitter::getEmittedEmitter(void) const
    {
        return mEmittedEmitter;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEmittedEmitter(const String& emittedEmitter)
    {
        mEmittedEmitter = emittedEmitter;
    }
    //-----------------------------------------------------------------------
    bool ParticleEmitter::isEmitted(void) const
    {
        return mEmitted;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEmitted(bool emitted)
    {
        mEmitted = emitted;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionDirection( const Vector3 &particlePos, Vector3& destVector )
    {
        if( mUseDirPositionRef )
        {
            Vector3 particleDir = particlePos - mDirPositionRef;
            particleDir.normalise();

            if (mAngle != Radian(0))
            {
                // Randomise angle
                Radian angle = Math::UnitRandom() * mAngle;

                // Randomise direction
                destVector = particleDir.randomDeviant( angle );
            }
            else
            {
                // Constant angle
                destVector = particleDir;
            }
        }
        else
        {
            if (mAngle != Radian(0))
            {
                // Randomise angle
                Radian angle = Math::UnitRandom() * mAngle;

                // Randomise direction
                destVector = mDirection.randomDeviant(angle, mUp);
            }
            else
            {
                // Constant angle
                destVector = mDirection;
            }
        }

        // Don't normalise, we can assume that it will still be a unit vector since
        // both direction and 'up' are.
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionVelocity(Vector3& destVector)
    {
        Real scalar;
        if (mMinSpeed != mMaxSpeed)
        {
            scalar = mMinSpeed + (Math::UnitRandom() * (mMaxSpeed - mMinSpeed));
        }
        else
        {
            scalar = mMinSpeed;
        }

        destVector *= scalar;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::genEmissionTTL(void)
    {
        if (mMaxTTL != mMinTTL)
        {
            return mMinTTL + (Math::UnitRandom() * (mMaxTTL - mMinTTL));
        }
        else
        {
            return mMinTTL;
        }
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleEmitter::genConstantEmissionCount(Real timeElapsed)
    {
        if (mEnabled)
        {
            // Keep fractions, otherwise a high frame rate will result in zero emissions!
            mRemainder += mEmissionRate * timeElapsed;
            unsigned short intRequest = (unsigned short)mRemainder;
            mRemainder -= intRequest;

            // Check duration
            if (mDurationMax)
            {
                mDurationRemain -= timeElapsed;
                if (mDurationRemain <= 0) 
                {
                    // Disable, duration is out (takes effect next time)
                    setEnabled(false);
                }
            }
            return intRequest;
        }
        else
        {
            // Check repeat
            if (mRepeatDelayMax)
            {
                mRepeatDelayRemain -= timeElapsed;
                if (mRepeatDelayRemain <= 0)
                {
                    // Enable, repeat delay is out (takes effect next time)
                    setEnabled(true);
                }
            }
            if(mStartTime)
            {
                mStartTime -= timeElapsed;
                if(mStartTime <= 0)
                {
                    setEnabled(true);
                    mStartTime = 0;
                }
            }
            return 0;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionColour(ColourValue& destColour)
    {
        if (mColourRangeStart != mColourRangeEnd)
        {
            // Randomise
            //Real t = Math::UnitRandom();
            destColour.r = mColourRangeStart.r + (Math::UnitRandom() * (mColourRangeEnd.r - mColourRangeStart.r));
            destColour.g = mColourRangeStart.g + (Math::UnitRandom() * (mColourRangeEnd.g - mColourRangeStart.g));
            destColour.b = mColourRangeStart.b + (Math::UnitRandom() * (mColourRangeEnd.b - mColourRangeStart.b));
            destColour.a = mColourRangeStart.a + (Math::UnitRandom() * (mColourRangeEnd.a - mColourRangeStart.a));
        }
        else
        {
            destColour = mColourRangeStart;
        }
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::addBaseParameters(void)    
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("angle", 
            "The angle up to which particles may vary in their initial direction "
            "from the emitters direction, in degrees." , PT_REAL),
            &msAngleCmd);

        dict->addParameter(ParameterDef("colour", 
            "The colour of emitted particles.", PT_COLOURVALUE),
            &msColourCmd);

        dict->addParameter(ParameterDef("colour_range_start", 
            "The start of a range of colours to be assigned to emitted particles.", PT_COLOURVALUE),
            &msColourRangeStartCmd);

        dict->addParameter(ParameterDef("colour_range_end", 
            "The end of a range of colours to be assigned to emitted particles.", PT_COLOURVALUE),
            &msColourRangeEndCmd);

        dict->addParameter(ParameterDef("direction", 
            "The base direction of the emitter." , PT_VECTOR3),
            &msDirectionCmd);

        dict->addParameter(ParameterDef("up", 
            "The up vector of the emitter." , PT_VECTOR3),
            &msUpCmd);

        dict->addParameter(ParameterDef("direction_position_reference", 
            "The reference position to calculate the direction of emitted particles "
            "based on their position. Good for explosions and implosions (use negative velocity)" , PT_COLOURVALUE),
            &msDirPositionRefCmd);

        dict->addParameter(ParameterDef("emission_rate", 
            "The number of particles emitted per second." , PT_REAL),
            &msEmissionRateCmd);

        dict->addParameter(ParameterDef("position", 
            "The position of the emitter relative to the particle system center." , PT_VECTOR3),
            &msPositionCmd);

        dict->addParameter(ParameterDef("velocity", 
            "The initial velocity to be assigned to every particle, in world units per second." , PT_REAL),
            &msVelocityCmd);

        dict->addParameter(ParameterDef("velocity_min", 
            "The minimum initial velocity to be assigned to each particle." , PT_REAL),
            &msMinVelocityCmd);

        dict->addParameter(ParameterDef("velocity_max", 
            "The maximum initial velocity to be assigned to each particle." , PT_REAL),
            &msMaxVelocityCmd);

        dict->addParameter(ParameterDef("time_to_live", 
            "The lifetime of each particle in seconds." , PT_REAL),
            &msTTLCmd);

        dict->addParameter(ParameterDef("time_to_live_min", 
            "The minimum lifetime of each particle in seconds." , PT_REAL),
            &msMinTTLCmd);

        dict->addParameter(ParameterDef("time_to_live_max", 
            "The maximum lifetime of each particle in seconds." , PT_REAL),
            &msMaxTTLCmd);

        dict->addParameter(ParameterDef("duration", 
            "The length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msDurationCmd);

        dict->addParameter(ParameterDef("duration_min", 
            "The minimum length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msMinDurationCmd);

        dict->addParameter(ParameterDef("duration_max", 
            "The maximum length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msMaxDurationCmd);

        dict->addParameter(ParameterDef("repeat_delay", 
            "If set, after disabling an emitter will repeat (reenable) after this many seconds." , PT_REAL),
            &msRepeatDelayCmd);

        dict->addParameter(ParameterDef("repeat_delay_min", 
            "If set, after disabling an emitter will repeat (reenable) after this minimum number of seconds." , PT_REAL),
            &msMinRepeatDelayCmd);

        dict->addParameter(ParameterDef("repeat_delay_max", 
            "If set, after disabling an emitter will repeat (reenable) after this maximum number of seconds." , PT_REAL),
            &msMaxRepeatDelayCmd);

        dict->addParameter(ParameterDef("name", 
            "This is the name of the emitter" , PT_STRING),
            &msNameCmd);
        
        dict->addParameter(ParameterDef("emit_emitter", 
            "If set, this emitter will emit other emitters instead of visual particles" , PT_STRING),
            &msEmittedEmitterCmd);
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getParticleVelocity(void) const
    {
        return mMinSpeed;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinParticleVelocity(void) const
    {
        return mMinSpeed;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxParticleVelocity(void) const
    {
        return mMaxSpeed;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinParticleVelocity(Real min)
    {
        assert(std::isfinite(min));
        mMinSpeed = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxParticleVelocity(Real max)
    {
        assert(std::isfinite(max));
        mMaxSpeed = max;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getTimeToLive(void) const
    {
        return mMinTTL;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinTimeToLive(void) const
    {
        return mMinTTL;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxTimeToLive(void) const
    {
        return mMaxTTL;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinTimeToLive(Real min)
    {
        mMinTTL = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxTimeToLive(Real max)
    {
        mMaxTTL = max;
    }
    //-----------------------------------------------------------------------
    const ColourValue& ParticleEmitter::getColour(void) const
    {
        return mColourRangeStart;
    }
    //-----------------------------------------------------------------------
    const ColourValue& ParticleEmitter::getColourRangeStart(void) const
    {
        return mColourRangeStart;
    }
    //-----------------------------------------------------------------------
    const ColourValue& ParticleEmitter::getColourRangeEnd(void) const
    {
        return mColourRangeEnd;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColourRangeStart(const ColourValue& val)
    {
        mColourRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColourRangeEnd(const ColourValue& val )
    {
        mColourRangeEnd = val;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEnabled(bool enabled)
    {
        mEnabled = enabled;
        // Reset duration & repeat
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    bool ParticleEmitter::getEnabled(void) const
    {
        return mEnabled;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setStartTime(Real startTime)
    {
        setEnabled(false);
        mStartTime = startTime;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getStartTime(void) const
    {
        return mStartTime;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDuration(Real duration)
    {
        setDuration(duration, duration);
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getDuration(void) const
    {
        return mDurationMin;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDuration(Real min, Real max)
    {
        mDurationMin = min;
        mDurationMax = max;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinDuration(Real min)
    {
        mDurationMin = min;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxDuration(Real max)
    {
        mDurationMax = max;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::initDurationRepeat(void)
    {
        if (mEnabled)
        {
            if (mDurationMin == mDurationMax)
            {
                mDurationRemain = mDurationMin;
            }
            else
            {
                mDurationRemain = Math::RangeRandom(mDurationMin, mDurationMax);
            }
        }
        else
        {
            // Reset repeat
            if (mRepeatDelayMin == mRepeatDelayMax)
            {
                mRepeatDelayRemain = mRepeatDelayMin;
            }
            else
            {
                mRepeatDelayRemain = Math::RangeRandom(mRepeatDelayMax, mRepeatDelayMin);
            }

        }
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setRepeatDelay(Real delay)
    {
        setRepeatDelay(delay, delay);
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getRepeatDelay(void) const
    {
        return mRepeatDelayMin;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setRepeatDelay(Real min, Real max)
    {
        mRepeatDelayMin = min;
        mRepeatDelayMax = max;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinRepeatDelay(Real min)
    {
        mRepeatDelayMin = min;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxRepeatDelay(Real max)
    {
        mRepeatDelayMax = max;
        initDurationRepeat();
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinDuration(void) const
    {
        return mDurationMin;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxDuration(void) const
    {
        return mDurationMax;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinRepeatDelay(void) const
    {
        return mRepeatDelayMin;    
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxRepeatDelay(void) const
    {
        return mRepeatDelayMax;    
    }

    //-----------------------------------------------------------------------
    ParticleEmitterFactory::~ParticleEmitterFactory()
    {
        // Destroy all emitters
        std::vector<ParticleEmitter*>::iterator i;
        for (i = mEmitters.begin(); i != mEmitters.end(); ++i)
        {
            OGRE_DELETE (*i);
        }
            
        mEmitters.clear();

    }
    //-----------------------------------------------------------------------
    void ParticleEmitterFactory::destroyEmitter(ParticleEmitter* e)        
    {
        std::vector<ParticleEmitter*>::iterator i;
        for (i = mEmitters.begin(); i != mEmitters.end(); ++i)
        {
            if ((*i) == e)
            {
                mEmitters.erase(i);
                OGRE_DELETE e;
                break;
            }
        }
    }

    //-----------------------------------------------------------------------

}

