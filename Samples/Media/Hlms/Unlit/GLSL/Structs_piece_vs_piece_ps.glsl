@property( hlms_shadowcaster )
@piece( PassDecl )
//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader
	vec2 depthRange;
} pass;
@end
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
	uvec4 materialIdx[4096];
} instance;
@end

@piece( VStoPS_block )
	@property( !hlms_shadowcaster )
		flat uint drawId;
		@property( hlms_colour )vec4 colour;@end
		@foreach( out_uv_count, n )
			vec@value( out_uv_count@n ) uv@n;@end
	@end
	@property( hlms_shadowcaster )	float depth;@end
@end
