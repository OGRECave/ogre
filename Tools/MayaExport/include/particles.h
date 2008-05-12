/****************************************************************
 Thanks to Bandures for the particle exporter
 ****************************************************************/

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#ifndef _PARTICLES_H
#define _PARTICLES_H

#include <math.h>
#include <vector>
#ifdef MAC_PLUGIN
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#include <maya/MDagPath.h>
#include "paramList.h"
#include "mayaExportLayer.h"
#pragma warning(disable: 4996)

namespace OgreMayaExporter
{
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float fabs( float fVal ) { return ::fabs( fVal ); }
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPos
{
	float x;
	float y;
	float z;

	SPos(): x(0), y(0), z(0) {}
	SPos( float _x, float _y, float _z ): x(_x), y(_y), z(_z) {}
};
inline const SPos operator-( const SPos &in ) { return SPos( -in.x, -in.y, -in.z ); }
inline const SPos operator+( const SPos &in1, const SPos &in2 ) { return SPos( in1.x + in2.x, in1.y + in2.y, in1.z + in2.z ); }
inline const SPos operator-( const SPos &in1, const SPos &in2 ) { return SPos( in1.x - in2.x, in1.y - in2.y, in1.z - in2.z ); }
inline float fabs2( const SPos &in ) { return in.x * in.x + in.y * in.y + in.z * in.z; }
inline float fabs( const SPos &in ) { return float( sqrt( fabs2( in ) ) ); }
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SColor
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		struct
		{
			float r, g, b, a;
		};
	};

	SColor(): r(0), g(0), b(0), a(0) {}
	SColor( float _r, float _g, float _b, float _a ): r(_r), g(_g), b(_b), a(_a) {}
};
inline const SColor operator-( const SColor &in1) { return SColor( -in1.x, -in1.y, -in1.z, -in1.w ); }
inline const SColor operator+( const SColor &in1, const SColor &in2 ) { return SColor( in1.x + in2.x, in1.y + in2.y, in1.z + in2.z, in1.w + in2.w ); }
inline const SColor operator-( const SColor &in1, const SColor &in2 ) { return SColor( in1.x - in2.x, in1.y - in2.y, in1.z - in2.z, in1.w - in2.w ); }
inline float fabs2( const SColor &in ) { return in.x * in.x + in.y * in.y + in.z * in.z + in.w * in.w; }
inline float fabs( const SColor &in ) { return float( sqrt( fabs2( in ) ) ); }
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SScale
{
	float x;
	float y;

	SScale(): x(0), y(0) {}
	SScale( float _x, float _y ): x(_x), y(_y) {}
};
inline const SScale operator+( const SScale &in1, const SScale &in2 ) { return SScale( in1.x + in2.x, in1.y + in2.y ); }
inline const SScale operator-( const SScale &in1, const SScale &in2 ) { return SScale( in1.x - in2.x, in1.y - in2.y ); }
inline float fabs2( const SScale &in ) { return in.x * in.x + in.y * in.y; }
inline float fabs( const SScale &in ) { return float( sqrt( fabs2( in ) ) ); }
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticleData
{
	int nFrame;
	int nSprite;
	SPos pos;
	SColor color;
	SScale scale;
	float fRotation;
	////
	SParticleData(): nFrame( 0 ), nSprite( 0 ), pos( 0, 0, 0 ), color( 1, 1, 1, 1 ), scale( 1, 1 ), fRotation( 0 ) {}
};
typedef std::vector<SParticleData> CParticlesTrack;
#ifdef MAC_PLUGIN
typedef __gnu_cxx::hash_map<int, CParticlesTrack> CParticlesData;
#else
typedef stdext::hash_map<int, CParticlesTrack> CParticlesData;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline void Interpolate( const T &v1, const T &v2, float fCoeff, T *pRes )
{
	pRes->Interpolate( v1, v2, fCoeff );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Interpolate( const int &v1, const int &v2, float fCoeff, int *pRes )
{
	*pRes = v1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Interpolate( const float &v1, const float &v2, float fCoeff, float *pRes )
{
	*pRes = ( 1 - fCoeff ) * v1 + fCoeff * v2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Interpolate( const SPos &v1, const SPos &v2, float fCoeff, SPos *pRes )
{
	Interpolate( v1.x, v2.x, fCoeff, &pRes->x );
	Interpolate( v1.y, v2.y, fCoeff, &pRes->y );
	Interpolate( v1.z, v2.z, fCoeff, &pRes->z );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Interpolate( const SColor &v1, const SColor &v2, float fCoeff, SColor *pRes )
{
	Interpolate( v1.r, v2.r, fCoeff, &pRes->r );
	Interpolate( v1.g, v2.g, fCoeff, &pRes->g );
	Interpolate( v1.b, v2.b, fCoeff, &pRes->b );
	Interpolate( v1.a, v2.a, fCoeff, &pRes->a );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Interpolate( const SScale &v1, const SScale &v2, float fCoeff, SScale *pRes )
{
	Interpolate( v1.x, v2.x, fCoeff, &pRes->x );
	Interpolate( v1.y, v2.y, fCoeff, &pRes->y );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class TKey
{
public:
	T value;
	int nTime;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class TKeyTrack
{
public:
	std::vector<TKey<T> > keys;

protected:
	void GetValueBinSearch( float fTime, T *pRes ) const
	{
		int nLeft = 0, nRight = keys.size() - 1;
		int nTime = int( fTime - 0.5f );
		while( nLeft - nRight > 1 )
		{
			int nTemp = ( nLeft + nRight ) / 2;
			if ( keys[nTemp].nTime <= nTime )
				nLeft = nTemp;
			else
				nRight = nTemp;
		}
		////
		const TKey<T> &end = keys[nRight];
		const TKey<T> &start = keys[nLeft];
		float fCoeff = ( fTime - start.nTime ) / ( end.nTime - start.nTime );
		Interpolate( start.value, end.value, fCoeff, pRes );
	}

public:
	void GetValue( float fTime, T *pRes ) const
	{
		if ( keys.size() == 1 )
			*pRes = keys[0].value;
		else
			GetValueBinSearch( fTime, pRes );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticle
{
	int nEndTime;
	int nStartTime;
	TKeyTrack<int> sprite;
	TKeyTrack<SPos> pos;
	TKeyTrack<SColor> color;
	TKeyTrack<SScale> scale;
	TKeyTrack<float> rotation;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Particles
////////////////////////////////////////////////////////////////////////////////////////////////////
class Particles
{
private:
	CParticlesData data;
	////
	int nFrames;
	std::vector<SParticle> particleTracks;

protected:
	MStatus ExportFrame( MDagPath &dagPath, int nFrame );
	MStatus FinalizeData( int nMinFrame, int nMaxFrame );

public:
	Particles();
	virtual ~Particles();

	MStatus load( MDagPath& dagPath, ParamList& params );
	MStatus writeToXML( ParamList& params );
	void clear();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}; // end of namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif