
struct VS_INPUT
{
	float4 vertex	: POSITION;
	float3 normal	: NORMAL;
};

struct PS_INPUT
{
	float3 cameraDir	: TEXCOORD0;

	float4 gl_Position	: SV_Position;
};

PS_INPUT main
(
	VS_INPUT input,

	uniform matrix worldViewProj
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= mul( worldViewProj, input.vertex ).xyww;
	outVs.cameraDir		= input.normal;

	return outVs;
}
