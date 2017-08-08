@property( GL_ARB_texture_buffer_range )
@piece( Common_Matrix_DeclUnpackMatrix4x4 )
mat4 UNPACK_MAT4( samplerBuffer matrixBuf, uint pixelIdx )
{
	vec4 row0 = texelFetch( matrixBuf, int((pixelIdx) << 2u) );
	vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
	vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
	vec4 row3 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 3u) );
    return mat4( row0, row1, row2, row3 );
}
@end

@piece( Common_Matrix_DeclUnpackMatrix3x4 )
mat3x4 UNPACK_MAT3x4( samplerBuffer matrixBuf, uint pixelIdx )
{
	vec4 row0 = texelFetch( matrixBuf, int((pixelIdx) << 2u) );
	vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
	vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
	return mat3x4( row0, row1, row2 );
}
@end
@end

@property( !GL_ARB_texture_buffer_range )
@piece( Common_Matrix_DeclUnpackMatrix4x4 )
mat4 UNPACK_MAT4( in sampler2D matrixBuf, in uint pixelIdx )
{
    ivec2 pos0 = ivec2(int(((pixelIdx) << 2u) & 2047u), int(((pixelIdx) << 2u) >> 11u));
    ivec2 pos1 = ivec2(int((((pixelIdx) << 2u) + 1u) & 2047u), int((((pixelIdx) << 2u) + 1u) >> 11u));
    ivec2 pos2 = ivec2(int((((pixelIdx) << 2u) + 2u) & 2047u), int((((pixelIdx) << 2u) + 2u) >> 11u));
    ivec2 pos3 = ivec2(int((((pixelIdx) << 2u) + 3u) & 2047u), int((((pixelIdx) << 2u) + 3u) >> 11u));
    vec4 row0 = texelFetch( matrixBuf, pos0, 0 );
    vec4 row1 = texelFetch( matrixBuf, pos1, 0 );
    vec4 row2 = texelFetch( matrixBuf, pos2, 0 );
    vec4 row3 = texelFetch( matrixBuf, pos3, 0 );
    return mat4( row0, row1, row2, row3 );
}
@end

@piece( Common_Matrix_DeclUnpackMatrix3x4 )
mat3x4 UNPACK_MAT3x4( in sampler2D matrixBuf, in uint pixelIdx )
{
    ivec2 pos0 = ivec2(int(((pixelIdx) << 2u) & 2047u), int(((pixelIdx) << 2u) >> 11u));
    ivec2 pos1 = ivec2(int((((pixelIdx) << 2u) + 1u) & 2047u), int((((pixelIdx) << 2u) + 1u) >> 11u));
    ivec2 pos2 = ivec2(int((((pixelIdx) << 2u) + 2u) & 2047u), int((((pixelIdx) << 2u) + 2u) >> 11u));
    vec4 row0 = texelFetch( matrixBuf, pos0, 0 );
    vec4 row1 = texelFetch( matrixBuf, pos1, 0 );
    vec4 row2 = texelFetch( matrixBuf, pos2, 0 );
    return mat3x4( row0, row1, row2 );
}
@end
@end

