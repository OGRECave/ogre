#version 430

out vec4 fragColour;

layout (r32ui) uniform restrict readonly uimage2D testTexture;

in vec4 gl_FragCoord;

void main()
{
    fragColour = unpackUnorm4x8( imageLoad( testTexture, ivec2(gl_FragCoord.xy) ).x );
}
