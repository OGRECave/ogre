Texture2D Image;
Texture3D Rand;
Texture3D Noise;

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 OldTV_ps(float4 Pos : SV_Position, float2 texCoord: TEXCOORD0, float2 texCoord2: TEXCOORD1, 
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
   float f = (1 - texCoord2.x * texCoord2.x) * (1 - texCoord2.y * texCoord2.y);
   float frame = saturate(frameSharpness * (pow(f, frameShape) - frameLimit));

   // Interference ... just a texture filled with rand()
   float4 rand = Rand.Sample(g_samLinear, float3(1.5 * texCoord2, time_0_X));
   rand -= float4(0.2,0.2,0.2,0.2);

   // Some signed noise for the distortion effect
   float4 noisy = Noise.Sample(g_samLinear, float3(0, 0.5 * texCoord2.y, 0.1 * time_0_X));
   noisy -= float4(0.5,0.5,0.5,0.5);

   // Repeat a 1 - x^2 (0 < x < 1) curve and roll it with sinus.
   float dst = frac(texCoord2.y * distortionFreq + distortionRoll * sin_time_0_X);
   dst *= (1 - dst);
   // Make sure distortion is highest in the center of the image
   dst /= 1 + distortionScale * abs(texCoord2.y);

   // ... and finally distort
   texCoord.x += distortionScale * noisy.x * dst;
   float4 image = Image.Sample(g_samLinear, float2(texCoord.x,texCoord.y));

   // Combine frame, distorted image and interference
   return frame * (inerference * rand.x + image);
}