Texture2D Image;
Texture3D Rand;
Texture3D Noise;

SamplerState g_samVolume
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap; 
};

struct v3p
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
};

float4 OldTV_ps(v3p input, 
    uniform float distortionFreq: register(c3),
    uniform float distortionScale: register(c4),
    uniform float distortionRoll: register(c5),
    uniform float inerference: register(c7),
    uniform float frameLimit: register(c8),
    uniform float frameShape: register(c0),
    uniform float frameSharpness: register(c1),
    uniform float time_0_X: register(c2),
    uniform float sin_time_0_X: register(c6)

) : SV_Target {
   // Define a frame shape
   float f = (1 - input.texCoord2.x * input.texCoord2.x) * (1 - input.texCoord2.y * input.texCoord2.y);
   float frame = saturate(frameSharpness * (pow(f, frameShape) - frameLimit));

   // Interference ... just a texture filled with rand()
   float4 rand = Rand.Sample(g_samVolume, float3(1.5 * input.texCoord2, time_0_X));
   rand -= float4(0.2,0.2,0.2,0.2);

   // Some signed noise for the distortion effect
   float4 noisy = Noise.Sample(g_samVolume, float3(0, 0.5 * input.texCoord2.y, 0.1 * time_0_X));
   noisy -= float4(0.5,0.5,0.5,0.5);

   // Repeat a 1 - x^2 (0 < x < 1) curve and roll it with sinus.
   float dst = frac(input.texCoord2.y * distortionFreq + distortionRoll * sin_time_0_X);
   dst *= (1 - dst);
   // Make sure distortion is highest in the center of the image
   dst /= 1 + distortionScale * abs(input.texCoord2.y);

   // ... and finally distort
   input.texCoord.x += distortionScale * noisy.x * dst;
   float4 image = Image.Sample(g_samVolume, input.texCoord);

   // Combine frame, distorted image and interference
   return frame * (inerference * rand + image);
}