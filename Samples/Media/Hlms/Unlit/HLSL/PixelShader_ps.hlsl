@insertpiece( SetCrossPlatformSettings )

// START UNIFORM DECLARATION
@property( !hlms_shadowcaster )
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
@end
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
struct PS_INPUT
{
@insertpiece( VStoPS_block )
};

@property( !hlms_shadowcaster )

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
	@insertpiece( custom_ps_preExecution )

	@property( diffuse_map || alpha_test || diffuse )Material material;@end

	float4 outColour;
@property( diffuse_map || alpha_test || diffuse )
	uint materialId	= materialIdx[inPs.drawId].x;
	material = materialArray[materialId];
@end

	@insertpiece( custom_ps_posMaterialLoad )

@property( !diffuse_map && !diffuse_map0 )
@property( hlms_colour && !diffuse_map )	outColour = inPs.colour @insertpiece( MultiplyDiffuseConst );@end
@property( !hlms_colour && !diffuse )	outColour = float4(1, 1, 1, 1);@end
@property( !hlms_colour && diffuse )	outColour = material.diffuse;@end
@end

@property( diffuse_map )@property( diffuse_map0 )
	//Load base image
	outColour = @insertpiece( TextureOrigin0 ).Sample( @insertpiece( SamplerOrigin0 ), @insertpiece( SamplerUV0 ) ).@insertpiece(diffuse_map0_tex_swizzle);@end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	float4 topImage@n = @insertpiece( TextureOrigin@n ).Sample( @insertpiece( SamplerOrigin@n ), @insertpiece( SamplerUV@n ) ).@insertpiece(diffuse_map@n_tex_swizzle);@end @end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	@insertpiece( blend_mode_idx@n )@end @end

	@property( hlms_colour )outColour *= inPs.colour @insertpiece( MultiplyDiffuseConst );@end
	@property( !hlms_colour && diffuse )outColour *= material.diffuse;@end
@end

	@insertpiece( custom_ps_preLights )

@property( alpha_test )
	if( material.alpha_test_threshold.x @insertpiece( alpha_test_cmp_func ) outColour.a )
		discard;@end

	@insertpiece( custom_ps_posExecution )

	return outColour;
}
@end @property( hlms_shadowcaster )
	@property( hlms_shadow_uses_depth_texture )
		@set( hlms_disable_stage, 1 )
	@end
float main( PS_INPUT inPs ) : SV_Target0
{
	@insertpiece( custom_ps_preExecution )
	@insertpiece( custom_ps_posExecution )
	return inPs.depth;
}
@end
