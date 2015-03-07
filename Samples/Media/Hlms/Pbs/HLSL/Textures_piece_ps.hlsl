
//Set the sampler starts. Note that 'padd' get calculated before _any_ 'add'
@pset( textureRegShadowMapStart, 1 )
@padd( textureRegStart, hlms_num_shadow_maps, 1 )
@padd( envMapReg, textureRegStart, num_textures )

@padd( samplerStateStart, hlms_num_shadow_maps, 1 )
@pset( numSamplerStates, num_textures )
@property( envprobe_map )
	@add( numSamplerStates, 1 )
@end

@property( hlms_forward3d )
	@add( textureRegStart, 2 )
	@add( envMapReg, 2 )
	@add( textureRegShadowMapStart, 2 )
@end

@property( diffuse_map )
	@piece( SampleDiffuseMap )	diffuseCol = textureMaps[@value( diffuse_map_idx )].Sample( samplerStates[@value(diffuse_map_idx)], float3( inPs.uv@value(uv_diffuse).xy, diffuseIdx ) );
@property( !hw_gamma_read )	diffuseCol = diffuseCol * diffuseCol;@end @end
@end

@property( diffuse_map || detail_maps_diffuse )
    @piece( MulDiffuseMapValue )* diffuseCol.xyz@end
	@piece( diffuseExtraParamDef ), float3 diffuseCol@end
	@piece( diffuseExtraParam ), diffuseCol.xyz@end
@end

@property( specular_map )
	@piece( SampleSpecularMap )	specularCol = textureMaps[@value( specular_map_idx )].Sample( samplerStates[@value(specular_map_idx)], float3(inPs.uv@value(uv_specular).xy, specularIdx) ).xyz;@end
	@piece( MulSpecularMapValue )* specularCol@end
	@piece( specularExtraParamDef ), float3 specularCol@end
	@piece( specularExtraParam ), specularCol.xyz@end
@end

@property( roughness_map )
	@piece( SampleRoughnessMap )	ROUGHNESS = material.kS.w * (1.0f - textureMaps[@value( roughness_map_idx )].Sample( samplerStates[@value( roughness_map_idx )], float3(inPs.uv@value(uv_roughness).xy, roughnessIdx) ).x);
	ROUGHNESS = max( ROUGHNESS, 0.001f );@end
	@piece( roughnessExtraParamDef ), float ROUGHNESS@end
	@piece( roughnessExtraParam ), ROUGHNESS@end
@end

@piece( brdfExtraParamDefs )@insertpiece( diffuseExtraParamDef )@insertpiece( specularExtraParamDef )@insertpiece( roughnessExtraParamDef )@end
@piece( brdfExtraParams )@insertpiece( diffuseExtraParam )@insertpiece( specularExtraParam )@insertpiece( roughnessExtraParam )@end

@foreach( detail_maps_normal, n )
	@piece( SampleDetailMapNm@n )getTSDetailNormal( samplerStates[@value(detail_map_nm@n_idx)], textureMaps[@value(detail_map_nm@n_idx)], float3( inPs.uv@value(uv_detail_nm@n).xy@insertpiece( offsetDetailN@n ), detailNormMapIdx@n ) ) * detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul )@end
@end

@property( detail_weight_map )
	@piece( SamplerDetailWeightMap )textureMaps[@value(detail_weight_map_idx)].Sample( samplerStates[@value(detail_weight_map_idx)], float3(inPs.uv@value(uv_detail_weight).xy, weightMapIdx) )@end
@end


// Get the indexes to the textureMaps[] array using template code. We had to add 1
// to the actual value otherwise property( diffuse_map ) fails when the index is 0
@sub( diffuse_map_idx, diffuse_map, 1 )
@sub( normal_map_tex_idx, normal_map_tex, 1 )
@sub( specular_map_idx, specular_map, 1 )
@sub( roughness_map_idx, roughness_map, 1 )
@sub( detail_weight_map_idx, detail_weight_map, 1 )
@foreach( 4, n )
	@sub( detail_map@n_idx, detail_map@n, 1 )@end
@foreach( 4, n )
	@sub( detail_map_nm@n_idx, detail_map_nm@n, 1 )@end
