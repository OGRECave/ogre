struct VS_INPUT
{
	float4 Position	:	SV_POSITION;
	float3 Normal	:	NORMAL;
	float3 Tangent	:	TANGENT;
#ifdef BONE_TWO_WEIGHTS
	float4 weights		: 	BLENDWEIGHT;
#endif
	float2 uv0		:	TEXCOORD0;

	float4 m01		:	TEXCOORD1;
	float4 m23		:	TEXCOORD2;
};

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

struct VS_OUTPUT
{
	float4 Position	:	SV_POSITION;
	PS_INPUT	ps;
};

#define SHADOW_BIAS 0

#ifdef ST_DUAL_QUATERNION
#include "DualQuaternion_Common.hlsl"
#endif

#define LOD 0
SamplerState SampleType;

VS_OUTPUT main_vs( VS_INPUT input,
				   uniform float4x4 viewProjMatrix,
				   
#ifdef DEPTH_SHADOWCASTER
				   uniform Texture2D matrixTexture : register(t0)
#else
				   uniform Texture2D matrixTexture : register(t2)
#endif

#if defined( DEPTH_SHADOWCASTER ) || defined( DEPTH_SHADOWRECEIVER )
				,  uniform float4 depthRange
#endif
#ifdef DEPTH_SHADOWRECEIVER
				,  uniform float4x4 texViewProjMatrix
#endif
				)
{
	VS_OUTPUT output;

	float4 worldPos	 = 0;
	float3 worldNorm = 0;

#ifdef ST_DUAL_QUATERNION
	float2x4 blendDQ;	
	blendDQ[0] = matrixTexture.SampleLevel(SampleType, input.m01.xy, LOD );
	blendDQ[1] = matrixTexture.SampleLevel(SampleType, input.m01.zw, LOD );
#ifdef BONE_TWO_WEIGHTS
	float2x4 blendDQ2;
	//Use the empty parts of m03, z and w, for the second dual quaternion
	blendDQ2[0] = matrixTexture.SampleLevel(SampleType, input.m23.xy, LOD );
	blendDQ2[1] = matrixTexture.SampleLevel(SampleType, input.m23.zw, LOD);
	
	//Accurate antipodality handling. For speed increase, remove the following line
	if (dot(blendDQ[0], blendDQ2[0]) < 0.0) blendDQ2 *= -1.0;
	
	//Blend the dual quaternions based on the weights
	blendDQ *= input.weights.x;
	blendDQ += input.weights.y*blendDQ2;
	//Normalize the resultant dual quaternion
	blendDQ /= length(blendDQ[0]);
#endif
	//Only dealing with one weight so normalization of the dual quaternion and weighting are unnecessary
	worldPos = float4(calculateBlendPosition(input.Position.xyz, blendDQ), 1.0);
	worldNorm = calculateBlendNormal(input.Normal.xyz, blendDQ);
#else
	float3x4 worldMatrix;
	worldMatrix[0] = matrixTexture.SampleLevel(SampleType, float2(input.m01.xy), LOD );
	worldMatrix[1] = matrixTexture.SampleLevel(SampleType, input.m01.zw, LOD );
	worldMatrix[2] = matrixTexture.SampleLevel(SampleType, input.m23.xy, LOD );

	worldPos = float4( mul( worldMatrix, input.Position ).xyz, 1.0f );
	worldNorm= mul( (float3x3)(worldMatrix), input.Normal );
#endif

	//Transform the position
	output.Position		= mul( viewProjMatrix, worldPos );

#ifdef DEPTH_SHADOWCASTER
	output.ps.unused	= float3( 0, 0, 0 );
	output.ps.depth		= (output.Position.z - depthRange.x + SHADOW_BIAS) * depthRange.w;
#else
	output.ps.uv0		= input.uv0;

	//Pass Normal and position for Blinn Phong lighting
	output.ps.Normal	= normalize(worldNorm);
	output.ps.vPos		= worldPos.xyz;
	
	#ifdef DEPTH_SHADOWRECEIVER
		// Calculate the position of vertex in light space to do shadows
		output.ps.lightSpacePos = mul( texViewProjMatrix, worldPos );
		// make linear
		output.ps.lightSpacePos.z = (output.ps.lightSpacePos.z - depthRange.x) * depthRange.w;
	#endif
#endif

	return output;
}
