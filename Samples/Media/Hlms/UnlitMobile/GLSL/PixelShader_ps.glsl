@insertpiece( SetCrossPlatformSettings )

@property( hlms_colour )in @insertpiece( lowp ) vec4 psColour;@end

@foreach( hlms_uv_count, n )
in @insertpiece( mediump ) vec@value( hlms_uv_count@n ) psUv@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per pass
@property( alpha_test )uniform @insertpiece( lowp ) float alpha_test_threshold;@end
//Uniforms that change per entity
@property( uv_atlas )uniform @insertpiece( mediump ) vec3 atlasOffsets[@value( uv_atlas )];@end
// END UNIFORM DECLARATION

@property( diffuse_map )uniform @insertpiece( lowp ) sampler2D	texDiffuseMap[@value( diffuse_map )];@end

void main()
{
@property( !diffuse_map )
@property( hlms_colour )	outColour = psColour;@end
@property( !hlms_colour )	outColour = @insertpiece( lowp ) vec4(1, 1, 1, 1);@end
@end

@property( diffuse_map )
	//Load base image
	outColour = texture( texDiffuseMap[0], psUv@value( diffuse_map_count0 ) @insertpiece( atlasOffset0 ));@end

	//Group all texture loads together to help the GPU hide the
	//latency (bad GL ES2 drivers won't optimize this automatically)
@foreach( diffuse_map, n, 1 )
	@insertpiece( lowp ) vec4 topImage@n = texture( texDiffuseMap[@n], psUv@value( diffuse_map_count@n ) @insertpiece( atlasOffset@n ));@end

@foreach( diffuse_map, n, 1 )
	@insertpiece( blend_mode_idx@n )@end

@property( diffuse_map )@property( hlms_colour )	outColour *= psColour;@end @end

@property( alpha_test )
	if( outColour.a @insertpiece( alpha_test_cmp_func ) alpha_test_threshold )
		discard;@end
}
