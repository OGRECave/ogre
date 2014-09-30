@insertpiece( SetCrossPlatformSettings )

in vec4 vertex;
@property( hlms_colour )in vec4 colour;
out vec4 psColour;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end
@foreach( hlms_uv_count, n )
out vec@value( hlms_uv_count@n ) psUv@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per pass
@property( hlms_texture_matrix_count )uniform mat4 texture_matrix[@value( hlms_texture_matrix_count )];@end
//Uniforms that change per entity
uniform mat4 worldViewProj;
// END UNIFORM DECLARATION

void main()
{
@property( !hlms_dual_paraboloid_mapping )
	gl_Position = worldViewProj * vertex;
@end

@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	gl_Position.w	= 1.0f;
	gl_Position.xyz	= (worldViewProj * vertex).xyz;
	float L = length( gl_Position.xyz );
	gl_Position.z	+= 1.0f;
	gl_Position.xy	/= gl_Position.z;
	gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);
@end

@property( hlms_colour )	psColour = colour;@end

@foreach( hlms_uv_count, n )
	psUv@n = uv@n @property( hlms_texture_matrix_count@n ) * texture_matrix[@counter(CurrentTexMatrix)]@end ;@end
}
