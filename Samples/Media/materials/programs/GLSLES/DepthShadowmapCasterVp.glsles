#version 100
precision highp int;
precision highp float;

uniform mat4 worldViewProj;
uniform vec4 texelOffsets;
uniform vec4 depthRange;

varying vec2 depth;

attribute vec4 position;

void main()
{
	gl_Position = worldViewProj * position;

	// fix pixel / texel alignment
	gl_Position.xy += texelOffsets.zw * gl_Position.w;
	// linear depth storage
	// offset / scale range output
#if LINEAR_RANGE
	depth.x = (gl_Position.z - depthRange.x) * depthRange.w;
#else
	depth.x = gl_Position.z;
#endif
	depth.y = gl_Position.w;
}
