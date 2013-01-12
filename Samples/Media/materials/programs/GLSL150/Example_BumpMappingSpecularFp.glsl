#version 150

// General functions

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform sampler2D normalMap;

in vec4 oUv0;
in vec3 oTSLightDir;
in vec3 oTSHalfAngle;

out vec4 fragColour;

// NOTE: GLSL does not have the saturate function.  But it is equivalent to clamp(val, 0.0, 1.0)

/* Fragment program which supports specular component */
void main()
{
	// retrieve normalised light vector
	vec3 lightVec = normalize(oTSLightDir);

	// retrieve half angle and normalise
	vec3 halfAngle = normalize(oTSHalfAngle);

	// get bump map vector, again expand from range-compressed
	vec3 bumpVec = expand(texture(normalMap, oUv0.xy).xyz);

	// Pre-raise the specular exponent to the eight power
	float specFactor = pow(clamp(dot(bumpVec, halfAngle), 0.0, 1.0), 4.0);

	// Calculate dot product for diffuse
	fragColour = (lightDiffuse * clamp(dot(bumpVec, lightVec), 0.0, 1.0)) + 
			(lightSpecular * specFactor);
}
