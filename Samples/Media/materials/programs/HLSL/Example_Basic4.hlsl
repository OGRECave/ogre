struct a2v
{
    float4 position : POSITION;
    float2 uv	: TEXCOORD0;
};

struct v2p
{
	float4 oPosition : SV_POSITION;
	float2 oUv : TEXCOORD0;
	float4 colour   : COLOR;
};

/*
  Basic ambient lighting vertex program
*/
v2p ambientOneTexture_vp(a2v input,
						  
						  uniform float4x4 worldViewProj,
						  uniform float4 ambient)
{
	v2p output;
	output.oPosition = mul(worldViewProj, input.position);
	output.oUv = input.uv;
	output.colour = ambient;
	
	return output;
}

struct a2vhardwareSkinningOneWeight
{
	float4 position : POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float  blendIdx : BLENDINDICES;
};

/*
  Single-weight-per-vertex hardware skinning, 2 lights
  The trouble with vertex programs is they're not general purpose, but
  fixed function hardware skinning is very poorly supported
*/

v2p hardwareSkinningOneWeight_vp(
	a2vhardwareSkinningOneWeight input,
	
	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   lightPos[2],
	uniform float4   lightDiffuseColour[2],
	uniform float4   ambient)
{
	v2p output;
	// transform by indexed matrix
	float3 norm;
	float3 lightDir0;
	float3 lightDir1;
	float4 blendPos = float4(mul(worldMatrix3x4Array[input.blendIdx], input.position).xyz, 1.0);
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	// transform normal
	norm = mul((float3x3)worldMatrix3x4Array[input.blendIdx], input.normal);
	// Lighting - support point and directional
	lightDir0 = 	normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	lightDir1 = 	normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));

	output.oUv = input.uv;
	output.colour = ambient + 
		(saturate(dot(lightDir0, norm)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, norm)) * lightDiffuseColour[1]);
	return output;
}	

struct a2vhardwareSkinningOneWeightCaster
{
	float4 position : POSITION;
	float3 normal   : NORMAL;
	float  blendIdx : BLENDINDICES;
};

struct v2phardwareSkinningCaster
{
	float4 oPosition : SV_POSITION;
	float4 colour   : COLOR;
};
/*
  Single-weight-per-vertex hardware skinning, shadow-caster pass
*/
v2phardwareSkinningCaster hardwareSkinningOneWeightCaster_vp(
	a2vhardwareSkinningOneWeightCaster input,

	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   ambient)
{
	v2phardwareSkinningCaster output;
	// transform by indexed matrix
	float4 blendPos = float4(mul(worldMatrix3x4Array[input.blendIdx], input.position).xyz, 1.0);
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	
	output.colour = ambient;
	
	return output;
}	

/*
  Two-weight-per-vertex hardware skinning, 2 lights
  The trouble with vertex programs is they're not general purpose, but
  fixed function hardware skinning is very poorly supported
*/
struct a2vhardwareSkinningTwoWeights
{
	float4 position : POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float4 blendIdx : BLENDINDICES;
	float4 blendWgt : BLENDWEIGHT;
};

v2p hardwareSkinningTwoWeights_vp(
	a2vhardwareSkinningTwoWeights input,
	

	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   lightPos[2],
	uniform float4   lightDiffuseColour[2],
	uniform float4   ambient)
{
	v2p output;

	// transform by indexed matrix
	float3 norm;
	float3 lightDir0;
	float3 lightDir1;
	int i;
	float4 blendPos = float4(0,0,0,0);
	for (i = 0; i < 2; ++i)
	{
		blendPos += float4(mul(worldMatrix3x4Array[input.blendIdx[i]], input.position).xyz, 1.0) * input.blendWgt[i];
	}
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	// transform normal
	norm = float3(0,0,0);
	for (i = 0; i < 2; ++i)
	{
		norm += mul((float3x3)worldMatrix3x4Array[input.blendIdx[i]], input.normal) * 
		input.blendWgt[i];
	}
	norm = normalize(norm);
	// Lighting - support point and directional
	lightDir0 = 	normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	lightDir1 = 	normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));

	
	output.oUv = input.uv;
	output.colour = ambient + 
		(saturate(dot(lightDir0, norm)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, norm)) * lightDiffuseColour[1]);
	return output;
}

