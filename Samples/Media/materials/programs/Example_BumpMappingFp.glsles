#version 100
precision mediump int;
precision mediump float;

uniform vec4 lightDiffuse;
uniform sampler2D normalMap;

varying vec2 oUv0;
varying vec3 oTSLightDir;

// General functions

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

void main()
{
	// Retrieve normalised light vector, expand from range-compressed
	vec3 lightVec = normalize(oTSLightDir).xyz;

	// Get bump map vector, again expand from range-compressed
	vec3 bumpVec = expand(texture2D(normalMap, oUv0).xyz);

	// Calculate dot product
	gl_FragColor = lightDiffuse * dot(bumpVec, lightVec);
}
