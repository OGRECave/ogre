@insertpiece( SetCrossPlatformSettings )

// START UNIFORM DECLARATION
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
struct PS_INPUT
{
@insertpiece( VStoPS_block )
};
// END UNIFORM DECLARATION

@foreach( num_array_textures, n )
Texture2DArray textureMapsArray@n : register(t@value(array_texture_bind@n));@end
@foreach( num_textures, n )
Texture2D textureMaps@n : register(t@value(texture_bind@n));@end

@padd( numSamplerStates, num_array_textures, num_textures )
@pset( samplerStateBind, 2 )

@foreach( numSamplerStates, n )
SamplerState samplerState@n : register(s@counter(samplerStateBind));@end

@property( diffuse )@piece( MultiplyDiffuseConst )* material.diffuse@end @end

float4 main( PS_INPUT inPs ) : SV_Target0
{
	@property( diffuse_map || alpha_test || diffuse )Material material;@end

	float4 outColour;
@property( diffuse_map || alpha_test || diffuse )
	uint materialId	= materialIdx[inPs.drawId];
	material = materialArray[materialId];
@end

@property( !diffuse_map && !diffuse_map0 )
@property( hlms_colour && !diffuse_map )	outColour = inPs.colour @insertpiece( MultiplyDiffuseConst );@end
@property( !hlms_colour && !diffuse )	outColour = float4(1, 1, 1, 1);@end
@property( !hlms_colour && diffuse )	outColour = material.diffuse;@end
@end

@property( diffuse_map )@property( diffuse_map0 )
	//Load base image
		outColour = @insertpiece( TextureOrigin0 ).Sample( @insertpiece( SamplerOrigin0 ), @insertpiece( SamplerUV0 ) );@end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
		float4 topImage@n = @insertpiece( TextureOrigin@n ).Sample( @insertpiece( SamplerOrigin@n ), @insertpiece( SamplerUV@n ) );@end @end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	@insertpiece( blend_mode_idx@n )@end @end

	@property( hlms_colour )outColour *= inPs.colour @insertpiece( MultiplyDiffuseConst );@end
	@property( !hlms_colour && diffuse )outColour *= material.diffuse;@end
@end

@property( alpha_test )
	if( material.alpha_test_threshold.x @insertpiece( alpha_test_cmp_func ) outColour.a )
		discard;@end

	return outColour;
}
