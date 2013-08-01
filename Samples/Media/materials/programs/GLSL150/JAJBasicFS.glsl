#version 420

// uniform MyBlock {
//     vec4 mycolour;
// };

out vec4 fragColour;

uniform vec4 ColourMe[2] = vec4[](vec4(0,0,0,0),vec4(0,0,1,0));

uniform bool pretty_colours;
uniform bool image_load_store;

uniform uint colour_factor[3];

layout(binding = 0, offset = 4) uniform atomic_uint atom_counter;
// layout(binding = 0) uniform atomic_uint atom_counter2;

// layout(binding = 0, r32ui) coherent uniform uimage2D image_data;
layout(binding = 0, rgba8) coherent uniform image2D image_data;

// Pixel shader
void main()
{
    // for (int i = 0; i < 3; i++) {
    //     uint counter = atomicCounterIncrement(atom_counter[i]);
    // }

    uint counter = atomicCounterIncrement(atom_counter);

    // uint counter2 = atomicCounterIncrement(atom_counter2);
    // uint counter4eva = atomicCounterIncrement(atom_counter[1]);

    if (pretty_colours) 
    {
        // Test basic setting integer vectors.
        fragColour = ColourMe[0] + ColourMe[1]; // + counter / 4E9;
    }
    else if (image_load_store)
    {
        int in_colour = imageLoad(image_data, ivec2(0));

        //storeImage(image_data, );

        // Test image load/store and atomic counters.
        //fragColour = vec4(in_colour,0,0,1);
        fragColour = vec4(in_colour,0,0,1);
    }
    else 
    {
        // Test uint, bool, and atomic counters.
        fragColour =  counter / 4E9 + vec4(float(colour_factor[0]) / 100, float(colour_factor[1]) / 100, float(colour_factor[2]) / 100, 1);
    }

    // fragColour = vec4(1,1,1,1);
    // fragColour = mycolour;
}
