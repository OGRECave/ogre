// Simple PCF
// Number of samples in one dimension (square for total samples)
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1.0
#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

float calcDepthShadow(sampler2D shadowMap, vec4 uv, float invShadowMapSize)
{
    // 4-sample PCF
    float shadow = 0.0;
    float offset = (NUM_SHADOW_SAMPLES_1D/2.0 - 0.5) * SHADOW_FILTER_SCALE;
    uv /= uv.w;
    uv.z = uv.z * 0.5 + 0.5; // convert -1..1 to 0..1
    for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
        for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
        {
            vec2 newUV = uv.xy + vec2(x, y) * invShadowMapSize;
            float depth = texture2D(shadowMap, newUV).x;
            if (depth >= 1.0 || depth >= uv.z) // depth = 1.0 at PSSM end
                shadow += 1.0;
        }
    shadow /= SHADOW_SAMPLES;
    return shadow;
}

float calcSimpleShadow(sampler2D shadowMap, vec4 shadowMapPos)
{
    return texture2DProj(shadowMap, shadowMapPos).x;
}
vec4 expand(vec4 v)
{
    return v * 2.0 - 1.0;
}
// From http://substance.io/zauner/porting-vvvv-hlsl-shaders-to-vvvvjs
vec4 lit(float NdotL, float NdotH, float m) {
    float ambient = 1.0;
    float diffuse = max(0.0, NdotL);
    float specular = step(0.0, NdotL) * max(NdotH, 0.0);
    return vec4(ambient, diffuse, specular, 1.0);
}
