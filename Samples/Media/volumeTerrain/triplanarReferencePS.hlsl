float3 expand(float3 v)
{
	return (v - 0.5) * 2;
}

float4 doLighting(
	float3 position,
	float3 normal,
	float3 eyeDir,
	float exponent,
	float specularFactor,
	float3 lightDir,
	float4 lightPosition,
	float4 lightDiffuse,
	float4 lightSpecular,
	float4 lightAttenuation,
	float4 lightSpotlight,
	float3 lightSpotDir
) {
	float3 halfAngle = normalize(lightDir + eyeDir);

	float nDotL = dot(lightDir, normal);
	float nDotH = dot(halfAngle, normal);
	float4 lighting = lit(nDotL, nDotH, exponent);
	
	float attenuation = 1;
	#ifdef ATTENUATION
	if (lightPosition.w != 0) {
		float distance = length(lightPosition.xyz - position);
		attenuation = 1.0 / (lightAttenuation.y + lightAttenuation.z * distance  + lightAttenuation.w * distance * distance);
	}
	#endif
	
	float spot = 1;
	if (!(lightSpotlight.x == 1 && lightSpotlight.y == 0 && lightSpotlight.z == 0 && lightSpotlight.w == 1)) {
		spot = saturate(
			(dot(lightDir, normalize(-lightSpotDir)) - lightSpotlight.y) /
			(lightSpotlight.x - lightSpotlight.y));
	}
	
	return attenuation * spot * (lightDiffuse * lighting.y + specularFactor * lightSpecular * lighting.z);
}

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D texFromX: register(s0);
Texture2D texFromXNormal: register(s1);
Texture2D texFromY: register(s2);
Texture2D texFromYNormal: register(s3);
Texture2D texFromZ: register(s4);
Texture2D texFromZNormal: register(s5);

struct v2p
{
	float4 oClipPos: SV_POSITION;
	float3 position: TEXCOORD0;
	float4 normalAndFogVal: TEXCOORD1;
	float3 eyePos: TEXCOORD2;
};

