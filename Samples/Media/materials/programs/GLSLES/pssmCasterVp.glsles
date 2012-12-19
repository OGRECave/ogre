#version 100

precision mediump int;
precision mediump float;

varying vec2 oDepth;
uniform mat4 worldViewProjMatrix;
attribute vec4 vertex;

void main()
{
	// This is the view space position
	gl_Position = worldViewProjMatrix * vertex;

	// Depth info for the fragment.
	oDepth.x = gl_Position.z;
	oDepth.y = gl_Position.w;

	// Clamp z to zero. seem to do the trick. :-/
	//oPosition.z = max(oPosition.z, 0);
}
