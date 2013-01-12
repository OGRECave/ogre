#version 400 core

out vec4 fragColour;
layout(binding=0, offset=0) uniform atomic_uint ac;

void main(void)
{
    uint Counter = atomicCounterIncrement(ac);
	uint Mask = (1 << 8) - 1;
	fragColour = vec4(
                    ((Counter & (Mask <<  0)) % 255) / 255.f,
                    ((Counter & (Mask <<  8)) % 255) / 255.f,
                    ((Counter & (Mask << 16)) % 255) / 5.f,
                    0.5);
}
