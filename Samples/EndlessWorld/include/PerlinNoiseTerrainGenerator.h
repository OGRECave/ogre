#pragma once

#include "OgreTerrainPagedWorldSection.h"

#define B 0x100
using namespace Ogre;

///Use PerlinNoise algorithm to generate terrain height map
class PerlinNoiseTerrainGenerator : public TerrainPagedWorldSection::TerrainDefiner
{
public:
    /** Constructor of PerlinNoiseTerrainGenerator
        @param alpha The weight when the sum is formed
        @param beta The harmonic scaling/spacing
        @param iterationNum The iterations num to produce a point.
        @param cycle The terrain size maps to one cycle of perlin curve
        @param heightScale The height maps to curve amplitude
    */
    PerlinNoiseTerrainGenerator(const Real& alpha = 3.3, const Real& beta = 2.2, int iterationNum = 10, const Real& cycle = 128, const Real& heightScale = 4);

    /// Define terrain
    void define( Ogre::TerrainGroup* terrainGroup, long x, long y ) override;
    /// Random terrain origin point to make it looks different
    void randomize();
private:
    /** Produce a noise according to PerlinNoise algorithm
        Generate multiple values by calling noise():
            v0=noise(x,y),
            v1=noise(x*beta,y*beta),
            v2=noise(x*beta*beta,y*beta*beta)
            ...
        Accumulate them:
            result = v0/alpha + v1/(alpha*alpha) + v1/(alpha*alpha*alpha) + ...
    */
    Real produceSingleHeight(const Vector2& vec2);

    /// Generate a 2D noise
    Real noise(const Vector2& vec2);

    inline Real sCurve(const Real& t) const { return t * t * (3 - 2 * t); }
    inline Real lerp(const Real& t, const Real& a, const Real& b) const { return a + t*(b - a); }
    void setup( const Real& target, int& b0, int& b1, Real& r0, Real& r1);

    Real mAlpha;
    Real mBeta;
    int mIterationNum;
    Real mCycle;
    Real mHeightScale;
    Vector2 mOriginPoint;

    int p[B + B + 2];
    Vector3 g3[B + B + 2];
    Vector2 g2[B + B + 2];
    Real g1[B + B + 2];
};
