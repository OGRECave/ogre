@property( diffuse_map )
	@piece( SampleDiffuseMap )	diffuseCol = texture( textureMaps[@value( diffuse_map_idx )], vec3( inPs.uv@value(uv_diffuse).xy, diffuseIdx ) );
@property( !hw_gamma_read )	diffuseCol = diffuseCol * diffuseCol;@end @end
	@piece( MulDiffuseMapValue )* diffuseCol.xyz@end
@end

@property( specular_map )
	@piece( SampleSpecularMap )	specularCol = texture( textureMaps[@value( specular_map_idx )], vec3(inPs.uv@value(uv_specular).xy, specularIdx) ).xyz;@end
	@piece( MulSpecularMapValue )* specularCol@end
@end

@property( roughness_map )
	@piece( SampleRoughnessMap )ROUGHNESS = material.kS.w * texture( textureMaps[@value( roughness_map_idx )], vec3(inPs.uv@value(uv_roughness).xy, roughnessIdx) ).x;@end
@end

@foreach( detail_maps_normal, n )
	@piece( SampleDetailMapNm@n )getTSDetailNormal( textureMaps[@value(detail_map_nm@n_idx)], vec3( inPs.uv@value(uv_detail_nm@n).xy@insertpiece( offsetDetailN@n ), detailNormMapIdx@n ) ) * detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul )@end
@end

@property( detail_weight_map )
	@piece( SamplerDetailWeightMap )texture( textureMaps[@value(detail_weight_map_idx)], vec3(inPs.uv@value(uv_detail_weight).xy, weightMapIdx) )@end
@end


// Get the indexes to the textureMaps[] array using template code. We had to add 1
// to the actual value otherwise @property( diffuse_map ) fails when the index is 0
@sub( diffuse_map_idx, diffuse_map, 1 )
@sub( normal_map_tex_idx, normal_map_tex, 1 )
@sub( specular_map_idx, specular_map, 1 )
@sub( roughness_map_idx, roughness_map, 1 )
@sub( detail_weight_map_idx, detail_weight_map, 1 )
@foreach( 4, n )
	@sub( detail_map@n_idx, detail_map@n, 1 )@end
@foreach( 4, n )
	@sub( detail_map_nm@n_idx, detail_map_nm@n, 1 )@end
