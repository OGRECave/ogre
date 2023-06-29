void transformToTS(in vec3 TSnormal, in mat3 normalMatrix, inout vec3 normal)
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
    normal = normalize(normal);
    tangent = normalize(mul(normalMatrix, tangent));
    vec3 binormal = cross(tangent, normal);
    // note, now we need to re-cross to derive tangent again because it wasn't orthonormal
    tangent = cross(normal, binormal);
    // derive final matrix
    mat3 TBN = mtxFromCols(tangent, binormal, normal);

    normal = mul(TBN, TSnormal);
}

void getShadowFactor(in sampler2D lightmap, in vec2 uv, inout float shadowFactor)
{
    float lmShadow = texture2D(lightmap, uv).x;
    shadowFactor = min(shadowFactor, lmShadow);
}

#define MIN_BLEND_WEIGHT 0.0039 // 1/255

void blendTerrainLayer(in float blendWeight, in f32vec2 uv0, in float uvMul,
#ifdef TERRAIN_PARALLAX_MAPPING
                    in vec3 viewPos, in float scaleBias, in mat3 TBN,
#endif
#ifdef TERRAIN_NORMAL_MAPPING
                    in sampler2D normtex, inout vec3 normal,
#endif
                    in sampler2D difftex, inout vec4 diffuseSpec)
{
    if(blendWeight < MIN_BLEND_WEIGHT)
        return;

    // generate UV
    vec2 uv = mod(uv0 * uvMul, 1.0);

#ifdef TERRAIN_PARALLAX_MAPPING
    SGX_Generate_Parallax_Texcoord(normtex, uv, viewPos, scaleBias, TBN, uv);
#endif

    // sample diffuse texture
    vec4 diffuseSpecTex = texture2D(difftex, uv);
    // apply to common
    diffuseSpec = mix(diffuseSpec, diffuseSpecTex, blendWeight);

#ifdef TERRAIN_NORMAL_MAPPING
    vec3 TSnormal;
    // access TS normal map
    SGX_FetchNormal(normtex, uv, TSnormal);
    // Partial Derivative Blending https://blog.selfshadow.com/publications/blending-in-detail/
    normal = normalize(vec3(mix(normal.xy*TSnormal.z, TSnormal.xy*normal.z, blendWeight), TSnormal.z*normal.z));
#endif
}

//-----------------------------------------------------------------------------
void SGX_CalculateTerrainTBN(in vec3 normal, in mat3 normalMatrix, out mat3 TBN)
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
    normal = normalize(normal);
    tangent = normalize(mul(normalMatrix, tangent));
    vec3 binormal = cross(tangent, normal);
    // note, now we need to re-cross to derive tangent again because it wasn't orthonormal
    tangent = cross(normal, binormal);
    // derive final matrix
    TBN = mtxFromCols(tangent, binormal, normal);
}