#version 430

layout (r32ui) uniform restrict writeonly uimage2D testTexture;

in block
{
    vec2 uv0;
} inPs;

in vec4 gl_FragCoord;

void main()
{
	uint packedVal = packUnorm4x8( vec4( inPs.uv0.xy, 0.0f, 1.0f ) );
    imageStore( testTexture, ivec2(gl_FragCoord.xy), uvec4( packedVal, 0u, 0u, 0u ) );
}
