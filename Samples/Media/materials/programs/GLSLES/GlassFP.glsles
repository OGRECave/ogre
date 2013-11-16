#version 100

precision mediump int;
precision mediump float;

uniform sampler2D RT;
uniform sampler2D NormalMap;
varying vec2 oUv0;

void main()
{
	vec4 normal = 2.0 * (texture2D(NormalMap, oUv0 * 2.5) - 0.5);

	gl_FragColor = texture2D(RT, oUv0 + normal.xy * 0.05);
}
