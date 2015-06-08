Texture2D<float3> RT;
SamplerState samplerState;

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target
{
    float4 Color;
    Color.a = 1.0f;
    Color.rgb = 0.5f;
	Color -= RT.Sample( RT, uv0 - 0.001)*2.0f;
	Color += RT.Sample( RT, uv0 + 0.001)*2.0f;
    Color.rgb = (Color.r+Color.g+Color.b)/3.0f;
    return Color;
}
