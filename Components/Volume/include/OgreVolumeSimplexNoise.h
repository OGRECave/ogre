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
#ifndef __Ogre_Simplex_Noise_H__
#define __Ogre_Simplex_Noise_H__

#include "OgreVector3.h"

#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** Simplex Noise ported from public domain Java Implementation
     http://webstaff.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java
     Thanks Stefan Gustavson!
    */
    class _OgreVolumeExport SimplexNoise
    {
    private:
        
        /// Skewing and unskewing factor for 3 dimensions.
        static Real F3;
        
        /// Skewing and unskewing factor for 3 dimensions.
        static Real G3;

        /// Random seed.
        long mSeed;

        /// Permutation table.
        short perm[512];

        /// Permutation table modulo 12.
        short permMod12[512];

        /* Gets a random long.
        @return
            A random long.
        */
        unsigned long random(void);
        
        /** Dot product of a gradient with the given values.
        @param g
            The gradient.
        @param x
            The first value.
        @param y
            The second value.
        @param z
            The third value.
        @return
            The dot product.
        */
        inline Real dot(const Vector3 &g, Real x, Real y, Real z) const
        {
            return g.x * x + g.y * y + g.z * z;
        }
                
        /** Initializes the SimplexNoise instance.
        */
        void init(unsigned long definedSeed);

    public:

        /** Constructor with a random permutation table.
        */
        SimplexNoise(void);

        /** Constructor with a by seed defined permutation table.
        @param definedSeed
            The seed to use.
        */
        SimplexNoise(unsigned long definedSeed);

        /** 3D noise function.
        @param xIn
            The first dimension parameter.
        @param yIn
            The second dimension parameter.
        @param zIn
            The third dimension parameter.
        @return
            The noise value.
        */
        Real noise(Real xIn, Real yIn, Real zIn) const;
        
        /** Gets the current seed.
        @return
            The current seed.
        */
        long getSeed(void) const;

    };

}
}

#endif
