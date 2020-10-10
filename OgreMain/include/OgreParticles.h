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
#ifndef __Particles2_H__
#define __Particles2_H__

#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"
#include "OgreColourValue.h"
#include "OgreMath.h"
#include "OgreVector.h"

#include "sh/cpp/block_n.hpp"

namespace Ogre {


    struct ParticleTtl
    {
        Real remaining;
        Real total;
    };

    struct ParticleTranslation
    {
        Vector3 position;
        Vector3 direction;
    };

    struct ParticleRotation
    {
        Real angle;
        Real speed;
    };

    struct ParticleSize
    {
        Real width;
        Real height;
    };

    struct Particles2 : public FXAlloc, private block_n <ParticleTtl, ParticleTranslation, ParticleRotation, ColourValue, ParticleSize>
    {
        Particles2 ();
        ~Particles2 ();

        unsigned size;
        unsigned active;

        ParticleTtl* ttls;
        ParticleTranslation* translations;
        ParticleRotation* rotations;
        ColourValue* colors;
        ParticleSize* sizes;

        void clear ();
        void create (unsigned count, unsigned& start, unsigned& end);
        void resize (unsigned newsize);
        void swap (unsigned idx1, unsigned idx2);
    };
}

#include "OgreHeaderSuffix.h"

#endif

