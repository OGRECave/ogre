@piece( PassStructDecl )
//Uniforms that change per pass
struct PassData
{
	@insertpiece( PassInternalDecl )
};@end

@piece( PassDecl )
, constant PassData &passBuf [[buffer(CONST_SLOT_START+0)]]
@end

@piece( MaterialStructDecl )
struct Material
{
	float4 alpha_test_threshold;
	float4 diffuse;

	@foreach( 16, n )
		ushort diffuseIdx@n;@end

	@insertpiece( custom_materialBuffer )
};@end

@piece( MaterialDecl )
, constant Material *materialArray [[buffer(CONST_SLOT_START+1)]]
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
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
, constant uint4 *worldMaterialIdx [[buffer(CONST_SLOT_START+2)]]
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( VStoPS_block )
	@property( !hlms_shadowcaster )
		ushort materialId [[flat]];
		@property( hlms_colour )float4 colour;@end
		@foreach( out_uv_half_count, n )
			float@value( out_uv_half_count@n ) uv@n;@end
	@end
	@property( hlms_shadowcaster )
		@property( (!hlms_shadow_uses_depth_texture || exponential_shadow_maps) && !hlms_shadowcaster_point )
			float depth;
		@end
		@property( hlms_shadowcaster_point )
			float3 toCameraWS;
			@property( !exponential_shadow_maps )
				float constBias [[flat]];
			@end
		@end
	@end
	@insertpiece( custom_VStoPS )
@end
