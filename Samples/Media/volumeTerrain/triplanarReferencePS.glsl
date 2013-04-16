#version 150

in vec3 oPos;
in vec4 oNormAndFogVal;
in vec3 oEyePos;

#if LIGHT0
uniform vec4 lightPosition0;
uniform vec4 spotlightDir0;
uniform vec4 lightDiffuse0;
uniform vec4 lightSpecular0;
uniform vec4 lightAttenuation0;
uniform vec4 lightSpotlight0;
#endif
#if LIGHT1
uniform vec4 lightPosition1;
uniform vec4 spotlightDir1;
uniform vec4 lightDiffuse1;
uniform vec4 lightSpecular1;
uniform vec4 lightAttenuation1;
uniform vec4 lightSpotlight1;
#endif
#if LIGHT2
uniform vec4 lightPosition2;
uniform vec4 spotlightDir2;
uniform vec4 lightDiffuse2;
uniform vec4 lightSpecular2;
uniform vec4 lightAttenuation2;
uniform vec4 lightSpotlight2;
#endif

uniform mat4 world;
uniform float exponent;
uniform float texScale;
uniform float plateauSize;
uniform float transitionSpeed;

#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
uniform vec4 fogColour;
#endif

uniform vec4 ambient;
uniform sampler2D texFromX;
uniform sampler2D texFromXNormal;
uniform sampler2D texFromY;
uniform sampler2D texFromYNormal;
uniform sampler2D texFromZ;
uniform sampler2D texFromZNormal;

out vec4 fragColour;

vec3 expand(vec3 v)
{
	return (v - 0.5) * 2;
}

vec4 lit(float NdotL, float NdotH, float m)
{
    float ambient = 1.0;
    float diffuse = max(NdotL, 0.0);
    float specular = step(0.0, NdotL) * max(NdotH * m, 0.0);
    
    return vec4(ambient, diffuse, specular, 1.0);
}

vec4 doLighting(
	vec3 oPos,
	vec3 normal,
	vec3 eyeDir,
	float exponent,
	float specularFactor,
	vec3 lightDir,
	vec4 lightPosition,
	vec4 lightDiffuse,
	vec4 lightSpecular,
	vec4 lightAttenuation,
	vec4 lightSpotlight,
	vec3 lightSpotDir
) {
	vec3 halfAngle = normalize(lightDir + eyeDir);

	float nDotL = dot(lightDir, normal);
	float nDotH = dot(halfAngle, normal);
	vec4 lighting = lit(nDotL, nDotH, exponent);
	
	float attenuation = 1;
	#if ATTENUATION
	if (lightPosition.w != 0) {
		float distance = length(lightPosition.xyz - oPos);
		attenuation = 1.0 / (lightAttenuation.y + lightAttenuation.z * distance  + lightAttenuation.w * distance * distance);
	}
	#endif
	
	float spot = 1;
	if (!(lightSpotlight.x == 1 && lightSpotlight.y == 0 && lightSpotlight.z == 0 && lightSpotlight.w == 1)) {
		spot = clamp(
			(dot(lightDir, normalize(-lightSpotDir)) - lightSpotlight.y) /
			(lightSpotlight.x - lightSpotlight.y), 0.0, 1.0);
	}
	
	return attenuation * spot * (lightDiffuse * lighting.y + specularFactor * lightSpecular * lighting.z);
}

