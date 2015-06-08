Texture2D<float3> RT		: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target
{
    float4 Color;
    Color.a = 1.0f;
    Color.xyz = 0.5f;
	Color.xyz -= RT.Sample( samplerState, uv0 - 0.001)*2.0f;
	Color.xyz += RT.Sample( samplerState, uv0 + 0.001)*2.0f;
    Color.xyz = (Color.r+Color.g+Color.b)/3.0f;
    return Color;
}
