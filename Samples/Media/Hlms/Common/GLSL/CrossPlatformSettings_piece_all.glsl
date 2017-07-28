@piece( SetCrossPlatformSettings )
@property( GL3+ >= 430 )#version 430 core
@end @property( GL3+ < 430 )
#version 330 core
//#extension GL_ARB_shading_language_420pack: require
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

#define float3x3 mat3
#define float4x4 mat4

#define mul( x, y ) ((x) * (y))
#define saturate(x) clamp( (x), 0.0, 1.0 )
#define lerp mix
#define INLINE

#define outVs_Position gl_Position
#define OGRE_SampleLevel( tex, sampler, uv, lod ) textureLod( tex, uv.xy, lod )

@property( GL3+ >= 430 )
#define TEXEL_FETCH texelFetch
@end
@property( GL3+ < 430 )
vec4 TEXEL_FETCH(in sampler2D sampl, in int pixelIdx)
{
    ivec2 pos = ivec2(mod(pixelIdx, 2048), int(uint(pixelIdx) >> 11u));
    return texelFetch( sampl, pos, 0 );
}
ivec4 TEXEL_FETCH(in isampler2D sampl, in int pixelIdx)
{
    ivec2 pos = ivec2(mod(pixelIdx, 2048), int(uint(pixelIdx) >> 11u));
    return texelFetch( sampl, pos, 0 );
}
uvec4 TEXEL_FETCH(in usampler2D sampl, in int pixelIdx)
{
    ivec2 pos = ivec2(mod(pixelIdx, 2048), int(uint(pixelIdx) >> 11u));
    return texelFetch( sampl, pos, 0 );
}
@end

@end
