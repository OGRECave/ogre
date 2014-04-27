#version 330
/*#ifdef GL_ES
precision mediump float;
#endif*/
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

@property( hlms_colour )in vec4 psColour;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) psUv@n;@end

@property( diffuse_map )uniform sampler2D	texDiffuseMap[@value( diffuse_map_count )];@end

void main()
{
@property( !diffuse_map )
@property( hlms_colour )	outColour = psColour;@end
@property( !hlms_colour )	outColour = vec4(0);@end
@end

@property( diffuse_map )
	//Load base image
	outColour = texture( texDiffuseMap[0], psUv@value( diffuse_map_count0 ) );@end

	//Group all texture loads together to help the GPU to hide
	//the latency (bad GL ES2 drivers won't optimize this)
@foreach( diffuse_map_count, n, 1 )
	vec4 topImage@n = texture( texDiffuseMap[@n], psUv@value( diffuse_map_count@n ) );

@foreach( diffuse_map_count, n, 1 )
	@insertpiece( diffuse_map_blend_mode@n )@end

@property( diffuse_map )@property( hlms_colour )	outColour *= psColour;@end
}
