#include <stdlib.h>
#include <math.h>
#include "OgreMath.h"
#include "PerlinNoiseTerrainGenerator.h"

using namespace Ogre;

#define BM 0xff
#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

PerlinNoiseTerrainGenerator::PerlinNoiseTerrainGenerator(const Real& alpha, const Real& beta, int iterationNum, const Real& cycle, const Real& heightScale)
	: mAlpha(alpha)
	, mBeta(beta)
	, mIterationNum(iterationNum)
	, mCycle(cycle)
	, mHeightScale(heightScale)
	, mOriginPoint(0.f)
{
	for (int i = 0 ; i < B ; i++) {
		p[i] = i;
		g1[i] = Math::SymmetricRandom();

		g2[i] = Vector2(Math::SymmetricRandom(), Math::SymmetricRandom());
		g2[i].normalise();

		g3[i] = Vector3(Math::SymmetricRandom(), Math::SymmetricRandom(), Math::SymmetricRandom());
		g3[i].normalise();
	}

	for (int i = 0 ; i < B ; i++) {
		int j = (int) Math::RangeRandom (0,B);

		int k = p[i];
		p[i] = p[j];
		p[j] = k;
	}

	for (int i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		g2[B + i] = g2[i];
		g3[B + i] = g3[i];
	}
}

void PerlinNoiseTerrainGenerator::define( TerrainGroup* terrainGroup, long x, long y )
{
	uint16 terrainSize = terrainGroup->getTerrainSize();
	float* heightMap = OGRE_ALLOC_T(float, terrainSize*terrainSize, MEMCATEGORY_GEOMETRY);

	Vector2 worldOffset( Real(x*(terrainSize-1)), Real(y*(terrainSize-1)) );
	worldOffset += mOriginPoint;

	Vector2 revisedValuePoint;
	for( uint16 i=0; i<terrainSize; i++ )
		for( uint16 j=0; j<terrainSize; j++ )
		{
			revisedValuePoint = (worldOffset + Vector2(j,i)) / mCycle;
			heightMap[i*terrainSize + j] = produceSingleHeight( revisedValuePoint ) * mHeightScale;
		}
	terrainGroup->defineTerrain(x,y,heightMap);
	OGRE_FREE(heightMap, MEMCATEGORY_GEOMETRY);
}

Real PerlinNoiseTerrainGenerator::produceSingleHeight(const Vector2& vec2)
{
	Vector2 tempVec(vec2);
	Real sum = 0;
	Real scale = 1;

	for (int i=0;i<mIterationNum;i++)
	{
		sum += noise(tempVec) / scale;
		scale *= mAlpha;
		tempVec *= mBeta;
	}
	return sum;
}

Real PerlinNoiseTerrainGenerator::noise(const Vector2& vec2)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	Real rx0, rx1, ry0, ry1, sx, sy, a, b, u, v;

	setup(vec2.x, bx0,bx1, rx0,rx1);
	setup(vec2.y, by0,by1, ry0,ry1);

	int i = p[ bx0 ];
	int j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = sCurve(rx0);
	sy = sCurve(ry0);

	u = g2[b00].dotProduct( Vector2(rx0,ry0) );
	v = g2[b10].dotProduct( Vector2(rx1,ry0) );
	a = lerp(sx, u, v);

	u = g2[b01].dotProduct( Vector2(rx0,ry1) );
	v = g2[b11].dotProduct( Vector2(rx1,ry1) );
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

void PerlinNoiseTerrainGenerator::setup( const Real& target, int& b0, int& b1, Real& r0, Real& r1)
{
	Real t = target + N;
	b0 = ((int)t) & BM;
	b1 = (b0+1) & BM;
	r0 = t - (int)t;
	r1 = r0 - 1;
}

void PerlinNoiseTerrainGenerator:: randomize()
{
	mOriginPoint.x = Math::RangeRandom( -1000, 1000 );
	mOriginPoint.y = Math::RangeRandom( -1000, 1000 );
}