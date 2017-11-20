
//Set the sampler starts. Note that 'padd' get calculated before _any_ 'add'
@set( texUnit, 1 )

@property( hlms_forwardplus )
	@add( texUnit, 2 )
@end

@property( hlms_use_prepass )
	@set( gBuf_normals, texUnit )
	@add( gBuf_shadowRoughness, texUnit, 1 )
	@add( texUnit, 2 )

	@property( hlms_use_ssr )
		@set( ssrTexture, texUnit )
		@add( texUnit, 1 )
	@end
@end

@property( irradiance_volumes )
	@set( irradianceVolumeTexUnit, texUnit )
	@add( texUnit, 1 )
@end

@set( textureRegShadowMapStart, texUnit )
@add( texUnit, hlms_num_shadow_map_textures )

@property( parallax_correct_cubemaps )
	@set( globaPccTexUnit, texUnit )
	@add( texUnit, 1 )
@end

@property( has_planar_reflections )
	@set( planarReflectionTexUnit, texUnit )
	@add( texUnit, 1 )
@end

@set( textureRegStart, texUnit )
@set( samplerStateStart, texUnit )
@set( numSamplerStates, num_textures )
@add( texUnit, num_textures )

@set( envMapReg, texUnit )

@property( (envprobe_map && envprobe_map != target_envprobe_map) || parallax_correct_cubemaps )
	@set( use_envprobe_map, 1 )

	@property( !envprobe_map || envprobe_map == target_envprobe_map )
		/// "No cubemap"? Then we're in auto mode or...
		/// We're rendering to the cubemap probe we're using as manual. Use the auto mode as fallback.
		@piece( pccProbeSource )passBuf.autoProbe@end
		@set( use_parallax_correct_cubemaps, 1 )
		/// Auto cubemap textures are set at the beginning. Manual cubemaps are the end.
		@set( envMapReg, globaPccTexUnit )
	@end
	@property( envprobe_map && envprobe_map != target_envprobe_map && use_parallax_correct_cubemaps )
		@piece( pccProbeSource )manualProbe@end
	@end
@end

@property( diffuse_map )
	@property( !hlms_shadowcaster )
		@piece( SampleDiffuseMap )	diffuseCol = textureMaps@value( diffuse_map_idx ).sample(
												samplerState@value(diffuse_map_idx),
												UV_DIFFUSE( inPs.uv@value(uv_diffuse).xy ),
												diffuseIdx );
@property( !hw_gamma_read )	diffuseCol = diffuseCol * diffuseCol;@end @end
	@end @property( hlms_shadowcaster )
		@piece( SampleDiffuseMap )	diffuseCol = textureMaps@value( diffuse_map_idx ).sample(
												samplerState@value(diffuse_map_idx),
												UV_DIFFUSE( inPs.uv@value(uv_diffuse).xy ),
												diffuseIdx ).w;@end
	@end
@end

@property( transparent_mode )
	@piece( diffuseExtraParamDef ), float4 diffuseCol@end
	@piece( diffuseExtraParam ), diffuseCol.xyzw@end
@end @property( !transparent_mode )
	@piece( diffuseExtraParamDef ), float3 diffuseCol@end
	@piece( diffuseExtraParam ), diffuseCol.xyz@end
@end
//diffuseCol always has some colour and is multiplied against material.kD in PixelShader_ps.
@piece( kD )diffuseCol@end

@property( !hlms_prepass )
@property( !metallic_workflow )
	@property( specular_map && !fresnel_workflow )
		@piece( SampleSpecularMap )	specularCol = textureMaps@value( specular_map_idx ).sample(
												samplerState@value(specular_map_idx),
												UV_SPECULAR( inPs.uv@value(uv_specular).xy ),
												specularIdx ).xyz * material.kS.xyz;@end
		@piece( specularExtraParamDef ), float3 specularCol@end
		@piece( specularExtraParam ), specularCol.xyz@end
		@piece( kS )specularCol@end
	@end
	@property( specular_map && fresnel_workflow )
		@piece( SampleSpecularMap )	F0 = textureMaps@value( specular_map_idx ).sample(
											samplerState@value(specular_map_idx),
											UV_SPECULAR( inPs.uv@value(uv_specular).xy ),
											specularIdx ).@insertpiece( FresnelSwizzle ) * material.F0.@insertpiece( FresnelSwizzle );@end
		@piece( specularExtraParamDef ), @insertpiece( FresnelType ) F0@end
		@piece( specularExtraParam ), F0@end
	@end
	@property( !specular_map || fresnel_workflow )
		@piece( kS )material.kS@end
	@end
