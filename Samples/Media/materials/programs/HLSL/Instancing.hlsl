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

v2ptc instancing_vp(uniform float3x4   worldMatrix3x4Array[80], float4 position : POSITION,
						float3 normal : NORMAL,
						float2 uv : TEXCOORD0,
						float index : TEXCOORD1,
						uniform float4 lightPos,
						uniform float4 ambient,
						uniform float4 lightDiffuseColour
)
{
	v2ptc output;
	// transform by indexed matrix
	float4 transformedPos = float4(mul(worldMatrix3x4Array[index], position).xyz, 1.0);
	
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, transformedPos);
	output.oUv = uv;

	float3 norm = mul((float3x3)worldMatrix3x4Array[index], normal);
	
	float3 lightDir = 	normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));

	output.Colour = ambient + saturate(dot(lightDir, norm)) * lightDiffuseColour;
	
	return output;
}

/*
  Instancing shadow-caster pass
*/
v2ptc instancingCaster_vp(
	float4 position : POSITION,
	float3 normal   : NORMAL,
	float2 uv       : TEXCOORD0,
	float index     : TEXCOORD1,

	// Support up to 80 bones of float3x4
	uniform float3x4   worldMatrix3x4Array[80],
	uniform float4   ambient)
{
	v2ptc output;
	// transform by indexed matrix
	float4 transformedPos = float4(mul(worldMatrix3x4Array[index], position).xyz, 1.0);

	// view / projection
	output.oPosition = mul(viewProjectionMatrix, transformedPos);
	output.oUv = uv;
	output.Colour = ambient;
	return output;
}
