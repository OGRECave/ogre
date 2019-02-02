// Simple PCF
// Number of samples in one dimension (square for total samples)
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1.0
#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)
{
    return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}
float calcDepthShadow(sampler2D shadowMap, vec4 uv, float invShadowMapSize)
{
    // 4-sample PCF
    float shadow = 0.0;
    float offset = (NUM_SHADOW_SAMPLES_1D/2.0 - 0.5) * SHADOW_FILTER_SCALE;
    for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
        for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
        {
            vec4 newUV = offsetSample(uv, vec2(x, y), invShadowMapSize);
            // manually project and assign derivatives
            // to avoid gradient issues inside loops
            float depth = texture2DProj(shadowMap, newUV.xyw).x;
            if (depth >= 1.0 || depth >= uv.z)
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
