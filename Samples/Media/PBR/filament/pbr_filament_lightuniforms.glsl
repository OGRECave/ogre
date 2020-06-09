struct LightUniform
{
    mat4 lights[LIGHT_COUNT];
};

uniform LightUniform lightsUniforms;

uniform usampler2D light_froxels;
uniform usampler2D light_records;