@piece( SetCrossPlatformSettings )
#define INLINE

#define finalDrawId input.drawId

#define outVs_Position outVs.gl_Position
#define OGRE_SampleLevel( tex, sampler, uv, lod ) tex.SampleLevel( sampler, uv.xy, lod )
#define OGRE_SampleArray2DLevel( tex, sampler, uv, arrayIdx, lod ) tex.SampleLevel( sampler, float3( uv, arrayIdx ), lod )
@end
