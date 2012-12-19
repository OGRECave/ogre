#version 150

uniform sampler2D rt0;
uniform sampler2D rt1;
uniform sampler2D rt2;
uniform sampler2D rt3;

in vec2 uv;
out vec4 fragColour;

void main(void)
{
    
    //fragColour = texture(rt0, uv);
    fragColour = texture(rt1, uv);
    //fragColour = texture(rt2, uv);
    //fragColour = texture(rt3, uv);
	
}

