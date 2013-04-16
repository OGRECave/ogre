#version 150

in vec4 uv0;
in vec4 position;
in vec3 normal;

uniform mat4 worldViewProjMatrix;
uniform vec3 eyePosition; // object space
uniform float timeVal;
uniform float scale;  // the amount to scale the noise texture by
uniform float scroll; // the amount by which to scroll the noise
uniform float noise;  // the noise perturb as a factor of the time

out vec3 noiseCoord;
out vec4 projectionCoord;
out vec3 eyeDir;
out vec3 oNormal;

// Vertex program for fresnel reflections / refractions
void main()
{
	gl_Position = worldViewProjMatrix * position;
	// Projective texture coordinates, adjust for mapping
	mat4 scalemat = mat4(0.5, 0.0, 0.0, 0.0, 
                         0.0, -0.5, 0.0, 0.0,
                         0.0, 0.0, 0.5, 0.0,
                         0.5, 0.5, 0.5, 1.0);
	projectionCoord = scalemat * gl_Position;

	// Noise map coords
	noiseCoord.xy = (uv0.xy + (timeVal * scroll)) * scale;
	noiseCoord.z = noise * timeVal;

	eyeDir = normalize(position.xyz - eyePosition); 
	oNormal = normal.rgb;
}
