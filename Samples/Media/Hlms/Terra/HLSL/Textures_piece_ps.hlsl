
@undefpiece( kD )
@piece( kD )diffuseCol@end

@undefpiece( kS )
@piece( kS )float3( 1, 1, 1 )@end

@add( textureRegStart, 2 )
@add( envMapReg, 2 )
@add( textureRegShadowMapStart, 2 )
@add( samplerStateStart, 2 )

@undefpiece( diffuseExtraParam )
@undefpiece( specularExtraParam )
@undefpiece( roughnessExtraParam )
@undefpiece( metallicExtraParam )

@undefpiece( diffuseExtraParamDef )
@undefpiece( specularExtraParamDef )
@undefpiece( roughnessExtraParamDef )
@undefpiece( metallicExtraParamDef )

@piece( diffuseExtraParamDef ), float3 diffuseCol@end
@piece( diffuseExtraParam ), diffuseCol.xyz@end
@piece( metallicExtraParamDef ), float3 F0@end
@piece( metallicExtraParam ), F0@end
@piece( roughnessExtraParamDef ), float ROUGHNESS@end
@piece( roughnessExtraParam ), ROUGHNESS@end

@foreach( detail_maps_normal, n )
	@undefpiece( SampleDetailMapNm@n )
	@piece( SampleDetailMapNm@n )getTSDetailNormal( samplerStates[@value(detail_map_nm@n_idx)], textureMaps[@value(detail_map_nm@n_idx)],
													float3( inPs.uv0.xy * material.detailOffsetScale[@n].zw +
															material.detailOffsetScale[@n].xy, @value(detail_map_nm@n_slice_idx) ) )@end
@end
