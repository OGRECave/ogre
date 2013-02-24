#version 100

precision mediump int;
precision mediump float;

uniform sampler2D tex;
varying vec4 ambientUV;

void main ()
{
	gl_FragColor = vec4(texture2D(tex, ambientUV.xy));

	// Blow out the light a bit
	gl_FragColor *= 1.7;
}
