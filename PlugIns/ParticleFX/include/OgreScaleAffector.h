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
#ifndef __ScaleAffector_H__
#define __ScaleAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"

namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup ParticleFX
    *  @{
    */

    /** This affector scales particles in flight.
    */
    class _OgreParticleFXExport ScaleAffector : public ParticleAffector
    {
    public:

        /** Command object for scale adjust (see ParamCommand).*/
        class CmdScaleAdjust : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Default constructor. */
        ScaleAffector(ParticleSystem* psys);

        void _initParticle(Particle* pParticle) override;
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed) override;

        /** Sets the scale adjustment to be made per second to particles. 
        @param rate
            Sets the adjustment to be made to the x and y scale components per second. These
            values will be added to the scale of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setAdjust( Real rate );

        /** Gets the scale adjustment to be made per second to particles. */
        Real getAdjust(void) const;

        const Vector2& getScaleRange() const {  return mScaleRange; }
        void setScaleRange(const Vector2& range) {  mScaleRange = range; }

        static CmdScaleAdjust msScaleCmd;

    protected:
        float mScaleAdj;
        Vector2 mScaleRange;
    };

    /** @} */
    /** @} */
}


#endif

