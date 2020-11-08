float calcSimpleShadow(sampler2D shadowMap, vec4 shadowMapPos)
{
    return texture2DProj(shadowMap, shadowMapPos).x;
}

void calcPSSMSimpleShadow(float camDepth, vec4 pssmSplitPoints,
    vec4 lsPos0, sampler2D shadowMap0,
    #if PSSM_NUM_SPLITS > 2
    vec4 lsPos1, sampler2D shadowMap1,
    #endif
    #if PSSM_NUM_SPLITS > 3
    vec4 lsPos2, sampler2D shadowMap2,
    #endif
    vec4 lsPos3, sampler2D shadowMap3,
    out float shadow)
{
    if (camDepth <= pssmSplitPoints.r)
    {
        shadow = calcSimpleShadow(shadowMap0, lsPos0);
    }
#if PSSM_NUM_SPLITS > 2
    else if (camDepth <= pssmSplitPoints.g)
    {
        shadow = calcSimpleShadow(shadowMap1, lsPos1);
    }
#endif
#if PSSM_NUM_SPLITS > 3
    else if (camDepth <= pssmSplitPoints.b)
    {
        shadow = calcSimpleShadow(shadowMap2, lsPos2);
    }
#endif
    else
    {
        shadow = calcSimpleShadow(shadowMap3, lsPos3);
    }
}
