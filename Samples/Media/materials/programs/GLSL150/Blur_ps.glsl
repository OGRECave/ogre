#version 150

uniform sampler2D tex0;

in vec2 texCoord[5];

out vec4 fragColour;

void main()
{
	vec4 sum = texture(tex0, texCoord[0]) + 
			   texture(tex0, texCoord[1]) +
			   texture(tex0, texCoord[2]) + 
			   texture(tex0, texCoord[3]) +
			   texture(tex0, texCoord[4]);
	fragColour = sum / 5.0;
}

