#version 400

layout(binding=0, offset=0) uniform atomic_uint ac;

out vec4 fragColour;

void main(void)
{
    uint counter = atomicCounterIncrement(ac);
    uint mask = (1 << 8) - 1;
    // fragColour = vec4(
    //     ((counter & (mask <<  0)) % 255) / 255.f,
    //     ((counter & (mask <<  8)) % 255) / 255.f,
    //     ((counter & (mask << 16)) % 255) / 5.f,
    //     0.5);
    fragColour = vec4(counter/4E9,0,0,0);
}
