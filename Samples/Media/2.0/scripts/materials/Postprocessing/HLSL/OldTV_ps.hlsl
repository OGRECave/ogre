Texture2D Image	: register(t0);
Texture3D Rand	: register(t1);
Texture3D Noise	: register(t2);

SamplerState samplerState		: register(s0);
SamplerState samplerStateRand	: register(s1);
SamplerState samplerState3D		: register(s2);

float4 main
(
	float2 uv0 : TEXCOORD0,
	uniform float distortionFreq	: register(c3),
	uniform float distortionScale	: register(c4),
	uniform float distortionRoll	: register(c5),
	uniform float interference		: register(c7),
	uniform float frameLimit		: register(c8),
	uniform float frameShape		: register(c0),
	uniform float frameSharpness	: register(c1),
	uniform float time_0_X			: register(c2),
	uniform float sin_time_0_X		: register(c6)

) : SV_Target
{
   // Define a frame shape
   float2 pos = abs((uv0 - 0.5) * 2.0);
   float f = (1 - pos.x * pos.x) * (1 - pos.y * pos.y);
   float frame = saturate(frameSharpness * (pow(abs(f), frameShape) - frameLimit));

   // Interference ... just a texture filled with rand()
   float4 rand = Rand.Sample(samplerStateRand, float3(1.5 * uv0.xy * 2.0, time_0_X));
   rand -= float4(0.2,0.2,0.2,0.2);

   // Some signed noise for the distortion effect
   float4 noisy = Noise.Sample(samplerState3D, float3(0, 0.5 * pos.y, 0.1 * time_0_X));
   noisy -= float4(0.5,0.5,0.5,0.5);

   // Repeat a 1 - x^2 (0 < x < 1) curve and roll it with sinus.
   float dst = frac(pos.y * distortionFreq + distortionRoll * sin_time_0_X);
   dst *= (1 - dst);
   // Make sure distortion is highest in the center of the image
   dst /= 1 + distortionScale * abs(pos.y);

   // ... and finally distort
   uv0.x += distortionScale * noisy.x * dst;
   float4 image = Image.Sample(samplerState, uv0);

   // Combine frame, distorted image and interference
   return frame * (interference * rand.x + image);
}
