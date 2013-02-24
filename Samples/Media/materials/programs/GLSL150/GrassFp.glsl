#version 150

////////////////////////////// MOVING GRASS
// Vertex program to wave some grass about
// Assumes UV texture coords of v==0 indicates the top of the grass
uniform sampler2D diffuseMap;

in vec4 oUv0;
in vec4 oColour;

out vec4 fragColour;

void main()
{	
    vec4 texColor = texture(diffuseMap, oUv0.xy);
    fragColour = vec4(texColor.rgb * oColour.rgb, texColor.a); 

    // Do manual alpha rejection
    if (texColor.a < 0.58)
    {
        discard;
    }
}
