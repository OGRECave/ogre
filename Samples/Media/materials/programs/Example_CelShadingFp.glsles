#version 100
precision mediump int;
precision mediump float;

/* Cel shading fragment program for single-pass rendering */
uniform vec4 diffuse;
uniform vec4 specular;
uniform sampler2D diffuseRamp;
uniform sampler2D specularRamp;
uniform sampler2D edgeRamp;

varying float diffuseIn;
varying float specularIn;
varying float edge;

void main()
{
	// Step functions from textures
	vec4 diffuseStep = texture2D(diffuseRamp, vec2(diffuseIn));
	vec4 specularStep = texture2D(specularRamp, vec2(specularIn));
	vec4 edgeStep = texture2D(edgeRamp, vec2(edge));

	gl_FragColor = edgeStep.x * ((diffuse * diffuseStep.x) + 
					(specular * specularStep.x));
}
