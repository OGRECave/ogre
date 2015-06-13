Texture2D RT				: register(t0);
Texture2D Sum				: register(t1);
SamplerState samplerState	: register(s0);

float4 main
(
	float2 texCoord	: TEXCOORD0,
    uniform float blur
) : SV_Target
{
   float4 render = RT.Sample( samplerState, texCoord );
   float4 sum = Sum.Sample( samplerState, texCoord );

   return lerp( render, sum, blur );
}
