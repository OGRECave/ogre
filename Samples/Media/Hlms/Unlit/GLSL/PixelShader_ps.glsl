@insertpiece( SetCrossPlatformSettings )

layout(std140) uniform;
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

// START UNIFORM DECLARATION
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
in block
{
@insertpiece( VStoPS_block )
} inPs;
// END UNIFORM DECLARATION

@property( diffuse_map )uniform sampler2DArray	textureMaps[@value( num_textures )];@end

@property( diffuse )@piece( MultiplyDiffuseConst )* material.diffuse@end @end

@property( diffuse_map || alpha_test )Material material;@end

@piece( diffuseIdx0 )material.indices0_3.x & 0x0000FFFF@end
@piece( diffuseIdx1 )material.indices0_3.y & >> 16@end
@piece( diffuseIdx2 )material.indices0_3.z & 0x0000FFFF@end
@piece( diffuseIdx3 )material.indices0_3.w & >> 16@end
@piece( diffuseIdx4 )material.indices4_7.x & 0x0000FFFF@end
@piece( diffuseIdx5 )material.indices4_7.y & >> 16@end
@piece( diffuseIdx6 )material.indices4_7.z & 0x0000FFFF@end
@piece( diffuseIdx7 )material.indices4_7.w & >> 16@end

void main()
{
@property( diffuse_map || alpha_test )
	uint materialId	= instance.materialIdx[inPs.drawId];
	material = materialArray.m[materialId];
@end

@property( !diffuse_map && !diffuse_map0 )
@property( hlms_colour && !diffuse_map )	outColour = inPs.colour @insertpiece( MultiplyDiffuseConst );@end
@property( !hlms_colour && !diffuse )	outColour = vec4(1, 1, 1, 1);@end
@property( !hlms_colour && diffuse )	outColour = material.diffuse;@end
@end

@property( diffuse_map )@property( diffuse_map0 )
	//Load base image
	outColour = texture( textureMaps[@value(diffuse_map0_idx)], vec3( inPs.uv@value( uv_diffuse0 ).@insertpiece( uv_diffuse_swizzle0 ), @insertpiece( diffuseIdx0 ) ) );@end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	vec4 topImage@n = texture( textureMaps[@value(diffuse_map@n_idx)], vec3( inPs.uv@value( uv_diffuse@n ).@insertpiece( uv_diffuse_swizzle@n ), @insertpiece( diffuseIdx@n ) ) );@end @end

@foreach( diffuse_map, n, 1 )@property( diffuse_map@n )
	@insertpiece( blend_mode_idx@n )@end @end

	@property( hlms_colour )outColour *= inPs.colour @insertpiece( MultiplyDiffuseConst );@end
	@property( !hlms_colour && diffuse )outColour *= material.diffuse;@end
@end

@property( alpha_test )
	if( outColour.a @insertpiece( alpha_test_cmp_func ) material.alpha_test_threshold.x )
		discard;@end
}
