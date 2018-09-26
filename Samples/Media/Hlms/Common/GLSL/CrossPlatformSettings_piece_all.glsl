@piece( SetCrossPlatformSettings )
@property( GL3+ >= 430 )#version 430 core
@end @property( GL3+ < 430 )
#version 330 core
@end

@property( GL_ARB_shading_language_420pack )
    #extension GL_ARB_shading_language_420pack: require
    #define layout_constbuffer(x) layout( std140, x )
@end
@property( GL_ARB_texture_buffer_range )
	#define bufferFetch texelFetch
@end

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4

#define float2x2 mat2
#define float3x3 mat3
#define float4x4 mat4

#define ushort uint

#define toFloat3x3( x ) mat3( x )
#define buildFloat3x3( row0, row1, row2 ) mat3( row0, row1, row2 )

#define mul( x, y ) ((x) * (y))
#define saturate(x) clamp( (x), 0.0, 1.0 )
#define lerp mix
#define rsqrt inversesqrt
#define INLINE

#define finalDrawId drawId
#define PARAMS_ARG_DECL
#define PARAMS_ARG

#define outVs_Position gl_Position
#define OGRE_Sample( tex, sampler, uv ) texture( tex, uv )
#define OGRE_SampleLevel( tex, sampler, uv, lod ) textureLod( tex, uv.xy, lod )
#define OGRE_SampleArray2D( tex, sampler, uv, arrayIdx ) texture( tex, vec3( uv, arrayIdx ) )
#define OGRE_SampleArray2DLevel( tex, sampler, uv, arrayIdx, lod ) textureLod( tex, vec3( uv, arrayIdx ), lod )
#define OGRE_SampleGrad( tex, sampler, uv, ddx, ddy ) textureGrad( tex, uv, ddx, ddy )
#define OGRE_SampleArray2DGrad( tex, sampler, uv, arrayIdx, ddx, ddy ) textureGrad( tex, vec3( uv, arrayIdx ), ddx, ddy )
#define OGRE_ddx( val ) dFdx( val )
#define OGRE_ddy( val ) dFdy( val )

#define bufferFetch1( buffer, idx ) texelFetch( buffer, idx ).x
@end

@property( !GL_ARB_texture_buffer_range || !GL_ARB_shading_language_420pack )
@piece( SetCompatibilityLayer )
    @property( !GL_ARB_texture_buffer_range )
        #define samplerBuffer sampler2D
        #define isamplerBuffer isampler2D
        #define usamplerBuffer usampler2D
        vec4 bufferFetch( in sampler2D sampl, in int pixelIdx )
        {
            ivec2 pos = ivec2( mod( pixelIdx, 2048 ), int( uint(pixelIdx) >> 11u ) );
            return texelFetch( sampl, pos, 0 );
        }
        ivec4 bufferFetch(in isampler2D sampl, in int pixelIdx)
        {
            ivec2 pos = ivec2( mod( pixelIdx, 2048 ), int( uint(pixelIdx) >> 11u ) );
            return texelFetch( sampl, pos, 0 );
        }
        uvec4 bufferFetch( in usampler2D sampl, in int pixelIdx )
        {
            ivec2 pos = ivec2( mod( pixelIdx, 2048 ), int( uint(pixelIdx) >> 11u ) );
            return texelFetch( sampl, pos, 0 );
        }
    @end
    @property( !GL_ARB_shading_language_420pack )
        #define layout_constbuffer(x) layout( std140 )
    @end
@end
@end
