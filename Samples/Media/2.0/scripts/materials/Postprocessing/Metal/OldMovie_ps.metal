#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float time_cycle_period;
	float flicker;
	float DirtFrequency;
	float3 luminance;
	float frameJitter;
	float lumiShift;
};

inline float2 calcSpriteAddr( float2 uv0, float DirtFrequency1, float period,
							  texture1d<float> Texture2, sampler samplerState2 )
{
   return uv0 + Texture2.sample(samplerState2, period  * DirtFrequency1).xy;
}

inline float getSplotches( float2 spriteAddr, texture2d<float> SplotchesTx, sampler samplerState1 )
{
   // get sprite address into paged texture coords space
   spriteAddr = spriteAddr / 6.3f;
   //spriteAddr = spriteAddr - frac(spriteAddr);
   spriteAddr = spriteAddr - (spriteAddr /33.3f);

   return SplotchesTx.sample(samplerState1, spriteAddr).x;
}

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	texture2d<float>	SplotchesTx		[[texture(1)]],
	texture1d<float>	Texture2		[[texture(2)]],
	texture1d<float>	SepiaTx			[[texture(3)]],
	sampler				samplerState0	[[sampler(0)]],
	sampler				samplerState1	[[sampler(1)]],
	sampler				samplerState2	[[sampler(2)]],
	sampler				samplerState3	[[sampler(3)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
   // get sprite address
   float2 spriteAddr = calcSpriteAddr(inPs.uv0, p.DirtFrequency, p.time_cycle_period,
									  Texture2, samplerState2);

    // add some dark and light splotches to the film
   float splotches = getSplotches(spriteAddr, SplotchesTx, samplerState1);
   float specs = 1.0f - getSplotches(spriteAddr / 3.0f, SplotchesTx, samplerState1);

   // convert color to base luminance
   float4 base = RT.sample(samplerState0, inPs.uv0 + float2(0, spriteAddr.y * p.frameJitter));
   float lumi = dot(base.rgb, p.luminance);
   // randomly shift luminance
   lumi -= spriteAddr.x * p.lumiShift;
   // tone map luminance
   //float3 sepia = SepiaTx.sample(samplerState3,lumi).rgb;
   float4 sepia = SepiaTx.sample(samplerState3,lumi);
   base.rgb = sepia.rgb;
   // calc flicker speed
   float darken = fract(p.flicker * p.time_cycle_period);

   // we want darken to cycle between 0.6 and 1.0
   darken = abs(darken - 0.5f) * 0.4f + 0.6f;
   // composite dirt onto film
   float4 color = base * splotches * darken + specs;
   //color.y = color.x;
   //color.z = color.x;
   return color.rgba;
}
