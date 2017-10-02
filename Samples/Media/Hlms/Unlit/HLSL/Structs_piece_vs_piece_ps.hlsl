@piece( PassDecl )
//Uniforms that change per pass
cbuffer PassBuffer : register(b0)
{
	struct PassData
	{
	@insertpiece( PassInternalDecl )
	} passBuf;
};
@end

@piece( MaterialDecl )
struct Material
{
	float4 alpha_test_threshold;
	float4 diffuse;

	uint4 indices0_3;
	uint4 indices4_7;

	@insertpiece( custom_materialBuffer )
};

cbuffer materialArray : register(b1)
{
	Material materialArray[@value( materials_per_buffer )];
};
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
cbuffer instance : register(b2)
{
	//.x =
	//Contains the material's start index.
    //
    //.y =
    //shadowConstantBias. Send the bias directly to avoid an
    //unnecessary indirection during the shadow mapping pass.
    //Must be loaded with uintBitsToFloat
    //
    //.z =
	//Contains 0 or 1 to index into passBuf.viewProj[]. Only used
	//if hlms_identity_viewproj_dynamic is set.
	@property( fast_shader_build_hack )
		uint4 worldMaterialIdx[2];
	@end @property( !fast_shader_build_hack )
		uint4 worldMaterialIdx[4096];
	@end
};
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( VStoPS_block )
	@property( !hlms_shadowcaster )
		nointerpolation uint drawId	: TEXCOORD@counter(texcoord);
		@property( hlms_colour )float4 colour	: TEXCOORD@counter(texcoord);@end
		@foreach( out_uv_half_count, n )
			float@value( out_uv_half_count@n ) uv@n	: TEXCOORD@counter(texcoord);@end
	@end
	@property( hlms_shadowcaster )
		@property( (!hlms_shadow_uses_depth_texture || exponential_shadow_maps) && !hlms_shadowcaster_point )
			float depth	: TEXCOORD@counter(texcoord);
		@end
		@property( hlms_shadowcaster_point )
			float3 toCameraWS	: TEXCOORD@counter(texcoord);
			@property( !exponential_shadow_maps )
				nointerpolation float constBias	: TEXCOORD@counter(texcoord);
			@end
		@end
	@end
	@insertpiece( custom_VStoPS )
@end
