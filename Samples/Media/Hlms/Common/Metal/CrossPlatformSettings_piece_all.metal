@piece( SetCrossPlatformSettings )
#include <metal_stdlib>
using namespace metal;

struct float1
{
	float x;
	float1() {}
	float1( float _x ) : x( _x ) {}
};

#define mul( x, y ) ((x) * (y))
#define lerp mix
#define INLINE inline

#define finalDrawId drawId

#define outVs_Position outVs.gl_Position
#define OGRE_SampleLevel( tex, sampler, uv, lod ) tex.sample( sampler, float2( uv ), level( lod ) )
#define OGRE_SampleArray2DLevel( tex, sampler, uv, arrayIdx, lod ) tex.sample( tex, sampler, float2( uv ), ushort( arrayIdx ), level( lod ) )
@end
