#version 330

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform sampler2D RT;
uniform sampler2D SplotchesTx;
uniform sampler1D Texture2;
uniform sampler1D SepiaTx;

uniform float time_cycle_period;
uniform float flicker;
uniform float DirtFrequency;
uniform vec3 luminance;
uniform float frameJitter;
uniform float lumiShift;

vec2 calcSpriteAddr(vec2 texCoord, float DirtFrequency1, float period)
{
   return texCoord + texture(Texture2, period * DirtFrequency1).xy;
}

float getSplotches(vec2 spriteAddr)
{
   // get sprite address into paged texture coords space
   spriteAddr = spriteAddr / 6.3;
   spriteAddr = spriteAddr - (spriteAddr / 33.3);

   return texture(SplotchesTx, spriteAddr).x;
}

void main()
{
   // get sprite address
   vec2 spriteAddr = calcSpriteAddr(inPs.uv0, DirtFrequency, time_cycle_period);

   // add some dark and light splotches to the film
   float splotches = getSplotches(spriteAddr);
   float specs = 1.0 - getSplotches(spriteAddr / 3.0);

   // convert color to base luminance
   vec4 base = texture(RT, inPs.uv0 + vec2(0.0, spriteAddr.y * frameJitter));
   float lumi = dot(base.rgb, luminance);
   // randomly shift luminance
   lumi -= spriteAddr.x * lumiShift;
   // tone map luminance
   base.rgb = texture(SepiaTx, lumi).rgb;

   // calc flicker speed
   float darken = fract(flicker * time_cycle_period);

   // we want darken to cycle between 0.6 and 1.0
   darken = abs(darken - 0.5) * 0.4 + 0.6;
   // composite dirt onto film
   fragColour = base * splotches * darken + specs;
}
