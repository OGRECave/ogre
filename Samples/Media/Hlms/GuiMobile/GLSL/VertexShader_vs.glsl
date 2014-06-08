@property( GL3+ )#version 330@end
@property( !GL3+ )#define in attribute
#define out varying@end

in vec4 vertex;
@property( hlms_colour )in vec4 colour;
out vec4 psColour;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end
@foreach( hlms_uv_count, n )
out vec@value( hlms_uv_count@n ) psUv@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per entity
uniform mat4 worldViewProj;
@property( hlms_texture_matrix_count )uniform mat4 texture_matrix[@value( hlms_texture_matrix_count )];@end
// END UNIFORM DECLARATION

void main()
{
	gl_Position = worldViewProj * vertex;
@property( hlms_colour )	psColour = colour;@end

@foreach( hlms_uv_count, n )
	psUv@n = uv@n @property( hlms_texture_matrix_count@n ) * texture_matrix[@counter(CurrentTexMatrix)]@end ;@end
}
