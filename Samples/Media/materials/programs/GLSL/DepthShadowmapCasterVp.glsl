#version 120

uniform mat4 worldViewProj;
uniform vec4 texelOffsets;

attribute vec4 vertex;

varying vec2 depth;

void main()
{
	vec4 outPos = worldViewProj * vertex;
	outPos.xy += texelOffsets.zw * outPos.w;
	// fix pixel / texel alignment
	depth = outPos.zw;
	gl_Position = outPos;
}

