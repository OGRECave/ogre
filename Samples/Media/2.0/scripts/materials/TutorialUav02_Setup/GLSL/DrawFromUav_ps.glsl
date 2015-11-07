#version 430

out vec4 fragColour;

layout (rgba8) uniform restrict readonly image2D testTexture;

in vec4 gl_FragCoord;

void main()
{
    fragColour = imageLoad( testTexture, ivec2(gl_FragCoord.xy) );
}
