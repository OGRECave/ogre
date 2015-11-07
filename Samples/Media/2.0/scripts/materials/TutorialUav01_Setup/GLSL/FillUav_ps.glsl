#version 430

layout (rgba8) uniform restrict writeonly image2D testTexture;

in block
{
    vec2 uv0;
} inPs;

in vec4 gl_FragCoord;

void main()
{
    imageStore( testTexture, ivec2(gl_FragCoord.xy), vec4( inPs.uv0.xy, 0.0f, 1.0f ) );
}
