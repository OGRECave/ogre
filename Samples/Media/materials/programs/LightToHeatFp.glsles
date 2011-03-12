#version 100

precision mediump int;
precision mediump float;

uniform vec4 random_fractions;
uniform vec4 heatBiasScale;
uniform vec4 depth_modulator;

uniform sampler2D Input;         // output of HeatVisionCaster_fp (NdotV)
uniform sampler2D NoiseMap;
uniform sampler2D HeatLookup;

varying vec4 diffuse;
varying vec2 uv;

void main()
{
   float  depth, heat, interference;

   //  Output constant color:
   depth = texture2D( Input, uv ).x;
   depth *= (depth * depth_modulator).x;

   heat  = (depth * heatBiasScale.y);

//   if (depth > 0)
   {
		interference = -0.5 + texture2D( NoiseMap, uv + vec2( random_fractions.x, random_fractions.y ) ).x;
		interference *= interference;
		interference *= 1.0 - heat;
		heat += interference;//+ heatBiasScale.x;
   }

   // Clamp UVs
   heat  = max( 0.005, min( 0.995, heat ) );
   gl_FragColor = texture2D( HeatLookup, vec2( heat, 0.0 ) );
}

