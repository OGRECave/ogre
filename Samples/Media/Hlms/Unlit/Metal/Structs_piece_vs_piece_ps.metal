@piece( PassStructDecl )
//Uniforms that change per pass
struct PassData
{
//Vertex shader
float4x4 viewProj[2];
@property( hlms_shadowcaster )
	float4 depthRange;
@end
@insertpiece( custom_passBuffer )
};@end

@piece( PassDecl )
, constant PassData &pass [[buffer(CONST_SLOT_START+0)]]
@end

@piece( MaterialStructDecl )
struct Material
{
	float4 alpha_test_threshold;
	float4 diffuse;

	@foreach( 16, n )
		ushort diffuseIdx@n;@end
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
//Contains 0 or 1 to index into pass.viewProj[]. Only used
//if hlms_identity_viewproj_dynamic is set.
, constant uint4 *materialIdx [[buffer(CONST_SLOT_START+2)]]
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
	@property( hlms_shadowcaster )	float depth;@end
	@insertpiece( custom_VStoPS )
@end
