Texture2D Image				: register(t0);
SamplerState samplerState	: register(s0);

// The Laplace filter approximates the second order derivate,
// that is, the rate of change of slope in the image. It can be
// used for edge detection. The Laplace filter gives negative
// response on the higher side of the edge and positive response
// on the lower side.

// This is the filter kernel:
// 0  1  0
// 1 -4  1
// 0  1  0

float4 main
(
	float2 uv0 : TEXCOORD0,
	uniform float scale,
	uniform float pixelSize
) : SV_Target
{

	float2 samples[4] =
	{
		0, -1,
	   -1,  0,
		1,  0,
		0,  1
	};

	float4 laplace = -4 * Image.Sample( samplerState, uv0 );

   // Sample the neighbor pixels
   for (int i = 0; i < 4; i++)
	  laplace += Image.Sample( samplerState, uv0 + pixelSize * samples[i] );

   return (0.5 + scale * laplace);
}

