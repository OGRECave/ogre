#include <metal_stdlib>
using namespace metal;

#define FACE_POS_X 0
#define FACE_NEG_X 1
#define FACE_POS_Y 2
#define FACE_NEG_Y 3
#define FACE_POS_Z 4
#define FACE_NEG_Z 5

struct PS_INPUT
{
	float2 uv0;
};

// Get direction from cube texel for a given face. x and y are in the [-1, 1] range.
inline float3 getCubeDir( float2 uv )
{
	float x = uv.x;
	float y = uv.y;

	// Set direction according to face.
	// Note : No need to normalize as we sample in a cubemap
#if FACEIDX == FACE_POS_X
	return float3( 1.0, -y, -x );
#elif FACEIDX == FACE_NEG_X
	return float3( -1.0, -y, x );
#elif FACEIDX == FACE_POS_Y
	return float3( x, 1.0, y );
#elif FACEIDX == FACE_NEG_Y
	return float3( x, -1.0, -y );
#elif FACEIDX == FACE_POS_Z
	return float3( x, -y, 1.0 );
#elif FACEIDX == FACE_NEG_Z
	return float3( -x, -y, -1.0 );
#endif
}

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texturecube<float>	cubeTexture		[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],
	constant float &lodLevel	[[buffer(PARAMETER_SLOT)]]
)
{
	float3 vDir = getCubeDir( inPs.uv0.xy * 2.0 - 1.0 );
	return cubeTexture.sample( samplerState, vDir, level( lodLevel ) ).xyzw;
}
