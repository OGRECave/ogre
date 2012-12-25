#version 150

//------------------------------------------------------
//Radial_Blur_FP.glsl
//  Implements radial blur to be used with the compositor
//  It's very dependent on screen resolution
//------------------------------------------------------

uniform sampler2D tex;
uniform float sampleDist;
uniform float sampleStrength;

in vec2 oUv0;
out vec4 fragColour;

void main()
{
	float samples[10];
	
	samples[0] = -0.08;
	samples[1] = -0.05;
	samples[2] = -0.03;
	samples[3] = -0.02;
	samples[4] = -0.01;
	samples[5] = 0.01;
	samples[6] = 0.02;
	samples[7] = 0.03;
	samples[8] = 0.05;
	samples[9] = 0.08;
	
   //Vector from pixel to the center of the screen
   vec2 dir = 0.5 - oUv0;

   //Distance from pixel to the center (distant pixels have stronger effect)
   //float dist = distance( vec2( 0.5, 0.5 ), texCoord );
   float dist = sqrt( dir.x*dir.x + dir.y*dir.y );


   //Now that we have dist, we can normlize vector
   dir = normalize( dir );

   //Save the color to be used later
   vec4 color = texture( tex, oUv0 );
   //Average the pixels going along the vector
   vec4 sum = color;
   for (int i = 0; i < 10; i++)
   {
      vec4 res=texture( tex, oUv0 + dir * samples[i] * sampleDist );
      sum += res;
   }
   sum /= 11.0;

   //Calculate amount of blur based on
   //distance and a strength parameter
   float t = dist * sampleStrength;
   t = clamp( t, 0.0, 1.0 );//We need 0 <= t <= 1

   //Blend the original color with the averaged pixels
   fragColour = mix( color, sum, t );
}
