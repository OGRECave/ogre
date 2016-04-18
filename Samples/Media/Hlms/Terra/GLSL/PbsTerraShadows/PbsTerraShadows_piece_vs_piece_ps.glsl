@property( !hlms_shadowcaster && terra_enabled )

@piece( custom_VStoPS )
    float terrainShadow;
@end

/// Extra per-pass global data we need for applying our
/// shadows to regular objects, passed to all PBS shaders.
@piece( custom_passBuffer )
    //.xz = terrain XZ dimensions.
    //.y  = 1.0 / terrainHeight;
    //.w  = -(terrainOrigin.y / terrainHeight);
    vec4 terraBounds;
@end

/// Add the shadows' texture to the vertex shader
@piece( custom_vs_uniformDeclaration )
    uniform sampler2D terrainShadows;
@end

/// Evaluate the shadow based on world XZ position & height in the vertex shader.
/// Doing it at the pixel shader level would be more accurate, but the difference
/// is barely noticeable, and slower
@piece( custom_vs_posExecution )
    vec3 terraShadowData = texture( terrainShadows, worldPos.xz * pass.terraBounds.xz ).xyz;
    float terraHeightWeight = worldPos.y * pass.terraBounds.y + pass.terraBounds.w;
    terraHeightWeight = (terraHeightWeight - terraShadowData.y) * terraShadowData.z * 1023.0;
    outVs.terrainShadow = mix( terraShadowData.x, 1.0, clamp( terraHeightWeight, 0.0, 1.0 ) );
@end

@property( hlms_num_shadow_maps )
    @piece( custom_ps_preLights )fShadow *= inPs.terrainShadow;@end
@end @property( !hlms_num_shadow_maps )
    @piece( custom_ps_preLights )float fShadow = inPs.terrainShadow;@end
@end

@piece( custom_ps_posExecution )	outColour.xyz = fShadow.xxx;@end

@end
