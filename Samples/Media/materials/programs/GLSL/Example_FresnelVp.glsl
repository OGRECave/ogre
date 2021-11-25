#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
uniform mat4 worldViewProjMatrix;
uniform mat4 textureProjMatrix;
uniform mat3 normalMatrix;
uniform vec3 eyePosition; // object space
uniform float timeVal;
uniform float scale;  // the amount to scale the noise texture by
uniform float scroll; // the amount by which to scroll the noise
uniform float noise;  // the noise perturb as a factor of the time
)

MAIN_PARAMETERS
IN(vec4 position, POSITION)
IN(vec3 normal, NORMAL)
IN(vec4 uv0, TEXCOORD0)

OUT(vec3 noiseCoord, TEXCOORD0)
OUT(vec4 projectionCoord, TEXCOORD1)
OUT(vec3 eyeDir, TEXCOORD2)
OUT(vec3 oNormal, TEXCOORD3)
MAIN_DECLARATION
{
	gl_Position = mul(worldViewProjMatrix, position);
	projectionCoord = mul(textureProjMatrix, position);

	// Noise map coords
	noiseCoord.xy = (uv0.xy + (timeVal * scroll)) * scale;
	noiseCoord.z = noise * timeVal;

	eyeDir = normalize(position.xyz - eyePosition); 
	oNormal = normal;
}
