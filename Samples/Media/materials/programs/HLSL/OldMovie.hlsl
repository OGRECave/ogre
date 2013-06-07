Texture2D RT : register(s0);
Texture2D SplotchesTx : register(s1);
Texture1D Texture2 : register(s2);
Texture1D SepiaTx : register(s3);

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float2 calcSpriteAddr(float2 texCoord, float DirtFrequency1, float period)
{
   return texCoord + Texture2.Sample(g_samLinear, period  * DirtFrequency1).xy;
}

float4 getSplotches(float2 spriteAddr)
{
   // get sprite address into paged texture coords space
   spriteAddr = spriteAddr / 6.3f;
   //spriteAddr = spriteAddr - frac(spriteAddr);
   spriteAddr = spriteAddr - (spriteAddr /33.3f);

   return float4(1,1,1,1) * SplotchesTx.Sample(g_samLinear, spriteAddr).r;
}

float4 OldMovie_ps (float4 pos : SV_Position, 
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
   float4 splotches = getSplotches(spriteAddr);
   float4 specs = float4(1.0f,1.0f,1.0f,1.0f) - getSplotches(spriteAddr / 3.0f);

   // convert color to base luminance
   float4 base = RT.Sample(g_samLinear, texCoord + float2(0, spriteAddr.y * frameJitter));
   float lumi = dot(base.rgb, luminance);
   // randomly shift luminance
   lumi -= spriteAddr.x * lumiShift;
   // tone map luminance
   //float3 sepia = SepiaTx.Sample(g_samLinear,lumi).rgb;
   float4 sepia = SepiaTx.Sample(g_samLinear,lumi);
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