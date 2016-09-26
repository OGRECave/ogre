
#define FACE_POS_X 0
#define FACE_NEG_X 1
#define FACE_POS_Y 2
#define FACE_NEG_Y 3
#define FACE_POS_Z 4
#define FACE_NEG_Z 5

TextureCube<float4>		cubeTexture0	: register(t0);
TextureCube<float4>		cubeTexture1	: register(t1);
TextureCube<float4>		cubeTexture2	: register(t2);
TextureCube<float4>		cubeTexture3	: register(t3);
SamplerState			samplerState	: register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

// Get direction from cube texel for a given face. x and y are in the [-1, 1] range.
float3 getCubeDir( float2 uv )
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

float4 main
(
	PS_INPUT inPs,
	uniform float4 packed3x3Mat[7],
	uniform float4 weights,
	uniform float4 lodLevel
) : SV_Target0
{
	float3 vOrigDir = getCubeDir( inPs.uv0.xy * 2.0 - 1.0 );
	float3 vDir;

	float4 fAvg;
	float3x3 rotMat;

	vDir = vOrigDir;
	fAvg  = cubeTexture0.SampleLevel( samplerState, vDir, lodLevel.x ).xyzw * weights.x;

	rotMat = float3x3( packed3x3Mat[0].x, packed3x3Mat[0].y, packed3x3Mat[0].z,
					   packed3x3Mat[0].w, packed3x3Mat[1].x, packed3x3Mat[1].y,
					   packed3x3Mat[1].z, packed3x3Mat[1].w, packed3x3Mat[2].x );
	vDir = mul( rotMat, vOrigDir );
	fAvg += cubeTexture1.SampleLevel( samplerState, vDir, lodLevel.y ).xyzw * weights.y;

	rotMat = float3x3( packed3x3Mat[2].y, packed3x3Mat[2].z, packed3x3Mat[2].w,
					   packed3x3Mat[3].x, packed3x3Mat[3].y, packed3x3Mat[3].z,
					   packed3x3Mat[3].w, packed3x3Mat[4].x, packed3x3Mat[4].y );
	vDir = mul( rotMat, vOrigDir );
	fAvg += cubeTexture2.SampleLevel( samplerState, vDir, lodLevel.z ).xyzw * weights.z;

	rotMat = float3x3( packed3x3Mat[4].z, packed3x3Mat[4].w, packed3x3Mat[5].x,
					   packed3x3Mat[5].y, packed3x3Mat[5].z, packed3x3Mat[5].w,
					   packed3x3Mat[6].x, packed3x3Mat[6].y, packed3x3Mat[6].z );
	vDir = mul( rotMat, vOrigDir );
	fAvg += cubeTexture3.SampleLevel( samplerState, vDir, lodLevel.w ).xyzw * weights.w;

	return fAvg;
}
