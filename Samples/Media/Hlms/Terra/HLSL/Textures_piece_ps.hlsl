
@undefpiece( kS )
@piece( kS )float3( 1, 1, 1 )@end

@add( textureRegStart, 2 )
@add( envMapReg, 2 )
@add( textureRegShadowMapStart, 2 )
@add( samplerStateStart, 2 )

@undefpiece( specularExtraParam )
@undefpiece( roughnessExtraParam )
@undefpiece( metallicExtraParam )

@undefpiece( specularExtraParamDef )
@undefpiece( roughnessExtraParamDef )
@undefpiece( metallicExtraParamDef )

@property( metalness_map0 || metalness_map1 || metalness_map2 || metalness_map3 )
	@piece( metallicExtraParamDef ), float3 F0@end
	@piece( metallicExtraParam ), F0@end
@end
@property( roughness_map0 || roughness_map1 || roughness_map2 || roughness_map3 )
	@piece( roughnessExtraParamDef ), float ROUGHNESS@end
	@piece( roughnessExtraParam ), ROUGHNESS@end
@end

@foreach( detail_maps_normal, n )
	@undefpiece( SampleDetailMapNm@n )
	@piece( SampleDetailMapNm@n )getTSDetailNormal( samplerStates[@value(detail_map_nm@n_idx)], textureMaps[@value(detail_map_nm@n_idx)],
													float3( inPs.uv0.xy * material.detailOffsetScale[@n].zw +
															material.detailOffsetScale[@n].xy, @value(detail_map_nm@n_slice_idx) ) )@end
@end
