/**
*	Modified by: Juan Camilo Acosta Arango (ja0335 )
*	Date: 09-04-2014
*	Note: This shaders are based one my study over the 
* 	Eat3D course, "Shader Production - Writing Custom Shaders with CGFX"
*	http://eat3d.com/shaders_intro
**/

/////////////
// GLOBALS //
/////////////
Texture2D g_NormalTxt : register( t0 ); // normal
Texture2D g_DiffuseTxt : register( t1 ); // diffuse
Texture2D g_SpecularTxt : register( t2 ); // specular
SamplerState g_samLinear : register( s0 );

cbuffer cbVertexBuffer
{
    matrix g_WorldViewprojMatrix;
    matrix g_WorldInverseTranspose;
    matrix g_World;
    matrix g_InverseView;
};

cbuffer cbPixelBuffer
{
	float4 g_AmbientLightColour;
	float3 g_LightDirection;
	float3 g_LightDiffuseColour;
	float3 g_DiffuseColour;
	float3 g_SpecularColor;
	float3 g_FresnelColor;
	float g_SpecularPower;
	float g_FresnelPower;
};


//////////////
// TYPEDEFS //
//////////////
struct ambient_a2v
{
    float4 position 		: POSITION;
};

struct ambient_v2p
{
    float4 position 		: SV_POSITION;
};

struct perlight_a2v
{
    float4 position 		: POSITION;
	float2 texCoord  		: TEXCOORD0;
	float4 normal    		: NORMAL;
    float4 binormal  		: BINORMAL;
    float4 tangent   		: TANGENT;	
};

struct perlight_v2p
{
    float4 position 		: SV_POSITION;
	float2 texCoord		   	: TEXCOORD0;
	float3 worldNormal     	: TEXCOORD1;
	float3 worldBinormal   	: TEXCOORD2;
	float3 worldTangent    	: TEXCOORD3;		
	float3 eyeVector 	   	: TEXCOORD4;
};

//===============================================================
// AMBIENT
ambient_v2p ambient_color_vs(ambient_a2v In)
{
	ambient_v2p Out;
	
	Out.position = mul(g_WorldViewprojMatrix, In.position);
	
	return Out;
}

float4 ambient_color_ps(ambient_v2p In) : SV_TARGET
{
	return g_AmbientLightColour;
}

//===============================================================
// PER-LIGHT
perlight_v2p perlight_color_vs(perlight_a2v In)
{
	perlight_v2p Out;
	
	Out.position 		= mul(g_WorldViewprojMatrix, In.position);
	Out.worldNormal 	= mul(g_WorldInverseTranspose, In.normal).xyz;
	Out.worldBinormal   = mul(g_WorldInverseTranspose, In.binormal).xyz;
    Out.worldTangent    = mul(g_WorldInverseTranspose, In.tangent).xyz;
	
	float3 worldSpacePos 	= mul(g_World, In.position);
	float3 worldCameraPos	= float3(g_InverseView[0].w, g_InverseView[1].w, g_InverseView[2].w);
	
	Out.eyeVector 	= worldCameraPos - worldSpacePos;
	Out.texCoord 	= In.texCoord;
	
	return Out;
}

float4 perlight_color_ps(perlight_v2p In) : SV_TARGET
{
	float4 outColor;
	float3 worldNormal = g_NormalTxt.Sample( g_samLinear, In.texCoord ) * 2 - 1;
	worldNormal = normalize((worldNormal.x*In.worldTangent)+(worldNormal.y*In.worldBinormal)+(worldNormal.z*In.worldNormal));
	
	float3 lightDir = normalize(-g_LightDirection);
	float3 eyeVector = normalize(In.eyeVector);
	float3 reflectionVector = reflect(eyeVector, worldNormal)*-1;
	
	float4 diffuseMap = g_DiffuseTxt.Sample( g_samLinear, In.texCoord );
	float3 lambert = saturate(dot(lightDir, worldNormal)) * g_LightDiffuseColour;
	
	float4 specularMap = g_SpecularTxt.Sample( g_samLinear, In.texCoord );
	float3 specular = pow(saturate(dot(reflectionVector, lightDir)), g_SpecularPower) * g_SpecularColor;
	float3 fresnel = pow(1- saturate(dot(eyeVector, worldNormal)), g_FresnelPower) * g_FresnelColor;	
	float3 totalSpec = (fresnel * specularMap.r) + (specular * specularMap.g) ;
	
	outColor.rgb = (lambert + g_AmbientLightColour) * (diffuseMap.rgb * g_DiffuseColour) + totalSpec;
	outColor.a = 1.0f;
	
	return outColor;
}