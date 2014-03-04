cbuffer MatrixBuffer
{
	matrix viewProjectionMatrix;
};

struct v2ptc
{
    float4 oPosition : SV_POSITION;
    float2 oUv : TEXCOORD0;
	float4 Colour : COLOR;
};

struct v2pc
{
    float4 oPosition : SV_POSITION;
    float4 Colour : COLOR;
};

struct vs_input
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float index : TEXCOORD1;
};

v2ptc instancing_vp(	vs_input input,
						uniform float3x4   worldMatrix3x4Array[80],
						uniform float4 lightPos,
						uniform float4 ambient,
						uniform float4 lightDiffuseColour
)
{
	v2ptc output;
	// transform by indexed matrix
	float4 transformedPos = float4(mul(worldMatrix3x4Array[input.index], input.position).xyz, 1.0);
	
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, transformedPos);
	output.oUv = input.uv;

	float3 norm = mul((float3x3)worldMatrix3x4Array[input.index], input.normal);
	
	float3 lightDir = 	normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));

	output.Colour = ambient + saturate(dot(lightDir, norm)) * lightDiffuseColour;
	
	return output;
}

/*
  Instancing shadow-caster pass
*/
v2ptc instancingCaster_vp(
	vs_input input,

	// Support up to 80 bones of float3x4
	uniform float3x4   worldMatrix3x4Array[80],
	uniform float4   ambient)
{
	v2ptc output;
	// transform by indexed matrix
	float4 transformedPos = float4(mul(worldMatrix3x4Array[input.index], input.position).xyz, 1.0);

	// view / projection
	output.oPosition = mul(viewProjectionMatrix, transformedPos);
	output.oUv = input.uv;
	output.Colour = ambient;
	return output;
}

struct crowd_vp_input
{
	float4 position : SV_POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float4  blendIdx : BLENDINDICES;
	float4	blendWgt : BLENDWEIGHT;
	float   index : TEXCOORD1;
};

v2ptc crowd_vp(
	// Support up to 20 bones of float3x4
	// vs_2_0 only supports 256 params so more than this is not feasible
	crowd_vp_input input,
	uniform float4x4 viewProjectionMatrix,
	uniform float numBones,
	uniform float3x4   worldMatrix3x4Array[80],
	uniform float4 lightDiffuseColour,
	uniform float4 ambient,
	uniform float4 lightPos)
{
	v2ptc output;
	// transform by indexed matrix
	float4 blendPos = float4(0,0,0,0);
	int i;
	for (i = 0; i < 4; ++i)
	{
		blendPos += float4(mul(worldMatrix3x4Array[input.index*numBones+input.blendIdx[i]], input.position).xyz, 1.0) * input.blendWgt[i];
	}
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	output.oUv = input.uv;
	float3 norm = float3(0,0,0);
	for (i = 0; i < 4; ++i)
	{
		norm += mul((float3x3)worldMatrix3x4Array[input.index*numBones+input.blendIdx[i]], input.normal)* input.blendWgt[i];
	}
	float3 lightDir = 	normalize(
		lightPos.xyz -  (blendPos.xyz * lightPos.w));

	output.Colour = ambient + saturate(dot(lightDir, norm)) * lightDiffuseColour;
	return output;	
}

/*
  Single-weight-per-vertex hardware skinning, shadow-caster pass
*/
struct crowdCaster_input
{
	float4 position : SV_POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float  blendIdx : BLENDINDICES;
	float index     : TEXCOORD1;
};

v2ptc crowdCaster_vp(
	crowdCaster_input input,
	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[80],
	uniform float numBones,
	uniform float4   ambient)
{
	v2ptc output;
	// transform by indexed matrix
	float4 blendPos = float4(mul(worldMatrix3x4Array[input.index*numBones+input.blendIdx], input.position).xyz, 1.0);

	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	output.oUv = input.uv;
	output.Colour = ambient;
	return output;
}	