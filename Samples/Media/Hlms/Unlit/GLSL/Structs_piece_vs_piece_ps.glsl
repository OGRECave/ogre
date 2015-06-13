@piece( PassDecl )
//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader
	mat4 viewProj[2];
	@property( hlms_shadowcaster )
		vec4 depthRange;
	@end
	@insertpiece( custom_passBuffer )
} pass;
@end

@piece( MaterialDecl )
struct Material
{
	vec4 alpha_test_threshold;
	vec4 diffuse;

	uvec4 indices0_3;
	uvec4 indices4_7;
};

layout(binding = 1) uniform MaterialBuf
{
	Material m[@insertpiece( materials_per_buffer )];
} materialArray;
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
layout(binding = 2) uniform InstanceBuffer
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
	//Contains 0 or 1 to index into pass.viewProj[]. Only used
	//if hlms_identity_viewproj_dynamic is set.
	uvec4 materialIdx[4096];
} instance;
@end

@piece( VStoPS_block )
	@property( !hlms_shadowcaster )
		flat uint drawId;
		@property( hlms_colour )vec4 colour;@end
		@foreach( out_uv_half_count, n )
			vec@value( out_uv_half_count@n ) uv@n;@end
	@end
	@property( hlms_shadowcaster && !hlms_shadow_uses_depth_texture )	float depth;@end
	@insertpiece( custom_VStoPS )
@end
