//-------------------------------
//BrightBloom_ps40.hlsl
// High-pass filter for obtaining lumminance
// We use an aproximation formula that is pretty fast:
//   f(x) = ( -3 * ( x - 1 )^2 + 1 ) * 2
//   Color += f(Color)
//-------------------------------

Texture2D g_RT				: register(t0);
SamplerState samplerState	: register(s0);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main( PS_INPUT inPs ) : SV_Target
{
	float4 tex = g_RT.Sample( samplerState, inPs.uv0 );

	tex -= 1;
	float4 bright4= -6 * tex * tex + 2; //float4 bright4= ( -3 * tex * tex + 1 ) * 2;
	float bright = dot( bright4.xyx, float3( 0.333333, 0.333333, 0.333333 ) );
	tex += bright + 0.6;

	return tex;
}
