// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TextureAnimatorAffector_H__
#define __TextureAnimatorAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"
#include "OgreParticleAffectorFactory.h"

namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup ParticleFX
    *  @{
    */

	/** This affector makes it possible to have an animated texture for each individual particle.
    */
    class TextureAnimatorAffector : public ParticleAffector
    {
    public:
        TextureAnimatorAffector(ParticleSystem* psys);

        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed) override;

        void _initParticle(Particle* pParticle) override;

        /** Sets the texcoord index at which the animation should start
         *
         * default: 0
         */
        void setTexcoordStart(uint8 start) { mTexcoordStart = start; }
        uint8 getTexcoordStart(void) const { return mTexcoordStart; }

        /** Sets the number of texcoords to use.
         * @note you must set this one and start + count must not exceed the number of frames in the sprite sheet
         */
        void setTexcoordCount(uint8 count) { mTexcoordCount = count; }
        uint8 getTexcoordCount(void) const { return mTexcoordCount; }
        /** The length of time it takes to display the whole animation sequence, in seconds.
         *
         * If set to 0 (default) the duration equals the particle time to live. Using a different value here,
         * you can play back the animation faster (looping) or slower.
         * When negative, the animation is disabled. This is useful to just pick a random sprite (see below) and keep it.
         */
        void setDuration(float duration) { mDuration = duration; }
        float getDuration(void) const { return mDuration; }

        /** Start animation at random frame in the texture sheet. Useful to randomly phase the animation between particles.
         */
        void useRandomStartOffset(float enable) { mRandomStartOffset = enable; }
        float isRandomStartOffset(void) const { return mRandomStartOffset; }
    protected:
        bool mRandomStartOffset;
        uint8 mTexcoordStart;
        uint8 mTexcoordCount;
        float mDuration;
    };

    /** Factory class for TextureAnimatorAffector. */
    class TextureAnimatorAffectorFactory : public ParticleAffectorFactory
    {
        String getName() const override { return "TextureAnimator"; }

        ParticleAffector* createAffector(ParticleSystem* psys) override
        {
            ParticleAffector* p = OGRE_NEW TextureAnimatorAffector(psys);
            mAffectors.push_back(p);
            return p;
        }
    };

    /** @} */
    /** @} */
}

#endif
