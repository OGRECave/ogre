#version 150

in block {
	vec4 	pos;
    vec4 	color;
	vec2	texcoord;
} Firework;

out vec4 fragColour;

uniform sampler2D diffuseTex;

//The pixels shaders that colors the fireworks
void main()
{
	fragColour = texture(diffuseTex, Firework.texcoord) * Firework.color;
}
