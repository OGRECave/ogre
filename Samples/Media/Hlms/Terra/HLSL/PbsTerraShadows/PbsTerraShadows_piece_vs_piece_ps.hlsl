@property( !hlms_shadowcaster && terra_enabled )

@piece( custom_VStoPS )
    float terrainShadow : TEXCOORD@counter(texcoord);
@end

/// Extra per-pass global data we need for applying our
/// shadows to regular objects, passed to all PBS shaders.
@piece( custom_passBuffer )
    float4 terraOrigin; //Normalized. i.e. -terrainOrigin / terrainDimensions
    //.xz = terrain 1.0 / XZ dimensions.
    //.y  = 1.0 / terrainHeight;
    float4 invTerraBounds;
@end

/// Add the shadows' texture to the vertex shader
@piece( custom_vs_uniformDeclaration )
	SamplerState terrainShadowSampler : register(s12);
    Texture2D<unorm float4> terrainShadows : register(t12);
@end

/// Evaluate the shadow based on world XZ position & height in the vertex shader.
/// Doing it at the pixel shader level would be more accurate, but the difference
/// is barely noticeable, and slower
@piece( custom_vs_posExecution )
    float3 terraShadowData = terrainShadows.SampleLevel( terrainShadowSampler, worldPos.xz * passBuf.invTerraBounds.xz + passBuf.terraOrigin.xz, 0 ).xyz;
    float terraHeightWeight = worldPos.y * passBuf.invTerraBounds.y + passBuf.terraOrigin.y;
    terraHeightWeight = (terraHeightWeight - terraShadowData.y) * terraShadowData.z * 1023.0;
    outVs.terrainShadow = lerp( terraShadowData.x, 1.0, saturate( terraHeightWeight ) );
@end

@property( hlms_num_shadow_map_lights )
    @piece( custom_ps_preLights )fShadow *= inPs.terrainShadow;@end
@end @property( !hlms_num_shadow_map_lights )
    @piece( custom_ps_preLights )float fShadow = inPs.terrainShadow;@end
    @piece( DarkenWithShadowFirstLight )* fShadow@end
@end

@end
