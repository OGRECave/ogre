#version 430

in vec2 texCoord;
out vec4 fragColour;

layout(binding = 0, rgba8) coherent uniform image2D image_data;

// Pixel shader
void main()
{
    ivec2 frag_coord = ivec2(texCoord*256);
    vec4 in_colour = imageLoad(image_data, frag_coord);

    //imageStore(image_data, ivec2(0), vec4(0,0,1,1));

    // Test image load/store and atomic counters.
    //fragColour = vec4(texCoord,0,1);
    fragColour = in_colour;
}
