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

#include <OgreStableHeaders.h>

#include "OgrePlatformInformation.h"

#define __Mathlib_H__ //Neded to directly include OgreMathlibC

#include "Math/Array/OgreArrayConfig.h"
#include "Math/Array/C/OgreMathlibC.h"
#include "Math/Array/OgreBooleanMask.h"

namespace Ogre
{
    const ArrayReal MathlibC::HALF      = 0.5f;
    const ArrayReal MathlibC::ONE       = 1.0f;
    const ArrayReal MathlibC::THREE     = 3.0f;
    const ArrayReal MathlibC::NEG_ONE   = -1.0f;
    const ArrayReal MathlibC::fEpsilon  = 1e-6f;
    const ArrayReal MathlibC::fSqEpsilon= 1e-12f;
    const ArrayReal MathlibC::OneMinusEpsilon= 1.0f - 1e-6f;
    const ArrayReal MathlibC::FLOAT_MIN = std::numeric_limits<Real>::min();
    const ArrayReal MathlibC::SIGN_MASK = -0.0f;
    const ArrayReal MathlibC::INFINITEA = std::numeric_limits<Real>::infinity();
    const ArrayReal MathlibC::MAX_NEG   = -std::numeric_limits<Real>::max();
    const ArrayReal MathlibC::MAX_POS   = std::numeric_limits<Real>::max();

    static const Real _PI = Real( 4.0 * atan( 1.0 ) );
    //We can't use Math::fDeg2Rad & Math::fRad2Deg directly because
    //it's not guaranteed to have been initialized first
    const ArrayReal MathlibC::PI        = _PI;
    const ArrayReal MathlibC::TWO_PI    = 2.0f * _PI;
    const ArrayReal MathlibC::fDeg2Rad  = _PI / 180.0f;
    const ArrayReal MathlibC::fRad2Deg  = 180.0f / _PI;

    const ArrayReal MathlibC::ONE_DIV_2PI= 1.0f / (2.0f * _PI);
}
