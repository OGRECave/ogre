@property( GL3+ )#version 330@end
@property( !GL3+ )#define in varying@end
/*#ifdef GL_ES
precision mediump float;
#endif*/
#define FRAG_COLOR		0
@property( GL3+ )layout(location = FRAG_COLOR, index = 0) out vec4 outColour;@end
@property( !GL3+ )#define outColour gl_FragColor@end

@property( hlms_colour )in lowp vec4 psColour;@end

@foreach( hlms_uv_count, n )
in mediump vec@value( hlms_uv_count@n ) psUv@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per entity
@property( alpha_test )uniform lowp float alpha_test_threshold;@end
@property( uv_atlas )uniform mediump vec3 atlasOffsets[@value( uv_atlas )];@end
// END UNIFORM DECLARATION

@property( diffuse_map )uniform lowp sampler2D	texDiffuseMap[@value( diffuse_map )];@end

void main()
{
@property( !diffuse_map )
@property( hlms_colour )	outColour = psColour;@end
@property( !hlms_colour )	outColour = vec4(0, 0, 0, 1);@end
@end

@property( diffuse_map )
	//Load base image
	outColour = texture( texDiffuseMap[0], psUv@value( diffuse_map_count0 ) @insertpiece( atlasOffset0 ));@end

@property( alpha_test )
	if( outColour.a @insertpiece( alpha_test_cmp_func ) alpha_test_threshold )
		discard;@end

	//Group all texture loads together to help the GPU to hide the
	//latency (bad GL ES2 drivers won't optimize this automatically)
@foreach( diffuse_map, n, 1 )
	lowp vec4 topImage@n = texture( texDiffuseMap[@n], psUv@value( diffuse_map_count@n ) @insertpiece( atlasOffset@n ));@end

@foreach( diffuse_map, n, 1 )
	@insertpiece( blend_mode_idx@n )@end

@property( diffuse_map )@property( hlms_colour )	outColour *= psColour;@end @end
}
