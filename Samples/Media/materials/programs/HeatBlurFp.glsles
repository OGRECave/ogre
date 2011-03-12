#version 100

precision mediump int;
precision mediump float;

uniform sampler2D Input;
uniform vec4 blurAmount;

varying vec2 uv;

void main()
{
   int i;
   vec4 tmpOutColor;
   float  diffuseGlowFactor;
   vec2 offsets[4];
/*
		// hazy blur
		-1.8, -1.8,
		-1.8, 1.8,
		1.8, -1.8,
		1.8, 1.8
*/
/*
		// less-hazy blur
	  -1.0,  2.0,
	  -1.0, -1.0,
	   1.0, -1.0,
	   1.0,  1.0
*/
/*
      -0.326212, -0.405805,
      -0.840144, -0.073580,
      -0.695914,  0.457137,
      -0.203345,  0.620716
*/

   offsets[0] = vec2(-0.3,  0.4);
   offsets[1] = vec2(-0.3,  -0.4);
   offsets[2] = vec2(0.3,  -0.4);
   offsets[3] = vec2(0.3,  0.4);

   tmpOutColor = texture2D( Input, uv );	// UV coords are in image space

   // calculate glow amount
   diffuseGlowFactor = 0.0113 * (2.0 - max( tmpOutColor.r, tmpOutColor.g ));

   // basic blur filter
   for (i = 0; i < 4; i++) {
      tmpOutColor += texture2D( Input, uv + blurAmount.x * diffuseGlowFactor * offsets[i] );
   }

   tmpOutColor *= 0.25;

   // TIPS (old-skool strikes again!)
   // Pay attention here! If you use the "out float4 outColor" directly
   // in your steps while creating the output color (like you remove
   // the "tmpOutColor" var and just use the "outColor" directly)
   // your pixel-color output IS CHANGING EACH TIME YOU DO AN ASSIGNMENT TOO!
   // A temporary variable, instead, acts like a per-pixel double buffer, and
   // best of all, lead to better performance.
   gl_FragColor = tmpOutColor;
}