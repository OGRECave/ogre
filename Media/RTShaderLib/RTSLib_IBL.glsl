// This file is part of the OGRE project.
// code adapted from Google Filament
// SPDX-License-Identifier: Apache-2.0

void IBL_ComputeWorldPos(in mat4 worldMat, in vec4 objectPos, out vec3 worldPos)
{
    worldPos = mul(worldMat, objectPos).xyz;
}

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

vec3 IrradianceMap(samplerCube iblDiffuse, const vec3 n) {
    return decodeDataForIBL(textureCubeLod(iblDiffuse, n, 0.0));
}

vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

float probeWeightBox(vec3 worldPos, vec3 probePos, vec3 extents, float blendDistWorld)
{
    vec3 delta = worldPos - probePos;

    vec3 ad;
    ad.x = (delta.x < 0.0) ? -delta.x : delta.x;
    ad.y = (delta.y < 0.0) ? -delta.y : delta.y;
    ad.z = (delta.z < 0.0) ? -delta.z : delta.z;

    vec3 d = ad - extents; // <=0 inside

    vec3 dpos;
    dpos.x = (d.x > 0.0) ? d.x : 0.0;
    dpos.y = (d.y > 0.0) ? d.y : 0.0;
    dpos.z = (d.z > 0.0) ? d.z : 0.0;

    float outsideDist = sqrt(dot(dpos, dpos));

    float bd = (blendDistWorld > 0.0001) ? blendDistWorld : 0.0001;

    float w = 1.0 - smoothstep(0.0, bd, outsideDist);

    return clamp(w, 0.0, 1.0);
}

