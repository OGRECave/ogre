#version 430

out vec4 fragColour;

uniform sampler2D testTexture;

in block
{
    vec2 uv0;
} inPs;


void main()
{
    fragColour = texture( testTexture, inPs.uv0.xy );
}
