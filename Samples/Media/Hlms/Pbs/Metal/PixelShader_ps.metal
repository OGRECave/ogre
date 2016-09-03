@insertpiece( SetCrossPlatformSettings )

// START UNIFORM STRUCT DECLARATION
@property( !hlms_shadowcaster || alpha_test )
	@property( !hlms_shadowcaster )
		@insertpiece( PassStructDecl )
	@end
	@insertpiece( MaterialStructDecl )
@end
@insertpiece( custom_ps_uniformStructDeclaration )
// END UNIFORM STRUCT DECLARATION

struct PS_INPUT
{
@insertpiece( VStoPS_block )
@property( hlms_vpos )	float4 gl_FragCoord [[position]];@end
};

@insertpiece( PsOutputDecl )

@property( !hlms_shadowcaster )

@property( !roughness_map )#define ROUGHNESS material.kS.w@end

@property( normal_map )
@property( hlms_qtangent )
@piece( tbnApplyReflection ) * inPs.biNormalReflection@end
@end
@end

@property( hlms_num_shadow_maps )
inline float getShadow( depth2d<float> shadowMap, sampler shadowSampler,
						float4 psPosLN, float4 invShadowMapSize )
{
	float fDepth = psPosLN.z;
	half2 uv = half2( psPosLN.xy / psPosLN.w );
	/*float c = shadowMap.SampleCmpLevelZero( shadowSampler, uv.xy, fDepth );
	return c;*/

	float retVal = 0;

@property( pcf_3x3 || pcf_4x4 )
	half2 offsets[@value(pcf_iterations)] =
	{
	@property( pcf_3x3 )
		half2( 0.0h, 0.0h ),	//0, 0
		half2( 1.0h, 0.0h ),	//1, 0
		half2( 0.0h, 1.0h ),	//1, 1
		half2( 0.0h, 0.0h ) 	//1, 1
	@end
	@property( pcf_4x4 )
		half2( 0.0h, 0.0h ),	//0, 0
		half2( 1.0h, 0.0h ),	//1, 0
		half2( 1.0h, 0.0h ),	//2, 0

		half2(-2.0h, 1.0h ),	//0, 1
		half2( 1.0h, 0.0h ),	//1, 1
		half2( 1.0h, 0.0h ),	//2, 1

		half2(-2.0h, 1.0h ),	//0, 2
		half2( 1.0h, 0.0h ),	//1, 2
		half2( 1.0h, 0.0h )		//2, 2
	@end
	};
@end

	@foreach( pcf_iterations, n )
		@property( pcf_3x3 || pcf_4x4 )uv += offsets[@n] * half2(invShadowMapSize.xy);@end
		// 2x2 PCF
		retVal += shadowMap.sample_compare( shadowSampler, float2(uv.xy), fDepth );
	@end

	@property( pcf_3x3 )
		retVal *= 0.25;
	@end @property( pcf_4x4 )
		retVal *= 0.11111111111111;
	@end

	return retVal;
}
@end

@property( hlms_lights_spot_textured )@insertpiece( DeclQuat_zAxis )
inline float3 qmul( float4 q, float3 v )
{
	return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map_tex || detail_maps_normal )
