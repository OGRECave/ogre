// General functions

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform vec4 lightDiffuse;

varying vec4 uvproj;
varying vec4 oUv0;
varying vec3 oTSLightDir;

void main()
{
	// retrieve normalised light vector, expand from range-compressed
	vec3 lightVec = expand(normalize(oTSLightDir).xyz);

	// get bump map vector, again expand from range-compressed
	vec3 bumpVec = expand(texture2D(normalMap, oUv0.xy).xyz);

	// get shadow value
	vec3 shadow = texture2DProj(shadowMap, uvproj).xyz;

	// Calculate dot product
	gl_FragColor = vec4(shadow * lightDiffuse.xyz * dot(bumpVec, lightVec), 1.0);
}
