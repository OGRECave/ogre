#version 100

precision mediump int;
precision mediump float;

varying vec2 oUv0;
//varying vec2 uv1;

uniform sampler2D RT;
uniform sampler2D SplotchesTx;
uniform sampler2D Texture2;
uniform sampler2D SepiaTx;
uniform float time_cycle_period;
uniform float flicker;
uniform float DirtFrequency;
uniform vec3 luminance;
uniform float frameJitter;
uniform float lumiShift;

vec2 calcSpriteAddr(vec2 texCoord, float DirtFrequency1, float period)
{
   return texCoord + texture2D(Texture2, vec2(period * DirtFrequency1, 0.0)).xy;
}

vec4 getSplotches(vec2 spriteAddr)
{
   // get sprite address into paged texture coords space
   spriteAddr = spriteAddr / 6.3;
   spriteAddr = spriteAddr - (spriteAddr / 33.3);

   return texture2D(SplotchesTx, spriteAddr);
}

void main()
{
   // get sprite address
   vec2 spriteAddr = calcSpriteAddr(oUv0, DirtFrequency, time_cycle_period);

   // add some dark and light splotches to the film
   vec4 splotches = getSplotches(spriteAddr);
   vec4 specs = 1.0 - getSplotches(spriteAddr / 3.0);

   // convert color to base luminance
   vec4 base = texture2D(RT, oUv0 + vec2(0.0, spriteAddr.y * frameJitter));
   float lumi = dot(base.rgb, luminance);
   // randomly shift luminance
   lumi -= spriteAddr.x * lumiShift;
   // tone map luminance
   base.rgb = texture2D(SepiaTx, vec2(lumi, 0.0)).rgb;

   // calc flicker speed
   float darken = fract(flicker * time_cycle_period);

   // we want darken to cycle between 0.6 and 1.0
   darken = abs(darken - 0.5) * 0.4 + 0.6;
   // composite dirt onto film
   gl_FragColor = base * splotches * darken + specs;
}
