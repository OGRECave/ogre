@piece( SetCrossPlatformSettings )
#define INLINE

#define finalDrawId input.drawId

#define outVs_Position outVs.gl_Position
#define OGRE_SampleLevel( tex, sampler, uv, lod ) tex.SampleLevel( sampler, uv.xy, lod )
@end
