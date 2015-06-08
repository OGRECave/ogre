Texture2D<float3> RT		: register(t0);
Texture2D<float> pattern	: register(t1);
SamplerState samplerState[2]: register(s0);

float4 main( float2 TexCoord : TEXCOORD0 ) : SV_Target
{	
	float c = dot( RT.Sample( samplerState[0], TexCoord ).xyz, float3( 0.30, 0.11, 0.59 ) );
	float n = pattern.Sample( samplerState[1], TexCoord ).x * 2.0 - 1.0;
	c += n;
	if (c > 0.5)
	{
		c = 0.0;
	}
	else
	{
		c = 1.0;
	}
	return float4( c.xxx, 1.0 );
}
