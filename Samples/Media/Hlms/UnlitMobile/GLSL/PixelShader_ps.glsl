@insertpiece( SetCrossPlatformSettings )

@property( hlms_colour )in @insertpiece( lowp ) vec4 psColour;@end

@foreach( hlms_uv_count, n )
in @insertpiece( mediump ) vec@value( hlms_uv_count@n ) psUv@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per material
@property( diffuse )uniform @insertpiece( mediump ) vec4 diffuseColour;@end
@property( alpha_test )uniform @insertpiece( lowp ) float alpha_test_threshold;@end
@property( uv_atlas )uniform @insertpiece( mediump ) vec3 atlasOffsets[@value( uv_atlas )];@end
// END UNIFORM DECLARATION

@property( diffuse_map )uniform @insertpiece( lowp ) sampler2D	texDiffuseMap[@value( diffuse_map )];@end

@property( diffuse )@piece( MultiplyDiffuseConst )* diffuseColour@end @end

void main()
{
@property( !diffuse_map )
@property( hlms_colour )	outColour = psColour @insertpiece( MultiplyDiffuseConst );@end
@property( !hlms_colour && !diffuse )	outColour = @insertpiece( lowp ) vec4(1, 1, 1, 1);@end
@property( !hlms_colour && diffuse )	outColour = diffuseColour;@end
@end

@property( diffuse_map )
	//Load base image
	outColour = texture( texDiffuseMap[0], psUv@value( diffuse_map_count0 ) @insertpiece( atlasOffset0 ));

	//Group all texture loads together to help the GPU hide the
	//latency (bad GL ES2 drivers won't optimize this automatically)
@foreach( diffuse_map, n, 1 )
	@insertpiece( lowp ) vec4 topImage@n = texture( texDiffuseMap[@n], psUv@value( diffuse_map_count@n ) @insertpiece( atlasOffset@n ));@end

@foreach( diffuse_map, n, 1 )
	@insertpiece( blend_mode_idx@n )@end

	@property( hlms_colour )outColour *= psColour @insertpiece( MultiplyDiffuseConst );@end
	@property( !hlms_colour )outColour *= diffuseColour;@end
@end

@property( alpha_test )
	if( outColour.a @insertpiece( alpha_test_cmp_func ) alpha_test_threshold )
		discard;@end
}