inline float3 getTSNormal( sampler samplerState, texture2d_array<float> normalMap, float2 uv, ushort normalIdx )
{
	float3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = normalMap.sample( samplerState, uv, normalIdx ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = normalMap.sample( samplerState, uv, normalIdx ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( max( 0.0f, 1.0f - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}
@end
@property( normal_weight_tex )#define normalMapWeight material.mNormalMapWeight )@end
@property( detail_maps_normal )
	@foreach( 4, n )
		@property( normal_weight_detail@n )
			@piece( detail@n_nm_weight_mul ) * material.normalWeights.@insertpiece( detail_swizzle@n )@end
		@end
	@end
@end

@property( hlms_normal || hlms_qtangent )
@insertpiece( DeclareBRDF )
@end

@property( hlms_num_shadow_maps )@piece( DarkenWithShadowFirstLight )* fShadow@end @end
@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap@value(CurrentShadowMap), shadowSampler, inPs.posL@value(CurrentShadowMap), pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize )@end @end

constexpr sampler shadowSampler = sampler( coord::normalized,
										   address::clamp_to_edge,
										   filter::linear,
										   compare_func::less_equal );

fragment @insertpiece( output_type ) main_metal
(
	PS_INPUT inPs [[stage_in]]
	// START UNIFORM DECLARATION
	@property( !hlms_shadowcaster || alpha_test )
		@property( !hlms_shadowcaster )
			@insertpiece( PassDecl )
		@end
		@insertpiece( MaterialDecl )
	@end
	@insertpiece( custom_ps_uniformDeclaration )
	// END UNIFORM DECLARATION
	@property( hlms_forward3d )
		, device const ushort *f3dGrid [[buffer(TEX_SLOT_START+1)]]
		, device const float4 *f3dLightList [[buffer(TEX_SLOT_START+2)]]
	@end

	@property( two_sided_lighting )
		, bool gl_FrontFacing [[front_facing]]
		@piece( two_sided_flip_normal )* (gl_FrontFacing ? 1.0 : -1.0)@end
	@end

	@foreach( num_textures, n )
		, texture2d_array<float> textureMaps@n [[texture(@counter(textureRegStart))]]@end
	@property( use_envprobe_map )
		, texturecube<float>	texEnvProbeMap [[texture(@value(envMapReg))]]@end
	@foreach( numSamplerStates, n )
		, sampler samplerStates@n [[sampler(@counter(samplerStateStart))]]@end
	@foreach( hlms_num_shadow_maps, n )
		, depth2d<float> texShadowMap@n [[texture(@counter(textureRegShadowMapStart))]]@end
)
{
	PS_OUTPUT outPs;

	@insertpiece( custom_ps_preExecution )

	Material material;

@property( diffuse_map )	ushort diffuseIdx;@end
@property( normal_map_tex )	ushort normalIdx;@end
@property( specular_map )	ushort specularIdx;@end
@property( roughness_map )	ushort roughnessIdx;@end
@property( detail_weight_map )	ushort weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )ushort detailMapIdx@n;@end @end
@foreach( 4, n )
	@property( detail_map_nm@n )ushort detailNormMapIdx@n;@end @end
@property( use_envprobe_map )	ushort envMapIdx;@end

@property( diffuse_map || detail_maps_diffuse )float4 diffuseCol;@end
@property( specular_map && !metallic_workflow && !fresnel_workflow )float3 specularCol;@end
@property( metallic_workflow || (specular_map && fresnel_workflow) )@insertpiece( FresnelType ) F0;@end
@property( roughness_map )float ROUGHNESS;@end

@property( hlms_normal || hlms_qtangent )	float3 nNormal;@end

@property( hlms_normal || hlms_qtangent )
	@property( !lower_gpu_overhead )
		material = materialArray[inPs.materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray[0];
	@end
@property( diffuse_map )	diffuseIdx			= material.diffuseIdx;@end
@property( normal_map_tex )	normalIdx			= material.normalIdx;@end
@property( specular_map )	specularIdx			= material.specularIdx;@end
@property( roughness_map )	roughnessIdx		= material.roughnessIdx;@end
@property( detail_weight_map )	weightMapIdx	= material.weightMapIdx;@end
@property( detail_map0 )	detailMapIdx0		= material.detailMapIdx0;@end
@property( detail_map1 )	detailMapIdx1		= material.detailMapIdx1;@end
@property( detail_map2 )	detailMapIdx2		= material.detailMapIdx2;@end
@property( detail_map3 )	detailMapIdx3		= material.detailMapIdx3;@end
@property( detail_map_nm0 )	detailNormMapIdx0	= material.detailNormMapIdx0;@end
@property( detail_map_nm1 )	detailNormMapIdx1	= material.detailNormMapIdx1;@end
@property( detail_map_nm2 )	detailNormMapIdx2	= material.detailNormMapIdx2;@end
@property( detail_map_nm3 )	detailNormMapIdx3	= material.detailNormMapIdx3;@end
@property( use_envprobe_map )	envMapIdx			= material.envMapIdx;@end

	@insertpiece( custom_ps_posMaterialLoad )

@property( detail_maps_diffuse || detail_maps_normal )
	//Prepare weight map for the detail maps.
	@property( detail_weight_map )
		float4 detailWeights = @insertpiece( SamplerDetailWeightMap );
		@property( detail_weights )detailWeights *= material.cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )float4 detailWeights = material.cDetailWeights;@end
		@property( !detail_weights )float4 detailWeights = float4( 1.0, 1.0, 1.0, 1.0 );@end
	@end
@end

	/// Sample detail maps and weight them against the weight map in the next foreach loop.
@foreach( detail_maps_diffuse, n )@property( detail_map@n )
	float4 detailCol@n	= textureMaps@value(detail_map@n_idx).sample( samplerStates@value(detail_map@n_idx), inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol@n.xyz = detailCol@n.xyz * detailCol@n.xyz;@end
	detailWeights.@insertpiece(detail_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_swizzle@n);@end
@end

@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value. If there are no diffuse or detail maps,
	/// we must not access diffuseCol at all, but rather use material.kD directly (see piece( kD ) ).
	@property( !diffuse_map && detail_maps_diffuse )diffuseCol = float4( 0.0, 0.0, 0.0, 0.0 );@end

	/// Blend the detail diffuse maps with the main diffuse.
@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @add( t, 1 ) @end

	/// Apply the material's diffuse over the textures
@property( diffuse_map || detail_maps_diffuse )
	@property( !transparent_mode )
		diffuseCol.xyz *= material.kD.xyz;
	@end @property( transparent_mode )
		diffuseCol.xyz *= material.kD.xyz * diffuseCol.w * diffuseCol.w;
	@end
@end

@property( alpha_test )
	@property( diffuse_map || detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol.a )
		discard;
	@end @property( !diffuse_map && !detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) 1.0 )
		discard;
	@end
@end

@property( !normal_map )
	// Geometric normal
	nNormal = normalize( inPs.normal ) @insertpiece( two_sided_flip_normal );
@end @property( normal_map )
	//Normal mapping.
	float3 geomNormal = normalize( inPs.normal ) @insertpiece( two_sided_flip_normal );
	float3 vTangent = normalize( inPs.tangent );

	//Get the TBN matrix
	float3 vBinormal	= normalize( cross( geomNormal, vTangent )@insertpiece( tbnApplyReflection ) );
	float3x3 TBN		= float3x3( vTangent, vBinormal, geomNormal );

	@property( normal_map_tex )nNormal = getTSNormal( samplerStates@value( normal_map_tex_idx ),
													  textureMaps@value( normal_map_tex_idx ),
													  inPs.uv@value(uv_normal).xy, normalIdx );@end
	@property( normal_weight_tex )
		// Apply the weight to the main normal map
		nNormal = mix( float3( 0.0, 0.0, 1.0 ), nNormal, normalMapWeight );
	@end
@end

@property( hlms_pssm_splits )
	float fShadow = 1.0;
	if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
		fShadow = getShadow( texShadowMap@value(CurrentShadowMap), shadowSampler, inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@foreach( hlms_pssm_splits, n, 1 )	else if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
		fShadow = getShadow( texShadowMap@value(CurrentShadowMap), shadowSampler, inPs.posL@n, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end @end @property( !hlms_pssm_splits && hlms_num_shadow_maps && hlms_lights_directional )
	float fShadow = getShadow( texShadowMap@value(CurrentShadowMap), shadowSampler, inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end

@insertpiece( SampleSpecularMap )
@insertpiece( SampleRoughnessMap )

	/// If there is no normal map, the first iteration must
	/// initialize nNormal instead of try to merge with it.
@property( normal_map_tex )
	@piece( detail_nm_op_sum )+=@end
	@piece( detail_nm_op_mul )*=@end
@end @property( !normal_map_tex )
	@piece( detail_nm_op_sum )=@end
	@piece( detail_nm_op_mul )=@end
@end

	/// Blend the detail normal maps with the main normal.
@foreach( second_valid_detail_map_nm, n, first_valid_detail_map_nm )
	float3 vDetail = @insertpiece( SampleDetailMapNm@n );
	nNormal.xy	@insertpiece( detail_nm_op_sum ) vDetail.xy;
	nNormal.z	@insertpiece( detail_nm_op_mul ) vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul );@end
@foreach( detail_maps_normal, n, second_valid_detail_map_nm )@property( detail_map_nm@n )
	vDetail = @insertpiece( SampleDetailMapNm@n );
	nNormal.xy	+= vDetail.xy;
	nNormal.z	*= vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul );@end @end

@property( normal_map )
	nNormal = normalize( nNormal * TBN );
@end

	//Everything's in Camera space
@property( hlms_lights_spot || ambient_hemisphere || use_envprobe_map || hlms_forward3d )
	float3 viewDir	= normalize( -inPs.pos );
	float NdotV		= saturate( dot( nNormal, viewDir ) );@end

@property( !ambient_fixed )
	float3 finalColour = float3(0, 0, 0);
@end @property( ambient_fixed )
	float3 finalColour = pass.ambientUpperHemi.xyz * @insertpiece( kD ).xyz;
@end

	@insertpiece( custom_ps_preLights )

@property( !custom_disable_directional_lights )
@property( hlms_lights_directional )
	finalColour += BRDF( pass.lights[0].position, viewDir, NdotV, pass.lights[0].diffuse, pass.lights[0].specular, material, nNormal @insertpiece( brdfExtraParams ) ) @insertpiece( DarkenWithShadowFirstLight );
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += BRDF( pass.lights[@n].position, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	finalColour += BRDF( pass.lights[@n].position, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) );@end
@end

@property( hlms_lights_point || hlms_lights_spot )	float3 lightDir;
	float fDistance;
	float3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional_non_caster )
	lightDir = pass.lights[@n].position - inPs.pos;
	fDistance= length( lightDir );
	if( fDistance <= pass.lights[@n].attenuation.x )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = BRDF( lightDir, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (pass.lights[@n].attenuation.y + pass.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * atten;
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
	lightDir = pass.lights[@n].position - inPs.pos;
	fDistance= length( lightDir );
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - pass.lights[@n].position ), pass.lights[@n].spotDirection );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - pass.lights[@n].position ), zAxis( pass.lights[@n].spotQuaternion ) );@end
	if( fDistance <= pass.lights[@n].attenuation.x && spotCosAngle >= pass.lights[@n].spotParams.y )
	{
		lightDir *= 1.0 / fDistance;
	@property( hlms_lights_spot_textured )
		float3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], inPs.pos );
		float spotAtten = texture( texSpotLight, normalize( posInLightSpace ).xy ).x; //TODO
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = saturate( (spotCosAngle - pass.lights[@n].spotParams.y) * pass.lights[@n].spotParams.x );
		spotAtten = pow( spotAtten, pass.lights[@n].spotParams.z );
	@end
		tmpColour = BRDF( lightDir, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (pass.lights[@n].attenuation.y + pass.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )

@property( use_envprobe_map || ambient_hemisphere )
	float3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;

	@property( use_envprobe_map )
		float3 envColourS = texEnvProbeMap.sample( samplerStates@value(num_textures), reflDir * pass.invViewMatCubemap, level( ROUGHNESS * 12.0 ) ).xyz @insertpiece( ApplyEnvMapScale );
		float3 envColourD = texEnvProbeMap.sample( samplerStates@value(num_textures), nNormal * pass.invViewMatCubemap, level( 11.0 ) ).xyz @insertpiece( ApplyEnvMapScale );
		@property( !hw_gamma_read )	//Gamma to linear space
			envColourS = envColourS * envColourS;
			envColourD = envColourD * envColourD;
		@end
	@end
	@property( ambient_hemisphere )
		float ambientWD = dot( pass.ambientHemisphereDir.xyz, nNormal ) * 0.5 + 0.5;
		float ambientWS = dot( pass.ambientHemisphereDir.xyz, reflDir ) * 0.5 + 0.5;

		@property( use_envprobe_map )
			envColourS	+= mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWD );
			envColourD	+= mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWS );
		@end @property( !use_envprobe_map )
			float3 envColourS = mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWD );
			float3 envColourD = mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWS );
		@end
	@end

	@insertpiece( BRDF_EnvMap )
