#version 100
precision mediump int;
precision mediump float;

/* Cel shading vertex program for single-pass rendering
   In this program, we want to calculate the diffuse and specular
   ramp components, and the edge factor (for doing simple outlining)
   For the outlining to look good, we need a pretty well curved model.
*/
// Parameters
attribute vec4 vertex;
attribute vec3 normal;

uniform vec3 lightPosition; // object space
uniform vec3 eyePosition;   // object space
uniform vec4 shininess;
uniform mat4 worldViewProj;

varying float diffuseIn;
varying float specularIn;
varying float edge;

void main()
{
	// calculate output position
	gl_Position = worldViewProj * vertex;

	// calculate light vector
	vec3 N = normalize(normal);
	vec3 L = normalize(lightPosition - vertex.xyz);
	vec3 E = normalize(eyePosition - vertex.xyz);
	
	// Calculate diffuse component
	diffuseIn = max(dot(N, L) , 0.0);

	// Mask off specular if diffuse is 0
	if (diffuseIn == 0.0)
	{
	    specularIn = 0.0;
	}
	else
	{
		// Calculate specular component
		vec3 H = normalize(L + E);
		specularIn = pow(max(dot(N, H), 0.0), shininess.x);
	}

	// Edge detection, dot eye and normal vectors
	edge = max(dot(N, E), 0.0);
}
