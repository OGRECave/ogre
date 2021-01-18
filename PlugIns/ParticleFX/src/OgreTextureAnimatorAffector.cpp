// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#include "OgreTextureAnimatorAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {

class CmdStart : public ParamCommand
{
public:
    String doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const TextureAnimatorAffector*>(target)->getTexcoordStart());
    }
    void doSet(void* target, const String& val)
    {
        static_cast<TextureAnimatorAffector*>(target)->setTexcoordStart(StringConverter::parseInt(val));
    }
};
class CmdCount : public ParamCommand
{
public:
    String doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const TextureAnimatorAffector*>(target)->getTexcoordCount());
    }
    void doSet(void* target, const String& val)
    {
        static_cast<TextureAnimatorAffector*>(target)->setTexcoordCount(StringConverter::parseInt(val));
    }
};
class CmdDuration : public ParamCommand
{
public:
    String doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const TextureAnimatorAffector*>(target)->getDuration());
    }
    void doSet(void* target, const String& val)
    {
        static_cast<TextureAnimatorAffector*>(target)->setDuration(StringConverter::parseReal(val));
    }
};
class CmdOffset : public ParamCommand
{
public:
    String doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const TextureAnimatorAffector*>(target)->isRandomStartOffset());
    }
    void doSet(void* target, const String& val)
    {
        static_cast<TextureAnimatorAffector*>(target)->useRandomStartOffset(StringConverter::parseBool(val));
    }
};

    // init statics
    static CmdStart msStartCmd;
    static CmdCount msCountCmd;
    static CmdDuration msDurationCmd;
    static CmdOffset msOffset;

    //-----------------------------------------------------------------------
    TextureAnimatorAffector::TextureAnimatorAffector(ParticleSystem* psys) : ParticleAffector(psys)
    {
        mTexcoordStart = mTexcoordCount = mDuration = 0;
        mRandomStartOffset = false;
        mType = "TextureAnimator";

        // Init parameters
        if (createParamDictionary("TextureAnimatorAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("texcoord_start", "", PT_INT), &msStartCmd);
            dict->addParameter(ParameterDef("texcoord_count", "", PT_INT), &msCountCmd);
            dict->addParameter(ParameterDef("duration", "", PT_REAL), &msDurationCmd);
            dict->addParameter(ParameterDef("random_offset", "", PT_BOOL), &msOffset);
        }
    }
    //-----------------------------------------------------------------------
    void TextureAnimatorAffector::_initParticle(Particle* pParticle)
    {
        if (!mRandomStartOffset)
            return;

        pParticle->mRandomTexcoordOffset = Math::UnitRandom() * mTexcoordCount;
        pParticle->mTexcoordIndex = pParticle->mRandomTexcoordOffset;
    }

    void TextureAnimatorAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        // special case: randomly pick one cell in sprite-sheet
        if(mDuration < 0)
            return;

        for (auto p : pSystem->_getActiveParticles())
        {
            float particle_time = 1.0f - (p->mTimeToLive / p->mTotalTimeToLive);

            float speed = mDuration ? (p->mTimeToLive / mDuration) : 1.0f;
            uint8 idx = uint8(particle_time * speed * mTexcoordCount + p->mRandomTexcoordOffset) % mTexcoordCount;

            p->mTexcoordIndex = idx + mTexcoordStart;
        }
    }
}
