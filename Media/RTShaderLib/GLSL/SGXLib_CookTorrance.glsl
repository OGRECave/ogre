#define PI 3.14159265359

#ifdef OGRE_GLSLES
    // min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
    #define MIN_PERCEPTUAL_ROUGHNESS 0.089
#else
    #define MIN_PERCEPTUAL_ROUGHNESS 0.045
#endif

#define MEDIUMP_FLT_MAX    65504.0
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)

// Computes x^5 using only multiply operations.
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/diffusebrdf
float Fd_Lambert() {
    return 1.0 / PI;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/fresnel(specularf)
vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

vec3 computeDiffuseColor(const vec3 baseColor, float metallic) {
    return baseColor.rgb * (1.0 - metallic);
}

vec3 computeF0(const vec3 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambdaV + lambdaL);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    // clamp to the maximum value representable in mediump
    return saturateMediump(v);
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/normaldistributionfunction(speculard)
float D_GGX(float roughness, float NoH, const vec3 h, const vec3 n) {
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"

    // In mediump, there are two problems computing 1.0 - NoH^2
    // 1) 1.0 - NoH^2 suffers floating point cancellation when NoH^2 is close to 1 (highlights)
    // 2) NoH doesn't have enough precision around 1.0
    // Both problem can be fixed by computing 1-NoH^2 in highp and providing NoH in highp as well

    // However, we can do better using Lagrange's identity:
    //      ||a x b||^2 = ||a||^2 ||b||^2 - (a . b)^2
    // since N and H are unit vectors: ||N x H||^2 = 1.0 - NoH^2
    // This computes 1.0 - NoH^2 directly (which is close to zero in the highlights and has
    // enough precision).
    // Overall this yields better performance, keeping all computations in mediump
#ifdef OGRE_GLSLES
    vec3 NxH = cross(n, h);
    float oneMinusNoHSquared = dot(NxH, NxH);
#else
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
#endif

    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);
    return saturateMediump(d);
}

float getDistanceAttenuation(const vec3 params, float distance)
{
    return 1.0 / (params.x + params.y * distance + params.z * distance * distance);
}

float getAngleAttenuation(const vec3 params, const vec3 lightDir, const vec3 toLight)
{
    float rho		= dot(-lightDir, toLight);
    float fSpotE	= saturate((rho - params.y) / (params.x - params.y));
    return pow(fSpotE, params.z);
}

void PBR_Light(
                in vec3 vNormal,
                in vec3 viewPos,
                in vec4 lightPos,
                in vec3 lightColor,
                in vec4 pointParams,
                in vec4 vLightDirView,
                in vec4 spotParams,
                in vec3 baseColor,
                in vec2 mrParam,
                inout vec3 vOutColour)
{
    vec3 vLightView = lightPos.xyz;
    float fLightD = 0.0;

    if (lightPos.w != 0.0)
    {
        vLightView -= viewPos; // to light
        fLightD     = length(vLightView);

        if(fLightD > pointParams.x)
            return;
    }

	vLightView		   = normalize(vLightView);

	vec3 vNormalView = normalize(vNormal);
	float NoL		 = saturate(dot(vNormalView, vLightView));

    if(NoL <= 0.0)
        return; // not lit by this light

    float perceptualRoughness = mrParam.x;
    float metallic = saturate(mrParam.y);

    // TODO: some of this can be precomputed for multiple lights

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    float roughness = perceptualRoughnessToRoughness(perceptualRoughness);

    // gamma to linear
    baseColor = pow(baseColor, vec3_splat(2.2));

    vec3 f0 = computeF0(baseColor, metallic, 0.04);
    vec3 diffuseColor = computeDiffuseColor(baseColor, metallic);

    // https://google.github.io/filament/Filament.md.html#toc5.6.2
    float f90 = saturate(dot(f0, vec3_splat(50.0 * 0.33)));

	vec3 vView       = -normalize(viewPos);

    // https://google.github.io/filament/Filament.md.html#materialsystem/standardmodelsummary
    vec3 h    = normalize(vView + vLightView);
    float NoH = saturate(dot(vNormalView, h));
    float NoV = abs(dot(vNormalView, vView)) + 1e-5;

    float V = V_SmithGGXCorrelated(roughness, NoV, NoL);
    vec3 F  = F_Schlick(f0, f90, NoH);
    float D = D_GGX(roughness, NoH, h, vNormalView);

    vec3 Fr = (D * V) * F;
    vec3 Fd = diffuseColor * Fd_Lambert();

    // https://google.github.io/filament/Filament.md.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance
    vec3 energyCompensation = vec3_splat(1.0) + f0; // TODO: dfg.y

    vec3 color = NoL * lightColor * (Fr * energyCompensation + Fd);

    color *= getDistanceAttenuation(pointParams.yzw, fLightD);

    if(spotParams.w != 0.0)
    {
        color *= getAngleAttenuation(spotParams.xyz, vLightDirView.xyz, vLightView);
    }

    // linear to gamma
    color = pow(color, vec3_splat(1.0/2.2));
    vOutColour = saturate(vOutColour + color);
}