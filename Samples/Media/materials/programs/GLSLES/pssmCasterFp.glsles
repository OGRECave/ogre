#version 100

precision mediump int;
precision mediump float;

varying vec2 oDepth;
uniform vec4 pssmSplitPoints;

void main()
{
	float finalDepth = oDepth.x / oDepth.y;
	gl_FragColor = vec4(finalDepth, finalDepth, finalDepth, 1.0);
}
