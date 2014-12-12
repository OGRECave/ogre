@insertpiece( SetCrossPlatformSettings )

mat4 UNPACK_MAT4( samplerBuffer matrixBuf, uint pixelIdx )
{
        vec4 row0 = texelFetch( matrixBuf, int((pixelIdx) << 2u) );
        vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
        vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
        vec4 row3 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 3u) );
	return mat4( row0.x, row1.x, row2.x, row3.x,
				 row0.y, row1.y, row2.y, row3.y,
				 row0.z, row1.z, row2.z, row3.z,
				 row0.w, row1.w, row2.w, row3.w );
}

layout(std140) uniform;

in vec4 vertex;
@property( hlms_colour )in vec4 colour;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end

in uint drawId;

out block
{
@insertpiece( VStoPS_block )
} outVs;

// START UNIFORM DECLARATION
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
layout(binding = 0) uniform samplerBuffer worldMatBuf;
@property( texture_matrix )layout(binding = 1) uniform samplerBuffer animationMatrixBuf;@end
// END UNIFORM DECLARATION

void main()
{
	//uint drawId = 1;
	mat4 worldViewProj;
	worldViewProj = UNPACK_MAT4( worldMatBuf, drawId );

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

@property( hlms_colour )	outVs.colour = colour;@end

@property( texture_matrix )	mat4 textureMatrix;@end

@foreach( out_uv_count, n )
	@property( out_uv@_texture_matrix )textureMatrix = UNPACK_MAT4( animationMatrixBuf, (instance.materialIdx[drawId] << 4) + @value( out_uv@n_tex_unit ) );@end
	outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = uv@value( out_uv@n_source_uv ).xy @property( out_uv@_texture_matrix ) * textureMatrix@end ;@end

	outVs.drawId = drawId;
}
