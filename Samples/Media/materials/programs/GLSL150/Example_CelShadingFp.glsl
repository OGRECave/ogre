#version 150

/* Cel shading fragment program for single-pass rendering */
uniform vec4 diffuse;
uniform vec4 specular;
uniform sampler1D diffuseRamp;
uniform sampler1D specularRamp;
uniform sampler1D edgeRamp;

in float diffuseIn;
in float specularIn;
in float edge;

out vec4 fragColour;

/*uniform lighting
{
	vec4 diffuse;
	vec4 specular;
} LightingParams;*/

void main()
{
	// Step functions from textures
	float diffuseStep = texture(diffuseRamp, diffuseIn).x;
	float specularStep = texture(specularRamp, specularIn).x;
	float edgeStep = texture(edgeRamp, edge).x;

	fragColour = edgeStep * ((diffuse * diffuseStep) + 
                            (specular * specularStep));
//	fragColour = edgeStep * ((LightingParams.diffuse * diffuseStep) + 
//                        (LightingParams.specular * specularStep));
}
