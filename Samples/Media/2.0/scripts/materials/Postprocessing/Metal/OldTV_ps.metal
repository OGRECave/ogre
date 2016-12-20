#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float distortionFreq;
	float distortionScale;
	float distortionRoll;
	float interference;
	float frameLimit;
	float frameShape;
	float frameSharpness;
	float time_0_X;
	float sin_time_0_X;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	Image			[[texture(0)]],
	texture3d<float>	Rand			[[texture(1)]],
	texture3d<float>	Noise			[[texture(2)]],
	sampler				samplerState	[[sampler(0)]],
	sampler				samplerStateRand[[sampler(1)]],
	sampler				samplerState3D	[[sampler(2)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
   // Define a frame shape
   float2 pos = abs((inPs.uv0 - 0.5) * 2.0);
   float f = (1 - pos.x * pos.x) * (1 - pos.y * pos.y);
   float frame = saturate(p.frameSharpness * (pow(abs(f), p.frameShape) - p.frameLimit));

   // Interference ... just a texture filled with rand()
   float4 rand = Rand.sample(samplerStateRand, float3(1.5 * inPs.uv0.xy * 2.0, p.time_0_X));
   rand -= float4(0.2,0.2,0.2,0.2);

   // Some signed noise for the distortion effect
   float4 noisy = Noise.sample(samplerState3D, float3(0, 0.5 * pos.y, 0.1 * p.time_0_X));
   noisy -= float4(0.5,0.5,0.5,0.5);

   // Repeat a 1 - x^2 (0 < x < 1) curve and roll it with sinus.
   float dst = fract(pos.y * p.distortionFreq + p.distortionRoll * p.sin_time_0_X);
   dst *= (1 - dst);
   // Make sure distortion is highest in the center of the image
   dst /= 1 + p.distortionScale * abs(pos.y);

   // ... and finally distort
   inPs.uv0.x += p.distortionScale * noisy.x * dst;
   float4 image = Image.sample(samplerState, inPs.uv0);

   // Combine frame, distorted image and interference
   return frame * (p.interference * rand.x + image);
}
