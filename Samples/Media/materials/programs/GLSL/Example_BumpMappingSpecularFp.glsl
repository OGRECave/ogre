// General functions

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform sampler2D normalMap;

varying vec4 oUv0;
varying vec3 oTSLightDir;
varying vec3 oTSHalfAngle;

// NOTE: GLSL does not have the saturate function.  But it is equivalent to clamp(val, 0.0, 1.0)

/* Fragment program which supports specular component */
void main()
{
	// retrieve normalised light vector
	vec3 lightVec = normalize(oTSLightDir);

	// retrieve half angle and normalise
	vec3 halfAngle = normalize(oTSHalfAngle);

	// get bump map vector, again expand from range-compressed
	vec3 bumpVec = expand(texture2D(normalMap, oUv0.xy).xyz);

	// Pre-raise the specular exponent to the eight power
	float specFactor = pow(clamp(dot(bumpVec, halfAngle), 0.0, 1.0), 4.0);

	// Calculate dot product for diffuse
	gl_FragColor = (lightDiffuse * clamp(dot(bumpVec, lightVec), 0.0, 1.0)) + 
			(lightSpecular * specFactor);
}
