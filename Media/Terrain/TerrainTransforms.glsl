/**
 * @param delta: lodDelta, lodThreshold (vertex attribute)
 * @param lodMorph: morph amount, morph targetLOD (uniform)
 */
void applyLODMorph(vec2 delta, vec2 lodMorph, inout float height
#ifdef TERRAIN_DEBUG
, out vec2 lodInfo
#endif
)
{
    // determine whether to apply the LOD morph to this vertex
    // we store the deltas against all vertices so we only want to apply
    // the morph to the ones which would disappear.
    // If we subtract
    // the lodThreshold from the targetLOD, and arrange to only morph if the
    // result is negative (it will only be -1 in fact, since after that
    // the vertex will never be indexed), we will achieve our aim.
    // sign(lodThreshold - targetLOD) == -1 is to morph

    // this will either be 1 (morph) or 0 (don't morph)
    float toMorph = -min(0.0, sign(delta.y - lodMorph.y));
    height += delta.x * toMorph * lodMorph.x;

#ifdef TERRAIN_DEBUG
    // LOD level (-1 since value is target level, we want to display actual)
    lodInfo.x = (lodMorph.y - 1) / NUM_LODS;
    // LOD morph
    lodInfo.y = toMorph * lodMorph.x;
#endif
}

void expandVertex(mat4 idxToObjectSpace, float baseUVScale, vec2 idx, float height, out vec4 position, out vec2 uv)
{
    position = mul(idxToObjectSpace, vec4(idx, height, 1));
    uv = vec2(idx.x * baseUVScale, 1.0 - idx.y * baseUVScale);
}