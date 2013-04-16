#version 100

precision mediump int;
precision mediump float;

float shadowPCF(sampler2D shadowMap, vec4 shadowMapPos, vec2 offset)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
	vec2 uv = shadowMapPos.xy;
	vec3 o = vec3(offset, -offset.x) * 0.3;

	// Note: We using 2x2 PCF. Good enough and is a lot faster.
	float c =	(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.xy).r) ? 1.0 : 0.0; // top left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.xy).r) ? 1.0 : 0.0; // bottom right
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.zy).r) ? 1.0 : 0.0; // bottom left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.zy).r) ? 1.0 : 0.0; // top right

	return c / 4.0;
}

uniform vec4 invShadowMapSize0;
uniform vec4 invShadowMapSize1;
uniform vec4 invShadowMapSize2;
uniform vec4 pssmSplitPoints;
uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D normalMap;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 ambient;

varying vec4 oUv0;
varying vec3 oLightDir;
varying vec3 oHalfAngle;
varying vec4 oLightPosition0;
varying vec4 oLightPosition1;
varying vec4 oLightPosition2;
varying vec3 oNormal;

// to put it simply, this does 100% per pixel diffuse lighting
void main()
{
	// calculate shadow
	float shadowing = 1.0;
	vec4 splitColour;
	if (oUv0.z <= pssmSplitPoints.y)
	{
		splitColour = vec4(0.1, 0.0, 0.0, 1.0);
		shadowing = shadowPCF(shadowMap0, oLightPosition0, invShadowMapSize0.xy);
	}
	else if (oUv0.z <= pssmSplitPoints.z)
	{
		splitColour = vec4(0.0, 0.1, 0.0, 1.0);
		shadowing = shadowPCF(shadowMap1, oLightPosition1, invShadowMapSize1.xy);
	}
	else
	{
		splitColour = vec4(0.1, 0.1, 0.0, 1.0);
		shadowing = shadowPCF(shadowMap2, oLightPosition2, invShadowMapSize2.xy);
	}

	// retrieve normalised light vector, expand from range-compressed
	vec3 lightVec = normalize(oLightDir);

	// retrieve half angle and normalise through cube map
	vec3 halfAngle = normalize(oHalfAngle);

	// get diffuse colour
	vec4 diffuseColour = texture2D(diffuse, oUv0.xy);

	// specular
	vec4 specularColour = texture2D(specular, oUv0.xy);
	float shininess = specularColour.w;
	specularColour.w = 1.0;

	// calculate lit value.
	float diffuseCoeff = max(dot(oNormal, lightVec), 0.0);
	float specularCoeff = step(0.0, dot(oNormal, lightVec)) * max(dot(oNormal, halfAngle) * (shininess * 128.0), 0.0);
	vec4 lighting;
	lighting.y = diffuseCoeff * shadowing;
	lighting.z = specularCoeff * shadowing;
//	vec4 lighting = lit(dot(oNormal, lightVec), dot(oNormal, halfAngle), shininess * 128.0) * shadowing;

	// final lighting with diffuse and spec
	gl_FragColor = (diffuseColour * clamp(ambient + lightDiffuse * lighting.y, 0.0, 1.0)) + (lightSpecular * specularColour * lighting.z);
	gl_FragColor.w = diffuseColour.w;

	//oColour += splitColour;
}
