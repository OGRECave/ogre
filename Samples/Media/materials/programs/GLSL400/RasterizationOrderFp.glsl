#version 400
#extension GL_ARB_shader_storage_buffer_object : require

buffer CounterBuffer
{
    uint ac;
};

out vec4 fragColour;

void main(void)
{
    uint counter = atomicAdd(ac, 1);
    fragColour = vec4(float(counter)/(480*480),0,0,1);
}
