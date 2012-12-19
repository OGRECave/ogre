#version 100

precision mediump int;
precision mediump float;

uniform vec4 tintColour;
uniform float noiseScale;
uniform float fresnelBias;
uniform float fresnelScale;
uniform float fresnelPower;
uniform sampler2D noiseMap;
uniform sampler2D reflectMap;
uniform sampler2D refractMap;

varying vec3 noiseCoord;
varying vec4 projectionCoord;
varying vec3 eyeDir;
varying vec3 oNormal;

// Fragment program for distorting a texture using a 3D noise texture
void main()
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
