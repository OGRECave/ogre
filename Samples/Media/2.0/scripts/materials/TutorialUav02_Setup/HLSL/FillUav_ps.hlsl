RWTexture2D<float4> testTexture			: register(u1);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

void main( PS_INPUT inPs, float4 gl_FragCoord : SV_Position )
{
	int2 fragPos = int2( gl_FragCoord.xy );
	testTexture[fragPos].xyzw = float4( inPs.uv0.xy, 0.0f, 1.0f );
}
