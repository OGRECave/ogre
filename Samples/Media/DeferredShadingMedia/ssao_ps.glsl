OGRE_NATIVE_GLSL_VERSION_DIRECTIVE

uniform mat4 ptMat;
uniform float far;
uniform sampler2D geomMap;
uniform sampler2D randMap;

out vec4 fragColour;
in vec2 oUv0;
in vec3 ray;

void main()
{
    #define MAX_RAND_SAMPLES 14

    const vec3 RAND_SAMPLES[MAX_RAND_SAMPLES] =
    vec3[](
        vec3(1, 0, 0),
        vec3(-1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, -1, 0),
        vec3(0, 0, 1),
        vec3(0, 0, -1),
        normalize(vec3(1, 1, 1)),
        normalize(vec3(-1, 1, 1)),
        normalize(vec3(1, -1, 1)),
        normalize(vec3(1, 1, -1)),
        normalize(vec3(-1, -1, 1)),
        normalize(vec3(-1, 1, -1)),
        normalize(vec3(1, -1, -1)),
        normalize(vec3(-1, -1, -1)));

    // constant expression != const int :(
    #define NUM_BASE_SAMPLES 6

    // random normal lookup from a texture and expand to [-1..1]
    vec3 randN = textureLod(randMap, oUv0 * 24.0, 0.0).xyz * 2.0 - 1.0;
    vec4 geom = textureLod(geomMap, oUv0, 0.0);
    float depth = geom.w;

    // IN.ray will be distorted slightly due to interpolation
    // it should be normalized here
    vec3 viewPos = ray * depth;

    // By computing Z manually, we lose some accuracy under extreme angles
    // considering this is just for bias, this loss is acceptable
    vec3 viewNorm = geom.xyz;

    // Accumulated occlusion factor
    float occ = 0.0;
    for (int i = 0; i < NUM_BASE_SAMPLES; ++i)
    {
        // Reflected direction to move in for the sphere
        // (based on random samples and a random texture sample)
        // bias the random direction away from the normal
        // this tends to minimize self occlusion
        vec3 randomDir = reflect(RAND_SAMPLES[i], randN) + viewNorm;

        // Move new view-space position back into texture space
        #define RADIUS 0.2125
        vec4 nuv = ptMat * vec4(viewPos.xyz + randomDir * RADIUS, 1.0);
        nuv.xy /= nuv.w;

        // Compute occlusion based on the (scaled) Z difference
        float zd = clamp(far * (depth - textureLod(geomMap, nuv.xy, nuv.w).w), 0.0, 1.0);
        // This is a sample occlusion function, you can always play with
        // other ones, like 1.0 / (1.0 + zd * zd) and stuff
        occ += clamp(pow(1.0 - zd, 11.0) + zd, 0.0, 1.0);
    }
    occ /= float(NUM_BASE_SAMPLES);

    fragColour = vec4(occ, occ, occ, 1.0);
}
