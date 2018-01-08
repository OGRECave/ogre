/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreVolumeSimplexNoise.h"

#include <time.h>

#include <cmath>

namespace Ogre {
namespace Volume {

    Real SimplexNoise::F3 = (Real)(1.0 / 3.0);
    Real SimplexNoise::G3 = (Real)(1.0 / 6.0);
    
    Vector3 grad3[] = {
        Vector3(1,1,0), Vector3(-1,1,0), Vector3(1,-1,0), Vector3(-1,-1,0),
        Vector3(1,0,1), Vector3(-1,0,1), Vector3(1,0,-1), Vector3(-1,0,-1),
        Vector3(0,1,1), Vector3(0,-1,1), Vector3(0,1,-1), Vector3(0,-1,-1)
    };

    //-----------------------------------------------------------------------
    
    unsigned long SimplexNoise::random(void)
    {
        // Simple XORShift random number generator like in http://www.jstatsoft.org/v08/i14/paper
        mSeed ^= mSeed << 13;
        mSeed = mSeed >> 17;
        return mSeed ^= mSeed << 5;
    }
    
    //-----------------------------------------------------------------------
    
    void SimplexNoise::init(unsigned long definedSeed)
    {
        mSeed = definedSeed;
        short p[256];
        for (int i = 0; i < 256; ++i)
        {
            p[i] = (short)(random() % 256);
        }

        for(int i=0; i<512; i++)
        {
            perm[i]= p[i & 255];
            permMod12[i] = (short)(perm[i] % 12);
        }
    }
    
    //-----------------------------------------------------------------------
    
    SimplexNoise::SimplexNoise(void)
    {
        init((unsigned long)time(0));
    }
    
    //-----------------------------------------------------------------------
    
    SimplexNoise::SimplexNoise(unsigned long definedSeed)
    {
        init(definedSeed);
    }
    
    //-----------------------------------------------------------------------
    
    Real SimplexNoise::noise(Real xIn, Real yIn, Real zIn) const
    {
        Real n0, n1, n2, n3; // Noise contributions from the four corners
        // Skew the input space to determine which simplex cell we're in
        Real s = (xIn + yIn + zIn) * F3; // Very nice and simple skew factor for 3D
        int i = (int)std::floor(xIn + s);
        int j = (int)std::floor(yIn + s);
        int k = (int)std::floor(zIn + s);
        Real t = (i + j + k) * G3;
        Real X0 = i - t; // Unskew the cell origin back to (x,y,z) space
        Real Y0 = j - t;
        Real Z0 = k - t;
        Real x0 = xIn - X0; // The x,y,z distances from the cell origin
        Real y0 = yIn - Y0;
        Real z0 = zIn - Z0;
        // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
        // Determine which simplex we are in.
        int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
        int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
        if (x0 >= y0)
        {
            if (y0 >= z0)  // X Y Z order
            {
                i1 = 1;
                j1 = 0;
                k1 = 0;
                i2 = 1;
                j2 = 1;
                k2 = 0;
            }
            else if (x0 >= z0) // X Z Y order
            {
                i1 = 1;
                j1 = 0;
                k1 = 0;
                i2 = 1;
                j2 = 0;
                k2 = 1;
            }
            else // Z X Y order
            {
                i1 = 0;
                j1 = 0;
                k1 = 1;
                i2 = 1;
                j2 = 0;
                k2 = 1;
            } 
        }
        else // x0<y0
        { 
            if (y0 < z0) // Z Y X order
            {
                i1 = 0;
                j1 = 0;
                k1 = 1;
                i2 = 0;
                j2 = 1;
                k2 = 1;
            }
            else if(x0 < z0) // Y Z X order
            {
                i1 = 0;
                j1 = 1;
                k1 = 0;
                i2 = 0;
                j2 = 1;
                k2 = 1;
            }
            else // Y X Z order
            {
                i1 = 0;
                j1 = 1;
                k1 = 0;
                i2 = 1;
                j2 = 1;
                k2 = 0;
            } 
        }
        // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
        // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
        // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
        // c = 1/6.
        Real x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
        Real y1 = y0 - j1 + G3;
        Real z1 = z0 - k1 + G3;
        Real x2 = x0 - i2 + (Real)2.0 * G3; // Offsets for third corner in (x,y,z) coords
        Real y2 = y0 - j2 + (Real)2.0*G3;
        Real z2 = z0 - k2 + (Real)2.0*G3;
        Real x3 = x0 - (Real)1.0 + (Real)3.0 * G3; // Offsets for last corner in (x,y,z) coords
        Real y3 = y0 - (Real)1.0 + (Real)3.0 * G3;
        Real z3 = z0 - (Real)1.0 + (Real)3.0 * G3;
        // Work out the hashed gradient indices of the four simplex corners
        int ii = i & 255;
        int jj = j & 255;
        int kk = k & 255;
        int gi0 = permMod12[ii + perm[jj + perm[kk]]];
        int gi1 = permMod12[ii + i1 + perm[jj + j1 + perm[kk + k1]]];
        int gi2 = permMod12[ii + i2 + perm[jj + j2 + perm[kk + k2]]];
        int gi3 = permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]];
        // Calculate the contribution from the four corners
        Real t0 = (Real)0.6 - x0 * x0 - y0 * y0 - z0 * z0;
        if (t0 < 0)
        {
            n0 = 0.0;
        }
        else
        {
            t0 *= t0;
            n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
        }
        Real t1 = (Real)0.6 - x1 * x1 - y1 * y1 - z1 * z1;
        if (t1 < 0)
        {
            n1 = 0.0;
        }
        else
        {
            t1 *= t1;
            n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
        }
        Real t2 = (Real)0.6 - x2 * x2 - y2 * y2 - z2 * z2;
        if (t2 < 0)
        {
            n2 = 0.0;
        }
        else
        {
            t2 *= t2;
            n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
        }
        Real t3 = (Real)0.6 - x3 * x3 - y3 * y3 - z3 * z3;
        if (t3 < 0)
        {
            n3 = 0.0;
        }
        else
        {
            t3 *= t3;
            n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
        }
        // Add contributions from each corner to get the final noise value.
        // The result is scaled to stay just inside [-1,1]
        return (Real)32.0 * (n0 + n1 + n2 + n3);
    }
    
    //-----------------------------------------------------------------------
    
    long SimplexNoise::getSeed(void) const
    {
        return mSeed;
    }

}
}
