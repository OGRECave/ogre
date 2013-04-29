struct PS_INPUT
{
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

//---------------------------------------------
//Vertex Shader Output
//---------------------------------------------
struct VS_OUTPUT
{
	float4 Position	:	POSITION;
	PS_INPUT	ps;
};

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

#include "shadows.hlsl"

#define BIAS 0

struct VS_OUT
{
    float4  pos			  : POSITION;   
    float3  diffuseUV     : TEXCOORD0;  // xy = UV, z = fog/depth
#if !SHADOWCASTER
	float3	col			  : COLOR;
#endif
#if DEPTH_SHADOWCASTER
	float	depth		  : TEXCOORD1;
#endif
#if DEPTH_SHADOWRECEIVER
	float4 lightSpacePos0	: TEXCOORD2;
	float4 lightSpacePos1	: TEXCOORD3;
	float4 lightSpacePos2	: TEXCOORD4;
#endif
};

VS_OUT main_vp(
	float4   pos	: POSITION,     
    float3   normal	: NORMAL,    
    float4   uv0	: TEXCOORD0,

   	uniform float4x4 worldViewProj,
	uniform float4 lightPosition,
	uniform float3 lightDiffuse
#if FOG
	, uniform float2 fogParams		// x = fog start, y = fog distance
#endif
#if DEPTH_SHADOWCASTER
	, uniform float4 depthRange // x = min, y = max, z = range, w = 1/range
#elif DEPTH_SHADOWRECEIVER
	, uniform float4 depthRange0 // x = min, y = max, z = range, w = 1/range
	, uniform float4 depthRange1 // x = min, y = max, z = range, w = 1/range
	, uniform float4 depthRange2 // x = min, y = max, z = range, w = 1/range
#endif
#if DEPTH_SHADOWRECEIVER
	, uniform float4x4 texWorldViewProjMatrix0
	, uniform float4x4 texWorldViewProjMatrix1
	, uniform float4x4 texWorldViewProjMatrix2
#endif

	)
{
    VS_OUT outp;

    // project position to the screen
    outp.pos = mul(worldViewProj, pos);

#if !SHADOWCASTER
	// Get object space light direction
	float3 lightDir = normalize(lightPosition.xyz - (pos.xyz * lightPosition.w).xyz);
	outp.col = lightDiffuse.xyz * max(dot(lightDir, normal.xyz), 0.0);
#  if FOG
    outp.diffuseUV.z = linearFog(outp.pos.z, fogParams.x, fogParams.y);
#  endif

#endif

    // pass through other texcoords exactly as they were received
    outp.diffuseUV.xy = uv0.xy;
    

#if DEPTH_SHADOWCASTER
	outp.depth = (BIAS + outp.pos.z - depthRange.x) * depthRange.w;
#endif

#if DEPTH_SHADOWRECEIVER
	// Calculate the position of vertex in light space
	outp.lightSpacePos0 = mul(texWorldViewProjMatrix0, pos);
	outp.lightSpacePos1 = mul(texWorldViewProjMatrix1, pos);
	outp.lightSpacePos2 = mul(texWorldViewProjMatrix2, pos);

	// make linear
	outp.lightSpacePos0.z = (outp.lightSpacePos0.z - depthRange0.x) * depthRange0.w;
	outp.lightSpacePos1.z = (outp.lightSpacePos1.z - depthRange1.x) * depthRange1.w;
	outp.lightSpacePos2.z = (outp.lightSpacePos2.z - depthRange2.x) * depthRange2.w;

	// pass cam depth
	outp.diffuseUV.z = outp.pos.z;

#endif

    return outp;
}


float4 caster_fp(
	VS_OUT In,

	uniform texture2D diffuseMap	: register(s0),
#if DEPTH_SHADOWRECEIVER
	uniform Texture2D shadowMap0 : register(s1),
	uniform Texture2D shadowMap1 : register(s2),
	uniform Texture2D shadowMap2 : register(s3),
#endif

	uniform float3 materialAmbient
	
#if SHADOWCASTER
	, uniform float3 shadowColour
#endif
#if FOG
	, uniform float3 fogColour
#endif
#if DEPTH_SHADOWRECEIVER
	, uniform float inverseShadowmapSize0
	, uniform float inverseShadowmapSize1
	, uniform float inverseShadowmapSize2
	, uniform float4 pssmSplitPoints
#endif

	) : SV_Target
{

    // look up the diffuse map layer
    float4 texDiffuse = diffuseMap.Sample(g_samLinear, In.diffuseUV.xy);
    
#if SHADOWCASTER
#  if DEPTH_SHADOWCASTER
	// early-out with depth (we still include alpha for those cards that support it)
	return float4(In.depth.xxx, 1);
#  else
	return float4(shadowColour.xyz, texDiffuse.a);
#  endif

#else
    // compute the ambient contribution (pulled from the diffuse map)
    float3 vAmbient = texDiffuse.xyz * materialAmbient.xyz;
    float3 vColor3 = texDiffuse.rgb * In.col.rgb;

#  if DEPTH_SHADOWRECEIVER
	float camDepth = In.diffuseUV.z;
	float shadow = calcPSSMDepthShadow(shadowMap0, shadowMap1, shadowMap2, 
		In.lightSpacePos0, In.lightSpacePos1, In.lightSpacePos2,
		inverseShadowmapSize0, inverseShadowmapSize1, inverseShadowmapSize2,
		pssmSplitPoints, camDepth);
	vColor3 *= shadow;
	/*
	vAmbient = float3(0,0,0);
	vColor3 = calcPSSMDebugShadow(shadowMap0, shadowMap1, shadowMap2, 
		In.lightSpacePos0, In.lightSpacePos1, In.lightSpacePos2,
		inverseShadowmapSize0, inverseShadowmapSize1, inverseShadowmapSize2,
		pssmSplitPoints, camDepth);
	*/
#  endif    

	float4 vColor;
    vColor = float4(vColor3 + vAmbient, texDiffuse.a);
    
#  if FOG
    // if fog is active, interpolate between the unfogged color and the fog color
    // based on vertex shader fog value
    vColor.rgb = lerp(vColor.rgb, fogColour, In.diffuseUV.z).rgb;
#  endif

    return vColor;
#endif

}