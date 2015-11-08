RWTexture2D<uint> testTexture			: register(u1);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

void main( PS_INPUT inPs, float4 gl_FragCoord : SV_Position )
{
	uint2 rg = uint2( inPs.uv0.xy * 255.0f );

	int2 fragPos = int2( gl_FragCoord.xy );
	testTexture[fragPos] = (rg.x << 24u) | (rg.y << 16u) | (0u << 8u) | 1u;
}
