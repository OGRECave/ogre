struct VS_INPUT
{
	float4 vertex	: POSITION;
};

struct PS_INPUT
{
	float3 posLS		: TEXCOORD0;
	float4 gl_Position	: SV_Position;
};


PS_INPUT main
(
	VS_INPUT input,
	uniform float4x4 worldViewProj,
	uniform float4x3 worldScaledMatrix,
	uniform float3 probeCameraPosScaled
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= mul( worldViewProj, float4( input.vertex.xyz, 1.0 ) ).xyzw;
	outVs.posLS			= mul( float4( input.vertex.xyz, 1.0 ), worldScaledMatrix ).xyz;
	outVs.posLS			= outVs.posLS - probeCameraPosScaled;
	outVs.posLS.z		= -outVs.posLS.z; //Left handed

	return outVs;
}