@end

@property( !hw_gamma_write )
	//Linear to Gamma space
	outPs.colour0.xyz	= sqrt( finalColour );
@end @property( hw_gamma_write )
	outPs.colour0.xyz	= finalColour;
@end

@property( hlms_alphablend )
	@property( use_texture_alpha )
		outPs.colour0.w		= material.F0.w * diffuseCol.w;
	@end @property( !use_texture_alpha )
		outPs.colour0.w		= material.F0.w;
	@end
@end @property( !hlms_alphablend )
	outPs.colour0.w		= 1.0;@end

@end @property( !hlms_normal && !hlms_qtangent )
	outPs.colour0 = float4( 1.0, 1.0, 1.0, 1.0 );
@end

	@insertpiece( custom_ps_posExecution )

@property( !hlms_render_depth_only )
	return outPs;
@end
}
@end
@property( hlms_shadowcaster )
@property( num_textures )Texture2DArray textureMaps[@value( num_textures )] : register(t@value(textureRegStart));@end
@property( numSamplerStates )SamplerState samplerStates[@value(numSamplerStates)] : register(s@value(samplerStateStart));@end

@insertpiece( output_type ) main( PS_INPUT inPs ) @insertpiece( output_type_sv )
{
	@insertpiece( custom_ps_preExecution )

@property( alpha_test )
	Material material;
@property( diffuse_map )	ushort diffuseIdx;@end
@property( detail_weight_map )	ushort weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )ushort detailMapIdx@n;@end @end

