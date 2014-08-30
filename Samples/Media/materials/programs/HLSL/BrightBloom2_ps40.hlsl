//-------------------------------
//BrightBloom_ps40.hlsl
// High-pass filter for obtaining lumminance
// We use an aproximation formula that is pretty fast:
//   f(x) = ( -3 * ( x - 1 )^2 + 1 ) * 2
//   Color += f(Color)
//-------------------------------

Texture2D g_RT;          // Color texture for mesh

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 main( float4 pos : SV_Position, float2 texCoord  : TEXCOORD0) : SV_Target {

	float4 tex = g_RT.Sample( g_samLinear, texCoord );

	tex -= 1;
	float4 bright4= -6 * tex * tex + 2; //float4 bright4= ( -3 * tex * tex + 1 ) * 2;
	float bright = dot( bright4, float4( 0.333333, 0.333333, 0.333333, 0) );
	tex += bright + 0.6;

	return tex;
}
