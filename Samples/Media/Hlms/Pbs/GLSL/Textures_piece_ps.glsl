@property( diffuse_map )
	@property( !hlms_shadowcaster )
		@piece( SampleDiffuseMap )	diffuseCol = texture( textureMaps[@value( diffuse_map_idx )], vec3( inPs.uv@value(uv_diffuse).xy, diffuseIdx ) );
@property( !hw_gamma_read )	diffuseCol = diffuseCol * diffuseCol;@end @end
	@end @property( hlms_shadowcaster )
		@piece( SampleDiffuseMap )	diffuseCol = texture( textureMaps[@value( diffuse_map_idx )], vec3( inPs.uv@value(uv_diffuse).xy, diffuseIdx ) ).w;@end
	@end
@end

@property( diffuse_map || detail_maps_diffuse )
    //diffuseCol is multiplied against material.kD in PixelShader_ps as it is influenced by the detail maps.
	@piece( kD )diffuseCol@end
@end @property( !diffuse_map && !detail_maps_diffuse )
	@piece( kD )material.kD@end
@end

@property( !metallic_workflow )
	@property( specular_map && !fresnel_workflow )
		@piece( SampleSpecularMap )	specularCol = texture( textureMaps[@value( specular_map_idx )], vec3(inPs.uv@value(uv_specular).xy, specularIdx) ).xyz * material.kS.xyz;@end
		@piece( kS )specularCol@end
	@end
	@property( specular_map && fresnel_workflow )
		@piece( SampleSpecularMap )	F0 = texture( textureMaps[@value( specular_map_idx )], vec3(inPs.uv@value(uv_specular).xy, specularIdx) ).@insertpiece( FresnelSwizzle ) * material.F0.@insertpiece( FresnelSwizzle );@end
	@end
	@property( !specular_map || fresnel_workflow )
		@piece( kS )material.kS@end
	@end
@end @property( metallic_workflow )
@piece( SampleSpecularMap )
	@property( specular_map )
		float metalness = texture( textureMaps[@value( specular_map_idx )], vec3(inPs.uv@value(uv_specular).xy, specularIdx) ).x * material.F0.x;
		F0 = mix( vec3( 0.03f ), @insertpiece( kD ).xyz * 3.14159f, metalness );
		@insertpiece( kD ).xyz = @insertpiece( kD ).xyz - @insertpiece( kD ).xyz * metalness;
	@end @property( !specular_map )
		F0 = mix( vec3( 0.03f ), @insertpiece( kD ).xyz * 3.14159f, material.F0.x );
		@insertpiece( kD ).xyz = @insertpiece( kD ).xyz - @insertpiece( kD ).xyz * material.F0.x;
	@end
	@property( hlms_alphablend )F0 *= material.F0.w;@end
	@property( transparent_mode )F0 *= diffuseCol.w;@end
@end /// SampleSpecularMap

	@piece( kS )material.kS.xyz@end
@end

@property( roughness_map )
	@piece( SampleRoughnessMap )ROUGHNESS = material.kS.w * texture( textureMaps[@value( roughness_map_idx )], vec3(inPs.uv@value(uv_roughness).xy, roughnessIdx) ).x;
ROUGHNESS = max( ROUGHNESS, 0.001f );@end
@end

@foreach( detail_maps_normal, n )
	@piece( SampleDetailMapNm@n )getTSDetailNormal( textureMaps[@value(detail_map_nm@n_idx)], vec3( inPs.uv@value(uv_detail_nm@n).xy@insertpiece( offsetDetailN@n ), detailNormMapIdx@n ) ) * detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul )@end
@end

@property( detail_weight_map )
	@piece( SamplerDetailWeightMap )texture( textureMaps[@value(detail_weight_map_idx)], vec3(inPs.uv@value(uv_detail_weight).xy, weightMapIdx) )@end
@end

@property( envmap_scale )
	@piece( ApplyEnvMapScale )* pass.ambientUpperHemi.w@end
@end
