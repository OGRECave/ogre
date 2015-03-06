@piece( MaterialDecl )
struct Material
{
	float4 alpha_test_threshold;
	float4 diffuse;

	uint4 indices0_3;
	uint4 indices4_7;
};

cbuffer materialArray : register(b1)
{
	Material materialArray[@insertpiece( materials_per_buffer )];
};
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
cbuffer instance : register(b2)
{
	uint materialIdx[4096];
};
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( VStoPS_block )
	nointerpolation uint drawId	: TEXCOORD@counter(texcoord);
	@property( hlms_colour )float4 colour	: TEXCOORD@counter(texcoord);@end
	@foreach( out_uv_count, n )
		float@value( hlms_uv_count@n ) uv@n	: TEXCOORD@counter(texcoord);@end
@end