/*
  Two-weight-per-vertex hardware skinning, shadow caster pass
*/
v2phardwareSkinningCaster hardwareSkinningTwoWeightsCaster_vp(
	a2vhardwareSkinningTwoWeights input,
	
	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   ambient)
{
	v2phardwareSkinningCaster output;
	// transform by indexed matrix
	int i;
	float4 blendPos = float4(0,0,0,0);
	for (i = 0; i < 2; ++i)
	{
		blendPos += float4(mul(worldMatrix3x4Array[input.blendIdx[i]], input.position).xyz, 1.0) * input.blendWgt[i];
	}
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	

	output.colour = ambient;
		
	return output;
}


/*
  Four-weight-per-vertex hardware skinning, 2 lights
  The trouble with vertex programs is they're not general purpose, but
  fixed function hardware skinning is very poorly supported
*/
v2p hardwareSkinningFourWeights_vp(
	a2vhardwareSkinningTwoWeights input,
	
	// Support up to 24 bones of float3x4
	// vs_1_1 only supports 96 params so more than this is not feasible
	uniform float3x4   worldMatrix3x4Array[24],
	uniform float4x4 viewProjectionMatrix,
	uniform float4   lightPos[2],
	uniform float4   lightDiffuseColour[2],
	uniform float4   ambient)
{
	v2p output;
	// transform by indexed matrix
	float3 norm;
	float3 lightDir0;
	float3 lightDir1;
	int i;
	float4 blendPos = float4(0,0,0,0);
	for (i = 0; i < 4; ++i)
	{
		blendPos += float4(mul(worldMatrix3x4Array[input.blendIdx[i]], input.position).xyz, 1.0) * input.blendWgt[i];
	}
	// view / projection
	output.oPosition = mul(viewProjectionMatrix, blendPos);
	// transform normal
	norm = float3(0,0,0);
	for (i = 0; i < 4; ++i)
	{
		norm += mul((float3x3)worldMatrix3x4Array[input.blendIdx[i]], input.normal) * 
		input.blendWgt[i];
	}
	norm = normalize(norm);
	// Lighting - support point and directional
	lightDir0 = 	normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	lightDir1 = 	normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));

	
	output.oUv = input.uv;
	output.colour = ambient + 
		(saturate(dot(lightDir0, norm)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, norm)) * lightDiffuseColour[1]);
	return output;
}

struct a2vhardwareMorphAnimation
{
	float3 pos1   : POSITION;
	float4 normal : NORMAL;
	float2 uv	  : TEXCOORD0;
	float3 pos2	  : TEXCOORD1;
};

v2p hardwareMorphAnimation(
				a2vhardwareMorphAnimation input,

			  uniform float4x4 worldViewProj, 
			  uniform float4 anim_t)
{
	v2p output;
	// interpolate
	float4 interp = float4(input.pos1 + anim_t.x*(input.pos2 - input.pos1), 1.0f);
	
	output.oPosition = mul(worldViewProj, interp);
	output.oUv = input.uv;
	output.colour = float4(1,0,0,1);
	return output;
}

struct a2vhardwarePoseAnimation
{
	float3 pos : POSITION;
	float4 normal	  : NORMAL;
	float2 uv		  : TEXCOORD0;
	float3 pose1	  : TEXCOORD1;
	float3 pose2	  : TEXCOORD2;
};

v2p hardwarePoseAnimation(
			  a2vhardwarePoseAnimation input,

			  uniform float4x4 worldViewProj, 
			  uniform float4 anim_t)
{
	// interpolate
	v2p output;
	float4 interp = float4(input.pos + anim_t.x*input.pose1 + anim_t.y*input.pose2, 1.0f);
	
	output.oPosition = mul(worldViewProj, interp);
	output.oUv = input.uv;
	output.colour = float4(1,0,0,1);
	return output;
}
