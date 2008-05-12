//-------------------------------
//BrightBloom_ps20.hlsl
// High-pass filter for obtaining lumminance
// We use an aproximation formula that is pretty fast:
//   f(x) = ( -3 * ( x - 1 )^2 + 1 ) * 2
//   Color += f(Color)
//-------------------------------

float4 main(float2 texCoord: TEXCOORD0,
		uniform sampler RT: register(s0)
		) : COLOR {
	float4 tex = tex2D(RT,   texCoord);

	tex -= 1;
	float4 bright4= -6 * tex * tex + 2; //float4 bright4= ( -3 * tex * tex + 1 ) * 2;
	float bright = dot( bright4, float4( 0.333333, 0.333333, 0.333333, 0) );
	tex += bright + 0.6;

	return tex;
}
