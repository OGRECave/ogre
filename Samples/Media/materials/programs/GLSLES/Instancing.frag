//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------

#version 300 es

precision mediump int;
precision mediump float;

uniform sampler2D diffuseMap;

uniform vec4	lightPosition;
uniform vec3	cameraPosition;
uniform vec3 	lightAmbient;
uniform vec3	lightDiffuse;
uniform vec3	lightSpecular;
uniform vec4	lightAttenuation;
uniform float	lightGloss;

#if DEPTH_SHADOWRECEIVER
uniform float invShadowMapSize;
uniform lowp sampler2DShadow shadowMap;

//declare external function

// Simple PCF 
// Number of samples in one dimension (square for total samples)
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1.0

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)
{
	return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadow(lowp sampler2DShadow shadowMap, vec4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2.0 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float depth = textureProj(shadowMap, offsetSample(uv, vec2(x, y), invShadowMapSize));
			if (depth >= 1.0 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}
#endif

in vec2 _uv0;
in vec3 oNormal;
in vec3 oVPos;
#if DEPTH_SHADOWRECEIVER
	in vec4 oLightSpacePos;
#endif
out vec4 fragColour;

//---------------------------------------------
//Main Pixel Shader
//---------------------------------------------
void main(void)
{
	vec4 color = texture( diffuseMap, _uv0 );

	float fShadow = 1.0;
#if DEPTH_SHADOWRECEIVER
	fShadow = calcDepthShadow( shadowMap, oLightSpacePos, invShadowMapSize );
#endif

	vec4 baseColour = texture( diffuseMap, _uv0 );

	//Blinn-Phong lighting
	vec3 normal	= normalize( oNormal );
	vec3 lightDir		= lightPosition.xyz - oVPos * lightPosition.w;
	vec3 eyeDir			= normalize( cameraPosition - oVPos );

	float fLength	= length( lightDir );
	lightDir			= normalize( lightDir );

	float NdotL	= max( 0.0, dot( normal, lightDir ) );
	vec3 halfVector		= normalize(lightDir + eyeDir);
	float HdotN	= max( 0.0, dot( halfVector, normal ) );
	
	vec3 ambient  = lightAmbient * baseColour.xyz;
	vec3 diffuse  = lightDiffuse * NdotL * baseColour.xyz;
	vec3 specular = lightSpecular * pow( HdotN, lightGloss );
	
	vec3 directLighting = (diffuse + specular) * fShadow;
	
	fragColour = vec4( directLighting + ambient, baseColour.a );
	//fragColour = baseColour;
}