void main()
{
	
	vec3 unitNormal = normalize(oNormAndFogVal.xyz);
	vec3 eyeDir = normalize(oEyePos - oPos);
		
	// Ported from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
	vec3 blendWeights = abs(unitNormal);
	blendWeights = blendWeights - plateauSize;
	blendWeights = pow(max(blendWeights, 0), vec3(transitionSpeed));
	blendWeights /= vec3(blendWeights.x + blendWeights.y + blendWeights.z );
	// Move the planar mapping a bit according to the normal length to avoid bad looking skirts.
	float nLength = length(oNormAndFogVal.xyz - 1.0);
	vec2 coord1 = (oPos.yz + nLength) * texScale;
	vec2 coord2 = (oPos.zx + nLength) * texScale;
	vec2 coord3 = (oPos.xy + nLength) * texScale;
	
	vec4 col1 = texture(texFromX, coord1);
	vec4 col2 = texture(texFromY, coord2);
	vec4 col3 = texture(texFromZ, coord3);
	vec4 textColour = vec4(col1.xyz * blendWeights.x +
		col2.xyz * blendWeights.y +
		col3.xyz * blendWeights.z, 1);
	
	// Normal Mapping
	#if LIGHTNORMALMAPPING
	vec3 tangent = vec3(1, 0, 0);
	vec3 binormal = normalize(cross(tangent, unitNormal));
	tangent = normalize(cross(unitNormal, binormal));
	mat3 TBN = mat3(tangent, binormal, unitNormal);
	vec3 eyeDir2 = normalize(TBN * eyeDir);
	vec3 bumpFetch1 = expand(texture(texFromXNormal, coord1).rgb);
    vec3 bumpFetch2 = expand(texture(texFromYNormal, coord2).rgb);
    vec3 bumpFetch3 = expand(texture(texFromZNormal, coord3).rgb);
	vec3 normal2 = bumpFetch1.xyz * blendWeights.x +  
		bumpFetch2.xyz * blendWeights.y +  
		bumpFetch3.xyz * blendWeights.z;
	#else
	vec3 eyeDir2 = eyeDir;
	vec3 normal2 = unitNormal;
	#endif
	
	#if USESPECULARMAP
	float specularFactor = textColour.a;
	#else
	float specularFactor = 1.0;
	#endif
	
	// Light
	vec4 lightContribution = vec4(0, 0, 0, 0);
	#if LIGHT0
	vec3 lightSpotDir0 = (world * spotlightDir0).xyz;
	vec3 lightDir0 = normalize(lightPosition0.xyz - oPos * lightPosition0.w);
	#if LIGHTNORMALMAPPING
	vec3 lightSpotDir02 = normalize(TBN * lightSpotDir0);
	vec3 lightDir02 = normalize(TBN * lightDir0);
	#else
	vec3 lightSpotDir02 = lightSpotDir0;
	vec3 lightDir02 = lightDir0;
	#endif
	lightContribution += doLighting(oPos, normal2, eyeDir2, exponent, specularFactor, lightDir02, lightPosition0, lightDiffuse0, lightSpecular0, lightAttenuation0, lightSpotlight0, lightSpotDir02);
	#endif
	
	#if LIGHT1
	vec3 lightSpotDir1 = (world * spotlightDir1).xyz;
	vec3 lightDir1 = normalize(lightPosition1.xyz - oPos * lightPosition1.w);
	#if LIGHTNORMALMAPPING
	vec3 lightSpotDir12 = normalize(TBN * lightSpotDir1);
	vec3 lightDir12 = normalize(TBN * lightDir1);
	#else
	vec3 lightSpotDir12 = lightSpotDir1;
	vec3 lightDir12 = lightDir1;
	#endif
	lightContribution += doLighting(oPos, normal2, eyeDir2, exponent, specularFactor, lightDir12, lightPosition1, lightDiffuse1, lightSpecular1, lightAttenuation1, lightSpotlight1, lightSpotDir12);
	#endif
	
	#if LIGHT2
	vec3 lightSpotDir2 = (world * spotlightDir2).xyz;
	vec3 lightDir2 = normalize(lightPosition2.xyz - oPos * lightPosition2.w);
	#if LIGHTNORMALMAPPING
	vec3 lightSpotDir22 = normalize(TBN * lightSpotDir2);
	vec3 lightDir22 = normalize(TBN * lightDir2);
	#else
	vec3 lightSpotDir22 = lightSpotDir2;
	vec3 lightDir22 = lightDir2;
	#endif
	lightContribution += doLighting(oPos, normal2, eyeDir2, exponent, specularFactor, lightDir22, lightPosition2, lightDiffuse2, lightSpecular2, lightAttenuation2, lightSpotlight2, lightSpotDir22);
	#endif
	
	fragColour = clamp(textColour * (lightContribution + ambient), 0.0, 1.0);
	#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
	fragColour = mix(oColor, fogColour, oNormAndFogVal.w);
	#endif
	
}
