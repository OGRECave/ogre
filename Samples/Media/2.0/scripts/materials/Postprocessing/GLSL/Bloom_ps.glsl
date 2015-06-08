#version 330

//-------------------------------
// Blends using weights the blurred image with the sharp one
//-------------------------------

uniform sampler2D RT;
uniform sampler2D Blur1;

uniform float OriginalImageWeight;
uniform float BlurWeight;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	vec4 sharp	= texture( RT, inPs.uv0 );
	vec4 blur	= texture( Blur1, inPs.uv0 );
    
	fragColour = ( (blur * BlurWeight) + (sharp * OriginalImageWeight) );
}
