#version 150

uniform vec4 lightDiffuse;
uniform sampler2D normalMap;

in vec4 oUv0;
in vec3 oTSLightDir;

out vec4 fragColour;

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
    vec3 bumpVec = expand(texture(normalMap, oUv0.xy).xyz);

    // Calculate dot product
    fragColour = lightDiffuse * dot(bumpVec, lightVec);
}
