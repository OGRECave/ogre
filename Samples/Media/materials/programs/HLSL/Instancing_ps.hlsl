struct PS_INPUT
{
    float4 oPosition : SV_POSITION;
    float4 Colour : COLOR;
#ifdef DEPTH_SHADOWCASTER
	float3 unused	:	TEXCOORD0;
	float depth		:	TEXCOORD1;
#else
	float2 uv0		:	TEXCOORD0;
	float3 Normal	:	TEXCOORD1;
	float3 vPos		:	TEXCOORD2;
	
	#ifdef DEPTH_SHADOWRECEIVER
		float4 lightSpacePos	:	TEXCOORD3;
	#endif
#endif
};

#define SHADOW_BIAS 0

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

#include "shadows.hlsl"

//---------------------------------------------
//Main Pixel Shader
//---------------------------------------------
half4 main_ps( PS_INPUT input ,
				uniform float4	lightPosition,
				uniform float3	cameraPosition,
				uniform half3 	lightAmbient,
				uniform half3	lightDiffuse,
				uniform half3	lightSpecular,
				uniform half4	lightAttenuation,				
				uniform half	lightGloss,

				//Textures
				uniform Texture2D diffuseMap : register(s0)

#ifdef DEPTH_SHADOWRECEIVER
			,	uniform float invShadowMapSize,
				uniform Texture2D shadowMap : register(s1)
#endif
			) : SV_Target
{
	float fShadow = 1.0f;
#ifdef DEPTH_SHADOWRECEIVER
	fShadow = calcDepthShadow( shadowMap, input.lightSpacePos, invShadowMapSize );
#endif

	const half4 baseColour = diffuseMap.Sample(g_samLinear, input.uv0 );
	
	//Blinn-Phong lighting
	const half3 normal		= normalize( input.Normal );
	half3 lightDir			= lightPosition.xyz - input.vPos * lightPosition.w;
	half3 eyeDir			= normalize( cameraPosition - input.vPos  );

	const half fLength		= length( lightDir );
	lightDir				= normalize( lightDir );

	const half NdotL = max( 0.0f, dot( normal, lightDir ) );
	half3 halfVector = normalize(lightDir + eyeDir);
	const half HdotN = max( 0.0f, dot( halfVector, normal ) );
	
	const half3 ambient  = lightAmbient * baseColour.xyz;
	const half3 diffuse  = lightDiffuse * NdotL * baseColour.xyz;
	const half3 specular = lightSpecular * pow( HdotN, lightGloss );
	
	const half3 directLighting = (diffuse + specular) * fShadow;
	
	return half4( directLighting + ambient, baseColour.a );
}