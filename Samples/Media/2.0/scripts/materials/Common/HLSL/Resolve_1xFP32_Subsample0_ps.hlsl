
Texture2DMS<float>	myTexture : register(t0);

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

float main
(
	PS_INPUT inPs,
	float4 gl_FragCoord : SV_Position
) : SV_Target
{
	return myTexture.Load( int2( gl_FragCoord.xy ), 0 ).x;
}
