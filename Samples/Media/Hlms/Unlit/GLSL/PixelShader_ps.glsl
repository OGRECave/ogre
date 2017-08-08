@insertpiece( SetCrossPlatformSettings )
@insertpiece( SetCompatibilityLayer )

layout(std140) uniform;
#define FRAG_COLOR		0

@property( hlms_vpos )
in vec4 gl_FragCoord;
@end

@property( !hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
@end @property( hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out float outColour;
@end

// START UNIFORM DECLARATION
@property( has_planar_reflections )
	@insertpiece( PassDecl )
@end
@property( !hlms_shadowcaster )
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
@end
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
@property( !hlms_shadowcaster || !hlms_shadow_uses_depth_texture || exponential_shadow_maps )
in block
{
@insertpiece( VStoPS_block )
} inPs;
@end

@property( !hlms_shadowcaster )
@property( num_array_textures )uniform sampler2DArray	textureMapsArray[@value( num_array_textures )];@end
@property( num_textures )uniform sampler2D	textureMaps[@value( num_textures )];@end

@property( diffuse )@piece( MultiplyDiffuseConst )* material.diffuse@end @end

@property( diffuse_map || alpha_test || diffuse )Material material;@end

void main()
{
	@insertpiece( custom_ps_preExecution )
@property( diffuse_map || alpha_test || diffuse )
	uint materialId	= instance.worldMaterialIdx[inPs.drawId].x;
	material = materialArray.m[materialId];
@end
	@insertpiece( custom_ps_posMaterialLoad )

@property( !diffuse_map && !diffuse_map0 )
@property( hlms_colour && !diffuse_map )	outColour = inPs.colour @insertpiece( MultiplyDiffuseConst );@end
@property( !hlms_colour && !diffuse )	outColour = vec4(1, 1, 1, 1);@end
@property( !hlms_colour && diffuse )	outColour = material.diffuse;@end
@end

@property( diffuse_map )@property( diffuse_map0 )
	//Load base image
	outColour = texture( @insertpiece( SamplerOrigin0 ), @insertpiece( SamplerUV0 ) ).@insertpiece(diffuse_map0_tex_swizzle);@end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	vec4 topImage@n = texture( @insertpiece( SamplerOrigin@n ), @insertpiece( SamplerUV@n ) ).@insertpiece(diffuse_map@n_tex_swizzle);@end @end

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
}

@end @property( hlms_shadowcaster )
	@property( hlms_render_depth_only && !macOS)
		@set( hlms_disable_stage, 1 )
	@end

@insertpiece( DeclShadowCasterMacros )

@property( hlms_shadowcaster_point )
	@insertpiece( PassDecl )
@end

void main()
{
	@insertpiece( custom_ps_preExecution )
	@insertpiece( DoShadowCastPS )
	@insertpiece( custom_ps_posExecution )
}
@end