float4 main_triplanar_reference_fp(
	v2p input,
	#ifdef LIGHT0
	uniform float4 lightPosition0,
	uniform float4 spotlightDir0,
	uniform float4 lightDiffuse0,
	uniform float4 lightSpecular0,
	uniform float4 lightAttenuation0,
	uniform float4 lightSpotlight0,
	#endif
	#ifdef LIGHT1
	uniform float4 lightPosition1,
	uniform float4 spotlightDir1,
	uniform float4 lightDiffuse1,
	uniform float4 lightSpecular1,
	uniform float4 lightAttenuation1,
	uniform float4 lightSpotlight1,
	#endif
	#ifdef LIGHT2
	uniform float4 lightPosition2,
	uniform float4 spotlightDir2,
	uniform float4 lightDiffuse2,
	uniform float4 lightSpecular2,
	uniform float4 lightAttenuation2,
	uniform float4 lightSpotlight2,
	#endif
	
    uniform float4x4 world,
	uniform float exponent,
	uniform float texScale,
	uniform float plateauSize,
	uniform float transitionSpeed,

	#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
	uniform float4 fogColour,
	#endif
	
	uniform float4 ambient

) : SV_Target
{	
	float3 unitNormal = normalize(input.normalAndFogVal.xyz);
	float3 eyeDir = normalize(input.eyePos - input.position);
		
	// Ported from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
	float3 blendWeights = abs(unitNormal);
	blendWeights = blendWeights - plateauSize;
	blendWeights = pow(max(blendWeights, 0), transitionSpeed);
	blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z ).xxx;
	// Move the planar mapping a bit according to the normal length to avoid bad looking skirts.
	float nLength = length(input.normalAndFogVal.xyz - 1.0);
	float2 coord1 = (input.position.yz + nLength) * texScale;
	float2 coord2 = (input.position.zx + nLength) * texScale;
	float2 coord3 = (input.position.xy + nLength) * texScale;
	
	float4 col1 = texFromX.Sample(g_samLinear, coord1);
	float4 col2 = texFromY.Sample(g_samLinear, coord2);
	float4 col3 = texFromZ.Sample(g_samLinear, coord3);
	float4 textColour = float4(col1.xyz * blendWeights.x +
		col2.xyz * blendWeights.y +
		col3.xyz * blendWeights.z, 1);
	
	// Normal Mapping
	#ifdef LIGHTNORMALMAPPING
	float3 tangent = float3(1, 0, 0);
	float3 binormal = normalize(cross(tangent, unitNormal));
	tangent = normalize(cross(unitNormal, binormal));
	float3x3 TBN = float3x3(tangent, binormal, unitNormal);
	float3 eyeDir2 = normalize(mul(TBN, eyeDir));
	float3 bumpFetch1 = expand(texFromXNormal.Sample(g_samLinear, coord1).rgb);
    float3 bumpFetch2 = expand(texFromYNormal.Sample(g_samLinear, coord2).rgb);
    float3 bumpFetch3 = expand(texFromZNormal.Sample(g_samLinear, coord3).rgb);
	float3 normal2 = bumpFetch1.xyz * blendWeights.x +  
		bumpFetch2.xyz * blendWeights.y +  
		bumpFetch3.xyz * blendWeights.z;
	#else
	float3 eyeDir2 = eyeDir;
	float3 normal2 = unitNormal;
	#endif
	
	#ifdef USESPECULARMAP
	float specularFactor = textColour.a;
	#else
	float specularFactor = 1.0;
	#endif
	
	// Light
	float4 lightContribution = float4(0, 0, 0, 0);
	#ifdef LIGHT0
	float3 lightSpotDir0 = mul(world, spotlightDir0).xyz;
	float3 lightDir0 = normalize(lightPosition0.xyz - input.position * lightPosition0.w);
	#ifdef LIGHTNORMALMAPPING
	float3 lightSpotDir02 = normalize(mul(TBN, lightSpotDir0));
	float3 lightDir02 = normalize(mul(TBN, lightDir0));
	#else
	float3 lightSpotDir02 = lightSpotDir0;
	float3 lightDir02 = lightDir0;
	#endif
	lightContribution += doLighting(input.position, normal2, eyeDir2, exponent, specularFactor, lightDir02, lightPosition0, lightDiffuse0, lightSpecular0, lightAttenuation0, lightSpotlight0, lightSpotDir02);
	#endif
	
	#ifdef LIGHT1
	float3 lightSpotDir1 = mul(world, spotlightDir1).xyz;
	float3 lightDir1 = normalize(lightPosition1.xyz - input.position * lightPosition1.w);
	#ifdef LIGHTNORMALMAPPING
	float3 lightSpotDir12 = normalize(mul(TBN, lightSpotDir1));
	float3 lightDir12 = normalize(mul(TBN, lightDir1));
	#else
	float3 lightSpotDir12 = lightSpotDir1;
	float3 lightDir12 = lightDir1;
	#endif
	lightContribution += doLighting(input.position, normal2, eyeDir2, exponent, specularFactor, lightDir12, lightPosition1, lightDiffuse1, lightSpecular1, lightAttenuation1, lightSpotlight1, lightSpotDir12);
	#endif
	
	#ifdef LIGHT2
	float3 lightSpotDir2 = mul(world, spotlightDir2).xyz;
	float3 lightDir2 = normalize(lightPosition2.xyz - input.position * lightPosition2.w);
	#ifdef LIGHTNORMALMAPPING
	float3 lightSpotDir22 = normalize(mul(TBN, lightSpotDir2));
	float3 lightDir22 = normalize(mul(TBN, lightDir2));
	#else
	float3 lightSpotDir22 = lightSpotDir2;
	float3 lightDir22 = lightDir2;
	#endif
	lightContribution += doLighting(input.position, normal2, eyeDir2, exponent, specularFactor, lightDir22, lightPosition2, lightDiffuse2, lightSpecular2, lightAttenuation2, lightSpotlight2, lightSpotDir22);
	#endif
	
	float4 oColor = saturate(textColour * (lightContribution + ambient));
	#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
	oColor = lerp(oColor, fogColour, input.normalAndFogVal.w);
	#endif
	return oColor;
}