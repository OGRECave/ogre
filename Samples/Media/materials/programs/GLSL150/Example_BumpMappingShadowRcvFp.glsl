#version 150

// General functions

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform vec4 lightDiffuse;

in vec4 uvproj;
in vec4 oUv0;
in vec3 oTSLightDir;

out vec4 fragColour;

void main()
{
	// retrieve normalised light vector, expand from range-compressed
	vec3 lightVec = expand(normalize(oTSLightDir).xyz);

	// get bump map vector, again expand from range-compressed
	vec3 bumpVec = expand(texture(normalMap, oUv0.xy).xyz);

	// get shadow value
	vec3 shadow = textureProj(shadowMap, uvproj).xyz;

	// Calculate dot product
	fragColour = vec4(shadow * lightDiffuse.xyz * dot(bumpVec, lightVec), 1.0);
}
