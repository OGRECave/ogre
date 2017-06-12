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

#define outVs_Position outVs.gl_Position
#define OGRE_SampleLevel( tex, sampler, uv, lod ) tex.sample( sampler, float2( uv ), level( lod ) )
@end
