Texture2D RT				: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0, uniform float2 vTexelSize ) : SV_Target
{
	float2 usedTexelED[8] =
	{
		-1, -1,
		 0, -1,
		 1, -1,
		-1,  0,
		 1,  0,
		-1,  1,
		 0,  1,
	     1,  1,
	};

	float4 cAvgColor = 9 * RT.Sample( samplerState, uv0 );

	for(int t=0; t<8; t++)
		cAvgColor -= RT.Sample( samplerState, uv0 + vTexelSize * usedTexelED[t] );

	return cAvgColor;
}
