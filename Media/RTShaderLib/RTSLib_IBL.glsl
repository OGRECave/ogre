// This file is part of the OGRE project.
// code adapted from Google Filament
// SPDX-License-Identifier: Apache-2.0

vec3 specularDFG(const PixelParams pixel) {
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
}

vec3 decodeDataForIBL(const vec4 data) {
    return data.rgb;
}

vec3 Irradiance_RoughnessOne(samplerCube light_iblSpecular, const vec3 n, float iblRoughnessOneLevel) {
    // note: lod used is always integer, hopefully the hardware skips tri-linear filtering
    return decodeDataForIBL(textureCubeLod(light_iblSpecular, n, iblRoughnessOneLevel));
}

vec3 PrefilteredDFG_LUT(sampler2D light_iblDFG, float lod, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    // OGRE Specific: y is flipped compared to Filament code
    return texture2DLod(light_iblDFG, vec2(NoV, 1.0 - lod), 0.0).rgb;
}

float perceptualRoughnessToLod(float iblRoughnessOneLevel, float perceptualRoughness) {
    // The mapping below is a quadratic fit for log2(perceptualRoughness)+iblRoughnessOneLevel when
    // iblRoughnessOneLevel is 4. We found empirically that this mapping works very well for
    // a 256 cubemap with 5 levels used. But also scales well for other iblRoughnessOneLevel values.
    return iblRoughnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
}

vec3 prefilteredRadiance(samplerCube light_iblSpecular, const vec3 r, float perceptualRoughness, float iblRoughnessOneLevel) {
    float lod = perceptualRoughnessToLod(iblRoughnessOneLevel, perceptualRoughness);
    return decodeDataForIBL(textureCubeLod(light_iblSpecular, r, lod));
}

vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

void evaluateIBL(inout PixelParams pixel,
                 in vec3 vNormal,
                 in vec3 viewPos,
                 in mat4 invViewMat,
                 in sampler2D dfgTex,
                 in samplerCube iblEnvTex,
                 in float iblRoughnessOneLevel,
                 in float iblLuminance,
                 inout vec3 color)
{
    vec3 shading_normal = normalize(vNormal);
    vec3 shading_view = -normalize(viewPos);
    float shading_NoV = clampNoV(abs(dot(shading_normal, shading_view)));

    // the above is currently duplicated with CookTorrance

    vec3 shading_reflected = reflect(-shading_view, shading_normal);

    // Pre-filtered DFG term used for image-based lighting
    pixel.dfg = PrefilteredDFG_LUT(dfgTex, pixel.perceptualRoughness, shading_NoV);

    vec3 E = specularDFG(pixel);
    vec3 r = getSpecularDominantDirection(shading_normal, shading_reflected, pixel.roughness);

    // OGRE specific: convert r and n back to world space for texture sampling
    r = normalize(mul(invViewMat, vec4(r, 0.0)).xyz);
    r.z *= -1.0;
    shading_normal = normalize(mul(invViewMat, vec4(shading_normal, 0.0)).xyz);

    // specular layer
    vec3 Fr = E * prefilteredRadiance(iblEnvTex, r, pixel.perceptualRoughness, iblRoughnessOneLevel);

    vec3 diffuseIrradiance = Irradiance_RoughnessOne(iblEnvTex, shading_normal, iblRoughnessOneLevel);
    vec3 Fd = pixel.diffuseColor * diffuseIrradiance * (1.0 - E);

    Fr *= iblLuminance;
    Fd *= iblLuminance;

    // Combine all terms
    // Note: iblLuminance is already premultiplied by the exposure

    color = pow(color, vec3_splat(2.2)); // gamma to linear

    color += Fr + Fd;

    // linear to gamma
    color = pow(color, vec3_splat(1.0/2.2));
    color = saturate(color);
}