#version 150

in vec4 ambientUV;
in vec3 ray;

out vec4 fragColour;

uniform vec4 invTexSize;
uniform sampler2D map;
uniform sampler2D geomMap;

#define NUM_BLUR_SAMPLES 8

void main()
{
    vec2 o = vec2(invTexSize.x, 0);
    vec4 sum = textureLod(map, ambientUV.xy, ambientUV.w) * (NUM_BLUR_SAMPLES + 1);
    float denom = NUM_BLUR_SAMPLES + 1;
    vec4 geom = textureLod(geomMap, ambientUV.xy, ambientUV.w);
    for (int i = 1; i <= NUM_BLUR_SAMPLES; ++i)
    {
        vec2 nuv = ambientUV.xy + o * i;
        vec4 nGeom = textureLod(geomMap, nuv, 0);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * step(0.9, (dot(geom.xyz, nGeom.xyz)));
        sum += textureLod(map, nuv, 0) * coef;
        denom += coef;
    }
    for (int i = 1; i <= 4; ++i)
    {
        vec2 nuv = ambientUV.xy + o * -i;
        vec4 nGeom = textureLod(geomMap, nuv, 0);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * step(0.9, (dot(geom.xyz, nGeom.xyz)));
        sum += textureLod(map, nuv, 0) * coef;
        denom += coef;
    }
    fragColour = sum / denom;
}
