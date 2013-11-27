#version 150

in block {
    vec4 pos;
    vec4 colour;
    vec2 texcoord;
} Firework;

out vec4 fragColour;

uniform sampler2D diffuseTex;

// Colours the fireworks.
void main()
{
    fragColour = texture(diffuseTex, Firework.texcoord) * Firework.colour;
}
