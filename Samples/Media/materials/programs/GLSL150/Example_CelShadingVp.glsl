#version 150

/* Cel shading vertex program for single-pass rendering
   In this program, we want to calculate the diffuse and specular
   ramp components, and the edge factor (for doing simple outlining)
   For the outlining to look good, we need a pretty well curved model.
*/
// Parameters
in vec4 vertex;
in vec3 normal;

uniform vec3 lightPosition; // object space
uniform vec3 eyePosition;   // object space
uniform vec4 shininess;
uniform mat4 worldViewProj;

//uniform transform
//{
//	vec4 shininess;
//} MaterialShininess;

out float diffuseIn;
out float specularIn;
out float edge;

void main()
{
	// calculate output position
	gl_Position = worldViewProj * vertex;

	// calculate light vector
	vec3 N = normalize(normal);
	vec3 L = normalize(lightPosition - vertex.xyz);
	
	// Calculate diffuse component
	diffuseIn = max(dot(N, L) , 0.0);

	// Mask off specular if diffuse is 0
	if (diffuseIn == 0.0)
	    specularIn = 0.0;

    // Calculate specular component
    vec3 E = normalize(eyePosition - vertex.xyz);
    vec3 H = normalize(L + E);
    specularIn = pow(max(dot(N, H), 0.0), shininess.x);
//    specularIn = pow(max(dot(N, H), 0.0), MaterialShininess.shininess.x);

	// Edge detection, dot eye and normal vectors
	edge = max(dot(N, E), 0.0);
}