vec3 evalSH9Diffuse(sampler2D shAtlasTex, vec2 shAtlasSize, float pid, vec3 n)
{
    vec2 invSize = 1.0 / shAtlasSize;
    vec2 base = vec2(0.0, pid + 0.5) * invSize;

    vec3 sh0 = texture2DLod(shAtlasTex, (vec2(0.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh1 = texture2DLod(shAtlasTex, (vec2(1.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh2 = texture2DLod(shAtlasTex, (vec2(2.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh3 = texture2DLod(shAtlasTex, (vec2(3.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh4 = texture2DLod(shAtlasTex, (vec2(4.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh5 = texture2DLod(shAtlasTex, (vec2(5.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh6 = texture2DLod(shAtlasTex, (vec2(6.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh7 = texture2DLod(shAtlasTex, (vec2(7.5, pid + 0.5)) * invSize, 0.0).rgb;
    vec3 sh8 = texture2DLod(shAtlasTex, (vec2(8.5, pid + 0.5)) * invSize, 0.0).rgb;

    float x = n.x, y = n.y, z = n.z;

    return
        sh0 * 0.2820947918 +
        sh1 * (0.4886025119 * y) +
        sh2 * (0.4886025119 * z) +
        sh3 * (0.4886025119 * x) +
        sh4 * (1.0925484306 * x * y) +
        sh5 * (1.0925484306 * y * z) +
        sh6 * (0.3153915653 * (3.0 * z * z - 1.0)) +
        sh7 * (1.0925484306 * x * z) +
        sh8 * (0.5462742153 * (x * x - y * y));
}

#ifdef HAS_LOCAL_PROBE
float boxProbeWeight(f32vec3 localPos,
                     f32vec3 boxExtents,
                     float blendDistance)
{
    f32vec3 p = abs(localPos) / max(boxExtents, f32vec3(0.001f, 0.001f, 0.001f));

    //0 at centre, 1 at box edge
    float edgeAmount = max(p.x, max(p.y, p.z));

    if (edgeAmount >= 1.0f)
        return 0.0f;

    //Center weighted. This makes overlapping probes blend across the overlap, not just in a thin line at the edge.
    float w = 1.0f - edgeAmount;

    return max(w * w, 0.001f);
}

f32vec3 quatRotateInverse(f32vec4 q, f32vec3 v)
{
    f32vec3 t = 2.0f * cross(-q.xyz, v);
    return v + q.w * t + cross(-q.xyz, t);
}

//f32Vec3 used throughout - important for precision.
void accumulateLocalProbe(
    sampler2D shAtlasTex,
    vec2 shAtlasSize,
    vec3 worldNormal,
    f32vec3 worldPos,
    f32vec4 probePosID,
    f32vec4 probeExtentsBlend,
    f32vec4 probeOrientation,
    inout vec3 localIrr,
    inout float totalW)
{
    float probeId = probePosID.w;

    if (probeId < 0.0f)
        return;

    f32vec3 probePos = probePosID.xyz;
    f32vec3 boxExtents = probeExtentsBlend.xyz;
    float blendDistance = probeExtentsBlend.w;

    //Convert world position into probe-local space
    f32vec3 probeDelta = worldPos - probePos;
    f32vec3 probeLocalPos = quatRotateInverse(probeOrientation, probeDelta);

    //Box test in local probe space
    float w = boxProbeWeight(probeLocalPos, boxExtents, blendDistance);

    if (w <= 0.0001f)
        return;

    vec3 irr = evalSH9Diffuse(shAtlasTex, shAtlasSize, probeId, -worldNormal);

    localIrr += irr * w;
    totalW += w;
}
#endif

void evaluateIBL(inout PixelParams pixel,
                 in vec3 vNormal,
                 in vec3 viewPos,
                 #ifdef HAS_LOCAL_PROBE
                 in f32vec3 worldPos,
                 #endif
                 in mat4 invViewMat,
                 in sampler2D dfgTex,
                 in samplerCube iblEnvTex,
                 #ifdef HAS_DUAL_TEXTURE
                 in samplerCube iblEnvTexSpecular,
                 #endif
                 in float iblRoughnessOneLevel,
                 in float iblLuminance,

                 #ifdef HAS_LOCAL_PROBE
                 in sampler2D shAtlasTex,
                 in vec2 shAtlasSize,

                 //Local probe 1 data
                 in f32vec4 localProbePosID_1,
                 in f32vec4 localProbExtentsBlend_1,
                 in f32vec4 localProbeOrientation_1,

                 in f32vec4 localProbePosID_2,
                 in f32vec4 localProbExtentsBlend_2,
                 in f32vec4 localProbeOrientation_2,

                 in f32vec4 localProbePosID_3,
                 in f32vec4 localProbExtentsBlend_3,
                 in f32vec4 localProbeOrientation_3,

                 in f32vec4 localProbePosID_4,
                 in f32vec4 localProbExtentsBlend_4,
                 in f32vec4 localProbeOrientation_4,

                 #ifdef HAS_VERTEX_COLOUR_PROBE_MASK
                 in float vertexProbeMask,
                 #endif
                 #endif

                 inout vec3 color)
{
    vec3 shading_normal = normalize(vNormal);
    vec3 shading_view = -normalize(viewPos);
    float shading_NoV = clampNoV(abs(dot(shading_normal, shading_view)));

    vec3 shading_reflected = reflect(-shading_view, shading_normal);

    //Pre-filtered DFG term
    pixel.dfg = PrefilteredDFG_LUT(dfgTex, pixel.perceptualRoughness, shading_NoV);

    vec3 E = specularDFG(pixel);
    vec3 r = getSpecularDominantDirection(shading_normal, shading_reflected, pixel.roughness);

    //Convert to world space
    r = normalize(mul(invViewMat, vec4(r, 0.0)).xyz);
    r.z *= -1.0;
    shading_normal = normalize(mul(invViewMat, vec4(shading_normal, 0.0)).xyz);

    //Global IBL
    vec3 Fr_global;
    vec3 diffuseIrr_global;

    #ifdef HAS_DUAL_TEXTURE
        Fr_global = E * prefilteredRadiance(
            iblEnvTexSpecular, r, pixel.perceptualRoughness, iblRoughnessOneLevel);
        diffuseIrr_global = IrradianceMap(iblEnvTex, shading_normal) * 0.5f;
    #else
        Fr_global = E * prefilteredRadiance(
            iblEnvTex, r, pixel.perceptualRoughness, iblRoughnessOneLevel);
        diffuseIrr_global = Irradiance_RoughnessOne(
            iblEnvTex, shading_normal, iblRoughnessOneLevel);
    #endif

    vec3 Fd_global = pixel.diffuseColor * diffuseIrr_global * (1.0 - E) * pixel.ao;

    Fr_global *= iblLuminance;
    Fr_global *= mix(pixel.ao, 1.0, 0.7);
    Fd_global *= iblLuminance;

    vec3 Fr = Fr_global;
    vec3 Fd = Fd_global;

    #ifdef HAS_LOCAL_PROBE
    {
        vec3 localIrr = vec3_splat(0.0);
        float totalW = 0.0;

        accumulateLocalProbe(shAtlasTex, shAtlasSize, shading_normal, worldPos,
            localProbePosID_1, localProbExtentsBlend_1, localProbeOrientation_1, localIrr, totalW);

        accumulateLocalProbe(shAtlasTex, shAtlasSize, shading_normal, worldPos,
            localProbePosID_2, localProbExtentsBlend_2, localProbeOrientation_2, localIrr, totalW);

        accumulateLocalProbe(shAtlasTex, shAtlasSize, shading_normal, worldPos,
            localProbePosID_3, localProbExtentsBlend_3, localProbeOrientation_3, localIrr, totalW);

        accumulateLocalProbe(shAtlasTex, shAtlasSize, shading_normal, worldPos,
            localProbePosID_4, localProbExtentsBlend_4, localProbeOrientation_4, localIrr, totalW);

        #ifdef HAS_VERTEX_COLOUR_PROBE_MASK
            float localOnly = 1.0f - saturate(vertexProbeMask); // black = local
        #else
            float localOnly = 0.0f;
        #endif

        if (totalW > 0.0001f)
        {
            //This is the actual probe-to-probe blend. It normalises all overlapping local probes.
            localIrr /= totalW;

            vec3 Fd_local =
                pixel.diffuseColor * localIrr * (1.0f - E) * iblLuminance * pixel.ao;

            #ifdef HAS_VERTEX_COLOUR_PROBE_MASK
                if (localOnly > 0.5f)
                {
                    //Interior/local-only surface - no global contribution while any local probe exists.
                    Fd = Fd_local;
                }
                else
                {
                    //Outdoor/mixed surface -  normal local vs global blending
                    float localBlend = saturate(totalW);
                    Fd = mix(Fd_global, Fd_local, localBlend);
                }
            #else
                float localBlend = saturate(totalW);
                Fd = mix(Fd_global, Fd_local, localBlend);
            #endif
        }
    }
    #endif

    #ifndef USE_LINEAR_COLOURS
        color = pow(color, vec3_splat(2.2));
    #endif

    color += Fr + Fd;

    #ifndef USE_LINEAR_COLOURS
        color = pow(color, vec3_splat(1.0 / 2.2));
        color = saturate(color);
    #endif
}