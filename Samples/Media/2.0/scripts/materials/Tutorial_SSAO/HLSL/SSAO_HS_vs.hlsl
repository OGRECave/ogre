
struct VS_INPUT
{
	float4 vertex	: POSITION;
	float3 normal	: NORMAL;
	float2 uv0		: TEXCOORD0;
};

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float3 cameraDir	: TEXCOORD1;

	float4 gl_Position	: SV_Position;
};

PS_INPUT main
(
	VS_INPUT input,

	uniform matrix worldViewProj
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= mul( worldViewProj, input.vertex ).xyzw;
	outVs.uv0			= input.uv0;
	outVs.cameraDir		= input.normal;

	return outVs;
}
