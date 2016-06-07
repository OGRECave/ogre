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
