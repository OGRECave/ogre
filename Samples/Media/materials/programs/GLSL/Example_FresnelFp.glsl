#include <OgreUnifiedShader.h>

SAMPLER2D(noiseMap, 0);
SAMPLER2D(reflectMap, 1);
SAMPLER2D(refractMap, 2);

OGRE_UNIFORMS(
uniform vec4 tintColour;
uniform float noiseScale;
uniform float fresnelBias;
uniform float fresnelScale;
uniform float fresnelPower;
)

MAIN_PARAMETERS
IN(vec3 noiseCoord, TEXCOORD0)
IN(vec4 projectionCoord, TEXCOORD1)
IN(vec3 eyeDir, TEXCOORD2)
IN(vec3 oNormal, TEXCOORD3)
MAIN_DECLARATION
{
	// Do the tex projection manually so we can distort _after_
	vec2 final = projectionCoord.xy / projectionCoord.w;

	// Noise
	vec3 noiseNormal = (texture2D(noiseMap, (noiseCoord.xy / 5.0)).rgb - 0.5).rbg * noiseScale;
	final += noiseNormal.xz;

	// Fresnel
	//normal = normalize(normal + noiseNormal.xz);
	float fresnel = fresnelBias + fresnelScale * pow(1.0 + dot(eyeDir, oNormal), fresnelPower);

	// Reflection / refraction
	vec4 reflectionColour = texture2D(reflectMap, final);
	vec4 refractionColour = texture2D(refractMap, final) + tintColour;

	// Final colour
	gl_FragColor = mix(refractionColour, reflectionColour, fresnel);
}
