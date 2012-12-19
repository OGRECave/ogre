#version 150

//-------------------------------
//Bloom_ps20.glsl
// Blends using weights the blurred image with the sharp one
// Params:
//   OriginalImageWeight
//   BlurWeight
//-------------------------------

uniform sampler2D RT;
uniform sampler2D Blur1;

uniform float OriginalImageWeight;
uniform float BlurWeight;

out vec4 fragColour;
in vec2 texCoord[5];

void main()
{
    vec4 sharp;
    vec4 blur;
    

    sharp = texture(RT, texCoord[0]);
    blur = texture(Blur1, texCoord[0]);
    
    fragColour = ( (blur * BlurWeight) + (sharp * OriginalImageWeight) );
    //fragColour = vec4(0);
}
