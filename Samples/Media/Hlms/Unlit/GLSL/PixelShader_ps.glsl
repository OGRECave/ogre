@insertpiece( SetCrossPlatformSettings )

layout(std140) uniform;
#define FRAG_COLOR		0

@property( !hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
@end @property( hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out float outColour;
@end

// START UNIFORM DECLARATION
@property( !hlms_shadowcaster )
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
@end
in block
{
@insertpiece( VStoPS_block )
} inPs;
// END UNIFORM DECLARATION

@property( !hlms_shadowcaster )
@property( num_array_textures )uniform sampler2DArray	textureMapsArray[@value( num_array_textures )];@end
@property( num_textures )uniform sampler2D	textureMaps[@value( num_textures )];@end

@property( diffuse )@piece( MultiplyDiffuseConst )* material.diffuse@end @end

@property( diffuse_map || alpha_test || diffuse )Material material;@end

void main()
{
@property( diffuse_map || alpha_test || diffuse )
	uint materialId	= instance.materialIdx[inPs.drawId].x;
	material = materialArray.m[materialId];
@end

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

@property( alpha_test )
	if( material.alpha_test_threshold.x @insertpiece( alpha_test_cmp_func ) outColour.a )
		discard;@end
}

@end @property( hlms_shadowcaster )
void main()
{
	outColour = inPs.depth;
}
@end
