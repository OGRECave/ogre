@piece( PassDecl )
struct ShadowReceiverData
{
	float4x4 texViewProj;
	float2 shadowDepthRange;
	float2 padding;
	float4 invShadowMapSize;
};

struct Light
{
	float3 position;
	float3 diffuse;
	float3 specular;
@property( hlms_num_shadow_maps )
	float3 attenuation;
	float3 spotDirection;
	float3 spotParams;
@end
};

//Uniforms that change per pass
cbuffer PassBuffer : register(b0)
{
	struct PassData
	{
	//Vertex shader (common to both receiver and casters)
	float4x4 viewProj;

@property( !hlms_shadowcaster )
	//Vertex shader
	float4x4 view;
	@property( hlms_num_shadow_maps )ShadowReceiverData shadowRcv[@value(hlms_num_shadow_maps)];@end

	//-------------------------------------------------------------------------

	//Pixel shader
	float3x3 invViewMatCubemap;
	float padding; //Compatibility with GLSL.
@property( hlms_pssm_splits )@foreach( hlms_pssm_splits, n )
	float pssmSplitPoints@n;@end @end
	@property( hlms_lights_spot )Light lights[@value(hlms_lights_spot)];@end
@end @property( hlms_shadowcaster )
	//Vertex shader
	float2 depthRange;
@end

@property( hlms_forward3d )
	//f3dData.x = minDistance;
	//f3dData.y = invMaxDistance;
	//f3dData.z = f3dNumSlicesSub1;
	//f3dData.w = uint cellsPerTableOnGrid0 (floatBitsToUint);
	float4 f3dData;
	float4 f3dGridHWW[@value( hlms_forward3d )];
@end
	} passBuf;
};
@end

@property( fresnel_scalar )@piece( FresnelType )float3@end @piece( FresnelSwizzle )xyz@end @end
@property( !fresnel_scalar )@piece( FresnelType )float@end @piece( FresnelSwizzle )x@end @end

@piece( MaterialDecl )
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	float4 kD; //kD.w is alpha_test_threshold
	float4 kS; //kS.w is roughness
	//Fresnel coefficient, may be per colour component (float3) or scalar (float)
	//F0.w is mNormalMapWeight
	float4 F0;
	float4 normalWeights;
	float4 cDetailWeights;
	float4 detailOffsetScaleD[4];
	float4 detailOffsetScaleN[4];

	uint4 indices0_3;
	uint4 indices4_7;
};

cbuffer MaterialBuf : register(b1)
{
	Material materialArray[@insertpiece( materials_per_buffer )];
};
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
cbuffer InstanceBuffer : register(b2)
{
    //.x =
	//The lower 9 bits contain the material's start index.
    //The higher 23 bits contain the world matrix start index.
    //
    //.y =
    //shadowConstantBias. Send the bias directly to avoid an
    //unnecessary indirection during the shadow mapping pass.
    //Must be loaded with uintBitsToFloat
	uint4 worldMaterialIdx[4096];
};
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( VStoPS_block )
    @property( !hlms_shadowcaster )
		nointerpolation uint drawId	: TEXCOORD@counter(texcoord);
		@property( hlms_normal || hlms_qtangent )
			float3 pos	: TEXCOORD@counter(texcoord);
			float3 normal	: TEXCOORD@counter(texcoord);
			@property( normal_map )float3 tangent	: TEXCOORD@counter(texcoord);
				@property( hlms_qtangent )nointerpolation float biNormalReflection	: TEXCOORD@counter(texcoord);@end
			@end
		@end
		@foreach( hlms_uv_count, n )
			float@value( hlms_uv_count@n ) uv@n	: TEXCOORD@counter(texcoord);@end

		@foreach( hlms_num_shadow_maps, n )
			float4 posL@n	: TEXCOORD@counter(texcoord);@end
	@end
	@property( hlms_shadowcaster || hlms_pssm_splits )	float depth	: TEXCOORD@counter(texcoord);@end
@end
