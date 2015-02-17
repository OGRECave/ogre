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
	float4 position : SV_POSITION;
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
	float4 position : SV_POSITION;
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
	float4 position : SV_POSITION;
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
	uniform float4   ambient,
	uniform float4   diffuse)
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
	output.colour = diffuse * (ambient + 
		(saturate(dot(lightDir0, norm)) * lightDiffuseColour[0]) + 
		(saturate(dot(lightDir1, norm)) * lightDiffuseColour[1]));
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
	float3 pos1   : SV_POSITION;
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
	float3 pos : SV_POSITION;
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

struct hardwareMorphAnimationWithNormals_in
{
	float3 pos1 : SV_POSITION;
	float3 normal1  : NORMAL;
	float2 uv		  : TEXCOORD0;
	float3 pos2	  : TEXCOORD1;
	float3 normal2  : TEXCOORD2;
};


// hardware morph animation (with normals)
v2p hardwareMorphAnimationWithNormals(
			  hardwareMorphAnimationWithNormals_in input,
			  uniform float4x4 worldViewProj, 
			  uniform float4 objSpaceLightPos,
			  uniform float4 ambient,
			  uniform float4 anim_t)
{
	v2p output;
	// interpolate position
	float4 posinterp = float4(input.pos1 + anim_t.x*(input.pos2 - input.pos1), 1.0f);

	// nlerp normal
	float3 ninterp = input.normal1 + anim_t.x*(input.normal2 - input.normal1);
	ninterp = normalize(ninterp);
	
	output.oPosition = mul(worldViewProj, posinterp);
	output.oUv = input.uv;
	
	float3 lightDir = normalize(
		objSpaceLightPos.xyz -  (posinterp.xyz * objSpaceLightPos.w));

	// Colour it red to make it easy to identify
	float lit = saturate(dot(lightDir, ninterp));
	output.colour = float4((ambient.rgb + float3(lit,lit,lit)) * float3(1,0,0), 1);
	return output;
}

struct hardwarePoseAnimationWithNormals_in
{
	float3 pos : SV_POSITION;
	float3 normal	   : NORMAL;
	float2 uv		   : TEXCOORD0;
	float3 pose1pos  : TEXCOORD1;
	float3 pose1norm : TEXCOORD2;
	float3 pose2pos  : TEXCOORD3;
	float3 pose2norm : TEXCOORD4;
};

// hardware pose animation (with normals)
v2p hardwarePoseAnimationWithNormals(
			  hardwarePoseAnimationWithNormals_in input,

			  uniform float4x4 worldViewProj, 
			  uniform float4 objSpaceLightPos,
			  uniform float4 ambient,
			  uniform float4 anim_t)
{
	v2p output;
	// interpolate
	float4 posinterp = float4(input.pos + anim_t.x*input.pose1pos + anim_t.y*input.pose2pos, 1.0f);
	
	// nlerp normal
	// First apply the pose normals (these are actual normals, not offsets)
	float3 ninterp = anim_t.x*input.pose1norm + anim_t.y*input.pose2norm;

	// Now add back any influence of the original normal
	// This depends on what the cumulative weighting left the normal at, if it's lacking or cancelled out
	//float remainder = 1.0 - min(anim_t.x + anim_t.y, 1.0);
	float remainder = 1.0 - min(length(ninterp), 1.0);
	ninterp = ninterp + (input.normal * remainder);
	ninterp = normalize(ninterp);

	output.oPosition = mul(worldViewProj, posinterp);
	output.oUv = input.uv;
	
	float3 lightDir = normalize(
		objSpaceLightPos.xyz -  (posinterp.xyz * objSpaceLightPos.w));

	// Colour it red to make it easy to identify
	float lit = saturate(dot(lightDir, ninterp));
	output.colour = float4((ambient.rgb + float3(lit,lit,lit)) * float3(1,0,0), 1);
	return output;
}

struct basic1
{
	float4 position : SV_POSITION;
	float3 tangent       : TANGENT;
};

struct basic1out
{
    float4 oPosition : SV_POSITION;
	float3 oTangent  : TEXCOORD0;
};

basic1out basicPassthroughTangent_v(basic1 input,
						  uniform float4x4 worldViewProj)
{
	basic1out output;

	output.oPosition = mul(worldViewProj, input.position);
	output.oTangent = input.tangent;
	
	return output;
}

struct basic2
{
	float4 position : SV_POSITION;
	float3 normal       : NORMAL;
};

struct basic2out
{
	float4 oPosition : SV_POSITION;
	float3 oNormal  : TEXCOORD0;
};

basic2out basicPassthroughNormal_v(basic2 input,
						  uniform float4x4 worldViewProj)
{
	basic2out output;
	output.oPosition = mul(worldViewProj, input.position);
	output.oNormal = input.normal;
	return output;
}
// Basic fragment program to display UV
float4 showuv_p (float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
	// wrap values using frac
	return float4(frac(uv.x), frac(uv.y), 0, 1);
}
// Basic fragment program to display 3d uv
float4 showuvdir3d_p (float4 position : SV_POSITION, float3 uv : TEXCOORD0) : SV_Target
{
	float3 n = normalize(uv);
	return float4(n.x, n.y, n.z, 1);
}

/*
  Basic fragment program using texture and diffuse colour.
*/
struct diffuse_in
{
	float4 position          : SV_POSITION;
	float2 uv                : TEXCOORD0;
	float4 diffuse           : COLOR;
};

SamplerState MySampler;

float4 diffuseOneTexture_fp(diffuse_in input,
						  uniform Texture2D texMap : register(t0)) : SV_Target
{
	float4 colour = texMap.Sample(MySampler,input.uv) * input.diffuse;
	return colour;
}