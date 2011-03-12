#version 100

precision mediump int;
precision mediump float;

////////////////////////////// MOVING GRASS
// Vertex program to wave some grass about
// Assumes UV texture coords of v==0 indicates the top of the grass
uniform mat4 worldViewProj;
uniform vec4 camObjPos;
uniform vec4 ambient;
uniform vec4 objSpaceLight;
uniform vec4 lightColour;
uniform vec4 offset;

attribute vec4 position;
attribute vec4 normal;
attribute vec4 uv0;

varying vec4 oUv0;
varying vec4 oColour;

void main()
{
	vec4 mypos = position;
	vec4 factor = vec4(1.0, 1.0, 1.0, 1.0) - uv0.yyyy;
	mypos = mypos + offset * factor;
	gl_Position = worldViewProj * mypos;

	oUv0 = uv0;
    // Color
	// get vertex light direction (support directional and point)
	vec3 light = normalize(objSpaceLight.xyz - (mypos.xyz * objSpaceLight.w).xyz);
	// grass is just 2D quads, so if light passes underneath we need to invert the normal
	// abs() will have the same effect
    float diffuseFactor = abs(dot(normal.xyz, light));
	oColour = ambient + diffuseFactor * lightColour;
}
