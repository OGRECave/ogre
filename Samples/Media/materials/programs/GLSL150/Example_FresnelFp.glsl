#version 150

uniform vec4 tintColour;
uniform float noiseScale;
uniform float fresnelBias;
uniform float fresnelScale;
uniform float fresnelPower;
uniform sampler2D noiseMap;
uniform sampler2D reflectMap;
uniform sampler2D refractMap;

in vec3 noiseCoord;
in vec4 projectionCoord;
in vec3 eyeDir;
in vec3 oNormal;

out vec4 fragColour;

// Fragment program for distorting a texture using a 3D noise texture
void main()
{
	// Do the tex projection manually so we can distort _after_
	vec2 final = projectionCoord.xy / projectionCoord.w;

	// Noise
	vec3 noiseNormal = (texture(noiseMap, (noiseCoord.xy / 5.0)).rgb - 0.5).rbg * noiseScale;
	final += noiseNormal.xz;

	// Fresnel
	//normal = normalize(normal + noiseNormal.xz);
	float fresnel = fresnelBias + fresnelScale * pow(1.0 + dot(eyeDir, oNormal), fresnelPower);

	// Reflection / refraction
	vec4 reflectionColour = texture(reflectMap, final);
	vec4 refractionColour = texture(refractMap, final) + tintColour;

	// Final colour
	fragColour = mix(refractionColour, reflectionColour, fresnel);
}