@end @property( metallic_workflow )
@piece( SampleSpecularMap )
	@property( specular_map )
		float metalness = textureMaps@value( specular_map_idx ).sample(
												samplerState@value(specular_map_idx),
												UV_SPECULAR( inPs.uv@value(uv_specular).xy ),
												specularIdx ).x * material.F0.x;
		F0 = mix( 0.03f, @insertpiece( kD ).xyz * 3.14159f, metalness );
		@insertpiece( kD ).xyz = @insertpiece( kD ).xyz - @insertpiece( kD ).xyz * metalness;
	@end @property( !specular_map )
		F0 = mix( 0.03f, @insertpiece( kD ).xyz * 3.14159f, material.F0.x );
		@insertpiece( kD ).xyz = @insertpiece( kD ).xyz - @insertpiece( kD ).xyz * material.F0.x;
	@end
	@property( hlms_alphablend )F0 *= material.F0.w;@end
	@property( transparent_mode )F0 *= diffuseCol.w;@end
@end /// SampleSpecularMap

	@piece( kS )material.kS.xyz@end
	@piece( metallicExtraParamDef ), float3 F0@end
	@piece( metallicExtraParam ), F0@end
@end
@end

@property( roughness_map )
	@piece( SampleRoughnessMap )	ROUGHNESS = material.kS.w * textureMaps@value( roughness_map_idx ).sample(
														samplerState@value( roughness_map_idx ),
														UV_ROUGHNESS( inPs.uv@value(uv_roughness).xy ),
														roughnessIdx ).x;
	ROUGHNESS = max( ROUGHNESS, 0.001f );@end
	@piece( roughnessExtraParamDef ), float ROUGHNESS@end
	@piece( roughnessExtraParam ), ROUGHNESS@end
@end

@piece( brdfExtraParamDefs )@insertpiece( diffuseExtraParamDef )@insertpiece( specularExtraParamDef )@insertpiece( roughnessExtraParamDef )@insertpiece( metallicExtraParamDef )@end
@piece( brdfExtraParams )@insertpiece( diffuseExtraParam )@insertpiece( specularExtraParam )@insertpiece( roughnessExtraParam )@insertpiece( metallicExtraParam )@end

@foreach( detail_maps_normal, n )
	@piece( SampleDetailMapNm@n )getTSNormal( samplerState@value(detail_map_nm@n_idx),
												textureMaps@value(detail_map_nm@n_idx),
												UV_DETAIL_NM@n( inPs.uv@value(uv_detail_nm@n).xy@insertpiece( offsetDetail@n ) ),
												detailNormMapIdx@n ) * detailWeights.@insertpiece(detail_swizzle@n)
												@insertpiece( detail@n_nm_weight_mul )@end
@end

@property( detail_weight_map )
	@piece( SamplerDetailWeightMap )textureMaps@value(detail_weight_map_idx).sample(
											samplerState@value(detail_weight_map_idx),
											UV_DETAIL_WEIGHT( inPs.uv@value(uv_detail_weight).xy ),
											weightMapIdx )@end
@end

@property( envmap_scale )
	@piece( ApplyEnvMapScale )* passBuf.ambientUpperHemi.w@end
@end

@property( emissive_map )
	@piece( SampleEmissiveMap )
		float3 emissiveCol = textureMaps@value( emissive_map_idx ).sample(
										samplerState@value(emissive_map_sampler),
										UV_EMISSIVE( inPs.uv@value(uv_emissive).xy ),
										emissiveMapIdx ).xyz;
		@property( emissive_constant )
			emissiveCol *= material.emissive.xyz;
		@end
	@end
@end
@property( !emissive_map && emissive_constant )
	@piece( SampleEmissiveMap )
		float3 emissiveCol = material.emissive.xyz;
	@end
@end

@property( !hlms_render_depth_only && !hlms_shadowcaster && hlms_prepass )
	@undefpiece( DeclOutputType )
	@piece( DeclOutputType )
		struct PS_OUTPUT
		{
			float4 normals			[[ color(0) ]];
			float2 shadowRoughness	[[ color(1) ]];
		};
	@end
@end
