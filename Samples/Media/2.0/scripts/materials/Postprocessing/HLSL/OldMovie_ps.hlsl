Texture2D RT			: register(t0);
Texture2D SplotchesTx	: register(t1);
Texture1D Texture2		: register(t2);
Texture1D SepiaTx		: register(t3);

SamplerState samplerState[4] : register(s0);

float2 calcSpriteAddr(float2 texCoord, float DirtFrequency1, float period)
{
   return texCoord + Texture2.Sample(samplerState[2], period  * DirtFrequency1).xy;
}

float getSplotches(float2 spriteAddr)
{
   // get sprite address into paged texture coords space
   spriteAddr = spriteAddr / 6.3f;
   //spriteAddr = spriteAddr - frac(spriteAddr);
   spriteAddr = spriteAddr - (spriteAddr /33.3f);

   return SplotchesTx.Sample(samplerState[1], spriteAddr).x;
}

float4 main
(
	float2 texCoord  : TEXCOORD0,
	uniform float time_cycle_period,
	uniform float flicker,
	uniform float DirtFrequency,
	uniform float3 luminance,
	uniform float frameJitter,
	uniform float lumiShift
) : SV_Target
{
   // get sprite address
   float2 spriteAddr = calcSpriteAddr(texCoord, DirtFrequency, time_cycle_period);

    // add some dark and light splotches to the film
   float splotches = getSplotches(spriteAddr);
   float specs = 1.0f - getSplotches(spriteAddr / 3.0f);

   // convert color to base luminance
   float4 base = RT.Sample(samplerState[0], texCoord + float2(0, spriteAddr.y * frameJitter));
   float lumi = dot(base.rgb, luminance);
   // randomly shift luminance
   lumi -= spriteAddr.x * lumiShift;
   // tone map luminance
   //float3 sepia = SepiaTx.Sample(samplerState[3],lumi).rgb;
   float4 sepia = SepiaTx.Sample(samplerState[3],lumi);
   base.rgb = sepia.rgb;
   // calc flicker speed
   float darken = frac(flicker * time_cycle_period);

   // we want darken to cycle between 0.6 and 1.0
   darken = abs(darken - 0.5f) * 0.4f + 0.6f;
   // composite dirt onto film
   float4 color = base * splotches * darken + specs;
   //color.y = color.x;
   //color.z = color.x;
   return color.rgba;
}
