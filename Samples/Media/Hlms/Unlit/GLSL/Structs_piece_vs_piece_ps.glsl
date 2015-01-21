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
	uint materialIdx[4096];
} instance;
@end

@piece( VStoPS_block )
	flat uint drawId;
	@property( hlms_colour )vec4 colour;@end
	@foreach( out_uv_count, n )
		vec@value( hlms_uv_count@n ) uv@n;@end
@end
