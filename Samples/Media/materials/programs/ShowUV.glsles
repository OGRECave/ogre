#version 100
precision mediump int;
precision mediump float;

varying vec2 ambientUV;

// Basic fragment program to display UV
void main()
{
	// wrap values using fract
	gl_FragColor = vec4(fract(ambientUV.x), fract(ambientUV.y), 0.0, 1.0);
}
