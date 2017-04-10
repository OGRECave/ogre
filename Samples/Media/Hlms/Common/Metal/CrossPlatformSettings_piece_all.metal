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
#define INLINE inline

#define outVs_Position outVs.gl_Position
@end
