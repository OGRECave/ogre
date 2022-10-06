void transformToTS(in vec3 normal, inout vec3 lightDir, inout vec3 eyeDir)
{
    // derive the tangent space basis
    // we do this in the pixel shader because we don't have per-vertex normals
    // because of the LOD, we use a normal map
    // tangent is always +x or -z in object space depending on alignment
#ifdef TERRAIN_ALIGN_Y_Z
    vec3 tangent = vec3(0, 0, -1);
#else
    vec3 tangent = vec3(1, 0, 0);
#endif
    vec3 binormal = normalize(cross(tangent, normal));
    // note, now we need to re-cross to derive tangent again because it wasn't orthonormal
    tangent = normalize(cross(normal, binormal));
    // derive final matrix
    mat3 TBN = mtxFromRows(tangent, binormal, normal);

    lightDir = normalize(mul(TBN, lightDir));
    eyeDir = normalize(mul(TBN, eyeDir));
}

void blendTerrainLayer(in float blendWeight, in vec2 uv0, in float uvMul,
#ifdef TERRAIN_PARALLAX_MAPPING
                    in vec3 eyeDir, in vec2 scaleBias,
#endif
#ifdef TERRAIN_NORMAL_MAPPING
                    in sampler2D normtex, inout vec3 normal,
#endif
                    in sampler2D difftex, inout vec4 diffuseSpec)
{
    // generate UV
    vec2 uv = mod(uv0 * uvMul, 1.0);

#ifdef TERRAIN_PARALLAX_MAPPING
    // modify UV - note we have to sample an extra time
	float height = texture2D(normtex, uv).a;
	float displacement = (height * scaleBias.x) + scaleBias.y;
	uv += eyeDir.xy * displacement;
#endif

    // sample diffuse texture
    vec4 diffuseSpecTex = texture2D(difftex, uv);
    // apply to common
    diffuseSpec = mix(diffuseSpec, diffuseSpecTex, blendWeight);

#ifdef TERRAIN_NORMAL_MAPPING
    vec3 TSnormal;
    // access TS normal map
    SGX_FetchNormal(normtex, uv, TSnormal);
    normal += TSnormal * blendWeight;
#endif
}