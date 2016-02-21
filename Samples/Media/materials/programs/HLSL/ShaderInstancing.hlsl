//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------

//---------------------------------------------
//Vertex Shader Input
//---------------------------------------------
struct VS_INPUT
{
	float4 Position	:	POSITION;
	float3 Normal	:	NORMAL;
	float3 Tangent	:	TANGENT;
	float2 uv0	:	TEXCOORD0;

	float4 BlendIdx	:	BLENDINDICES;
	float4 BlendWgt	:	BLENDWEIGHT;
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
#define LOD 0

#ifdef ST_DUAL_QUATERNION
#include "DualQuaternion_Common.hlsl"
#endif

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
VS_OUTPUT main_vs( VS_INPUT input,
				   uniform float4x4 viewProjMatrix,
#ifdef ST_DUAL_QUATERNION
				   uniform float2x4 worldDualQuaternion2x4Array[120]
#else
				   uniform float3x4 worldMatrix3x4Array[80]
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

	int idx = int(input.BlendIdx[0]);
#ifdef ST_DUAL_QUATERNION
	//Only dealing with one weight so normalization of the dual quaternion and weighting are unnecessary
	float2x4 blendDQ = worldDualQuaternion2x4Array[idx];
#ifdef BONE_TWO_WEIGHTS
	float2x4 blendDQ2 = worldDualQuaternion2x4Array[int(input.BlendIdx[1])];
	
	//Accurate antipodality handling. For speed increase, remove the following line
	if (dot(blendDQ[0], blendDQ2[0]) < 0.0) blendDQ2 *= -1.0;
	
	//Blend the dual quaternions based on the weights
	blendDQ *= input.BlendWgt.x;
	blendDQ += input.BlendWgt.y*blendDQ2;
	//Normalize the resultant dual quaternion
	blendDQ /= length(blendDQ[0]);
#endif
	worldPos = float4(calculateBlendPosition(input.Position.xyz, blendDQ), 1.0);
	worldNorm = calculateBlendNormal(input.Normal, blendDQ);
#else
	worldPos  = float4( mul( worldMatrix3x4Array[idx], input.Position ).xyz, 1.0f );
	worldNorm = mul( (float3x3)(worldMatrix3x4Array[idx]), input.Normal );
#endif

	/*int i;
	for( i=0; i<4; i++ )
	{
		int idx = int(input.BlendIdx[0]);
		worldPos += float4( mul( worldMatrix3x4Array[idx], input.Position ).xyz, 1.0f ) * input.BlendWgt[i];
		worldNorm += mul( (float3x3)(worldMatrix3x4Array[idx]), input.Normal ) * input.BlendWgt[i];
	}*/

	//Transform the position
	output.Position		= mul( viewProjMatrix, worldPos );
	
#ifdef DEPTH_SHADOWCASTER
	output.ps.unused	= float3( 0, 0, 0);
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
