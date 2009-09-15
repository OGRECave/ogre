/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __BoxEmitterFactory_H__
#define __BoxEmitterFactory_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreBoxEmitter.h"


namespace Ogre {

    /** Factory class for particle emitter of type "Box".
    @remarks
        Creates instances of BoxEmitter to be used in particle systems. 
    */
    class _OgreParticleFXExport BoxEmitterFactory : public ParticleEmitterFactory
    {
    protected:

    public:
        /** See ParticleEmitterFactory */
        String getName() const
        { 
            return "Box"; 
        }

        /** See ParticleEmitterFactory */
        ParticleEmitter* createEmitter(ParticleSystem* psys) 
        {
            ParticleEmitter* emit = OGRE_NEW BoxEmitter(psys);
            mEmitters.push_back(emit);
            return emit;
        }

    };

}

#endif