@property( diffuse_map || detail_maps_diffuse )float diffuseCol;@end

	@property( !lower_gpu_overhead )
		material = materialArray[inPs.materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray[0];
	@end
@property( diffuse_map )	diffuseIdx			= material.diffuseIdx;@end
@property( detail_weight_map )	weightMapIdx		= material.weightMapIdx;@end
@property( detail_map0 )	detailMapIdx0		= material.detailMapIdx0;@end
@property( detail_map1 )	detailMapIdx1		= material.detailMapIdx1;@end
@property( detail_map2 )	detailMapIdx2		= material.detailMapIdx2;@end
@property( detail_map3 )	detailMapIdx3		= material.detailMapIdx3;@end

@property( detail_maps_diffuse || detail_maps_normal )
	//Prepare weight map for the detail maps.
	@property( detail_weight_map )
		float4 detailWeights = @insertpiece( SamplerDetailWeightMap );
		@property( detail_weights )detailWeights *= material.cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )float4 detailWeights = material.cDetailWeights;@end
		@property( !detail_weights )float4 detailWeights = float4( 1.0, 1.0, 1.0, 1.0 );@end
	@end
@end

	/// Sample detail maps and weight them against the weight map in the next foreach loop.
@foreach( detail_maps_diffuse, n )@property( detail_map@n )
	float detailCol@n	= textureMaps@value(detail_map@n_idx).sample( samplerStates@value(detail_map@n_idx), inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ).w;
	detailCol@n = detailWeights.@insertpiece(detail_swizzle@n) * detailCol@n;@end
@end

@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value. If there are no diffuse or detail maps,
	/// we must not access diffuseCol at all, but rather use material.kD directly (see piece( kD ) ).
	@property( !diffuse_map && detail_maps_diffuse )diffuseCol = 0.0;@end

	/// Blend the detail diffuse maps with the main diffuse.
@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @add( t, 1 ) @end

	/// Apply the material's diffuse over the textures
@property( TODO_REFACTOR_ACCOUNT_MATERIAL_ALPHA )	diffuseCol.xyz *= material.kD.xyz;@end

	@property( diffuse_map || detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol )
		discard;
	@end @property( !diffuse_map && !detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) 1.0 )
		discard;
	@end
@end /// !alpha_test

	@insertpiece( custom_ps_posExecution )

@property( !hlms_render_depth_only )
	return inPs.depth;
@end
}
@end

