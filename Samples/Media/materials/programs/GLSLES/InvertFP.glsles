#version 100

precision mediump int;
precision mediump float;

uniform sampler2D RT;
varying vec2 oUv0;
//varying vec2 uv1;

void main()
{
	gl_FragColor = 1.0 - texture2D(RT, oUv0);
}
