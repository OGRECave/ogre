/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright ) 2002 Tels <http://bloodgate.com> based on BoxEmitter

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
#ifndef __RingEmitter_H__
#define __RingEmitter_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreAreaEmitter.h"
#include "OgreMath.h"

namespace Ogre {

    /** Particle emitter which emits particles randomly from points inside a ring (e.g. a tube).
    @remarks
        This particle emitter emits particles from a ring-shaped area.
        The initial direction of these particles can either be a single
        direction (i.e. a line), a random scattering inside a cone, or a random
        scattering in all directions, depending the 'angle' parameter, which
        is the angle across which to scatter the particles either side of the
        base direction of the emitter. 
    */
    class _OgreParticleFXExport RingEmitter : public AreaEmitter
    {
    public:
        // See AreaEmitter
        /** Command object for inner size (see ParamCommand).*/
        class CmdInnerX : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for inner size (see ParamCommand).*/
        class CmdInnerY : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        RingEmitter(ParticleSystem* psys);

        /** See ParticleEmitter. */
        void _initParticle(Particle* pParticle);

        /** Sets the size of the clear space inside the area from where NO particles are emitted.
        @param x,y,z
            Parametric values describing the proportion of the shape which is hollow in each direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSize(Real x, Real y);

        /** Sets the x component of the area inside the ellipsoid which doesn't emit particles. 
        @param x
            Parametric value describing the proportion of the shape which is hollow in this direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSizeX(Real x);
        /** Sets the y component of the area inside the ellipsoid which doesn't emit particles. 
        @param y
            Parametric value describing the proportion of the shape which is hollow in this direction.
            E.g. 0 is solid, 0.5 is half-hollow etc
        */
        void setInnerSizeY(Real y);
        /** Gets the x component of the area inside the ellipsoid which doesn't emit particles. */
        Real getInnerSizeX(void) const;
        /** Gets the y component of the area inside the ellipsoid which doesn't emit particles. */
        Real getInnerSizeY(void) const;

    protected:
        // See ParticleEmitter
        static CmdInnerX msCmdInnerX;
        static CmdInnerY msCmdInnerY;

        /// Size of 'clear' center area (> 0 and < 1.0)
        Real mInnerSizex;
        Real mInnerSizey;



    };

}

#endif

