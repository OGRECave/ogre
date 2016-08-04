
// START UNIFORM DECLARATION
@property( !hlms_shadowcaster || alpha_test )
	@property( !hlms_shadowcaster )
		@insertpiece( PassDecl )
	@end
	@insertpiece( MaterialDecl )
	@insertpiece( InstanceDecl )
@end
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
struct PS_INPUT
{
@insertpiece( VStoPS_block )
};

@property( !hlms_shadowcaster )

@property( hlms_forward3d )
Buffer<uint> f3dGrid : register(t1);
Buffer<float4> f3dLightList : register(t2);@end

@property( !roughness_map )#define ROUGHNESS material.kS.w@end
@property( num_textures )Texture2DArray textureMaps[@value( num_textures )] : register(t@value(textureRegStart));@end
@property( envprobe_map )TextureCube	texEnvProbeMap : register(t@value(envMapReg));@end

@property( numSamplerStates || envprobe_map )SamplerState samplerStates[@value(numSamplerStates)] : register(s@value(samplerStateStart));@end

@property( normal_map )
@property( hlms_qtangent )
@piece( tbnApplyReflection ) * inPs.biNormalReflection@end
@end
@end

@property( hlms_num_shadow_maps )
Texture2D texShadowMap[@value(hlms_num_shadow_maps)] : register(t@value(textureRegShadowMapStart));
SamplerComparisonState shadowSampler : register(s@value(textureRegShadowMapStart));

float getShadow( Texture2D shadowMap, float4 psPosLN, float4 invShadowMapSize )
{
	float fDepth = psPosLN.z;
	float2 uv = psPosLN.xy / psPosLN.w;
	/*float c = shadowMap.SampleCmpLevelZero( shadowSampler, uv.xy, fDepth );
	return c;*/
	
	float retVal = 0;

@property( pcf_3x3 || pcf_4x4 )
	float2 offsets[@value(pcf_iterations)] =
	{
	@property( pcf_3x3 )
		float2( 0, 0 ),	//0, 0
		float2( 1, 0 ),	//1, 0
		float2( 0, 1 ),	//1, 1
		float2( 0, 0 ) 	//1, 1
	@end
	@property( pcf_4x4 )
		float2( 0, 0 ),	//0, 0
		float2( 1, 0 ),	//1, 0
		float2( 1, 0 ),	//2, 0

		float2(-2, 1 ),	//0, 1
		float2( 1, 0 ),	//1, 1
		float2( 1, 0 ),	//2, 1

		float2(-2, 1 ),	//0, 2
		float2( 1, 0 ),	//1, 2
		float2( 1, 0 )	//2, 2
	@end
	};
@end

	@foreach( pcf_iterations, n )
		@property( pcf_3x3 || pcf_4x4 )uv += offsets[@n] * invShadowMapSize.xy;@end
		// 2x2 PCF
		retVal += shadowMap.SampleCmpLevelZero( shadowSampler, uv.xy, fDepth );
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
float3 qmul( float4 q, float3 v )
{
	return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map_tex )float3 getTSNormal( float3 uv )
{
	float3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = textureMaps[@value( normal_map_tex_idx )].Sample( samplerStates[@value( normal_map_tex_idx )], uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = textureMaps[@value( normal_map_tex_idx )].Sample( samplerStates[@value( normal_map_tex_idx )], uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( max( 0, 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}
@end
@property( normal_weight_tex )#define normalMapWeight asfloat( material.indices4_7.w )@end
@property( detail_maps_normal )float3 getTSDetailNormal( SamplerState samplerState, Texture2DArray normalMap, float3 uv )
{
	float3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = normalMap.Sample( samplerState, uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = normalMap.Sample( samplerState, uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( max( 0, 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}
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
@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@value(CurrentShadowMap), passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize )@end @end

@insertpiece( output_type ) main( PS_INPUT inPs
@property( hlms_vpos ), float4 gl_FragCoord : SV_Position@end ) @insertpiece( output_type_sv )
{
	@insertpiece( custom_ps_preExecution )

	Material material;
	float4 outColour;
	
@property( diffuse_map )	uint diffuseIdx;@end
@property( normal_map_tex )	uint normalIdx;@end
@property( specular_map )	uint specularIdx;@end
@property( roughness_map )	uint roughnessIdx;@end
@property( detail_weight_map )	uint weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )uint detailMapIdx@n;@end @end
@foreach( 4, n )
	@property( detail_map_nm@n )uint detailNormMapIdx@n;@end @end
@property( envprobe_map )	uint envMapIdx;@end

@property( diffuse_map || detail_maps_diffuse )float4 diffuseCol;@end
@property( specular_map && !metallic_workflow && !fresnel_workflow )float3 specularCol;@end
@property( metallic_workflow || (specular_map && fresnel_workflow) )@insertpiece( FresnelType ) F0;@end
@property( roughness_map )float ROUGHNESS;@end
	
@property( hlms_normal || hlms_qtangent )	float3 nNormal;@end
	
@property( hlms_normal || hlms_qtangent )
	@property( !lower_gpu_overhead )
		uint materialId	= worldMaterialIdx[inPs.drawId].x & 0x1FFu;
		material = materialArray[materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray[0];
	@end
@property( diffuse_map )	diffuseIdx			= material.indices0_3.x & 0x0000FFFFu;@end
@property( normal_map_tex )	normalIdx			= material.indices0_3.x >> 16u;@end
@property( specular_map )	specularIdx			= material.indices0_3.y & 0x0000FFFFu;@end
@property( roughness_map )	roughnessIdx		= material.indices0_3.y >> 16u;@end
@property( detail_weight_map )	weightMapIdx		= material.indices0_3.z & 0x0000FFFFu;@end
@property( detail_map0 )	detailMapIdx0		= material.indices0_3.z >> 16u;@end
@property( detail_map1 )	detailMapIdx1		= material.indices0_3.w & 0x0000FFFFu;@end
@property( detail_map2 )	detailMapIdx2		= material.indices0_3.w >> 16u;@end
@property( detail_map3 )	detailMapIdx3		= material.indices4_7.x & 0x0000FFFFu;@end
@property( detail_map_nm0 )	detailNormMapIdx0	= material.indices4_7.x >> 16u;@end
@property( detail_map_nm1 )	detailNormMapIdx1	= material.indices4_7.y & 0x0000FFFFu;@end
@property( detail_map_nm2 )	detailNormMapIdx2	= material.indices4_7.y >> 16u;@end
@property( detail_map_nm3 )	detailNormMapIdx3	= material.indices4_7.z & 0x0000FFFFu;@end
@property( envprobe_map )	envMapIdx			= material.indices4_7.z >> 16u;@end

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
	float4 detailCol@n	= textureMaps[@value(detail_map@n_idx)].Sample( samplerStates[@value(detail_map@n_idx)], float3( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ) );
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
	nNormal = normalize( inPs.normal );
@end @property( normal_map )
	//Normal mapping.
	float3 geomNormal = normalize( inPs.normal );
	float3 vTangent = normalize( inPs.tangent );

	//Get the TBN matrix
	float3 vBinormal	= normalize( cross( geomNormal, vTangent )@insertpiece( tbnApplyReflection ) );
	float3x3 TBN		= float3x3( vTangent, vBinormal, geomNormal );

	@property( normal_map_tex )nNormal = getTSNormal( float3( inPs.uv@value(uv_normal).xy, normalIdx ) );@end
	@property( normal_weight_tex )
		// Apply the weight to the main normal map
		nNormal = lerp( float3( 0.0, 0.0, 1.0 ), nNormal, normalMapWeight );
	@end
@end

@property( hlms_pssm_splits )
    float fShadow = 1.0;
    if( inPs.depth <= passBuf.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@foreach( hlms_pssm_splits, n, 1 )	else if( inPs.depth <= passBuf.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@n, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end @end @property( !hlms_pssm_splits && hlms_num_shadow_maps && hlms_lights_directional )
    float fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
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
	nNormal = normalize( mul( nNormal, TBN ) );
@end

	//Everything's in Camera space
@property( hlms_lights_spot || ambient_hemisphere || envprobe_map || hlms_forward3d )
	float3 viewDir	= normalize( -inPs.pos );
	float NdotV		= saturate( dot( nNormal, viewDir ) );@end

@property( !ambient_fixed )
	float3 finalColour = float3(0, 0, 0);
@end @property( ambient_fixed )
	float3 finalColour = passBuf.ambientUpperHemi.xyz * @insertpiece( kD ).xyz;
@end

	@insertpiece( custom_ps_preLights )

@property( !custom_disable_directional_lights )
@property( hlms_lights_directional )
	finalColour += BRDF( passBuf.lights[0].position, viewDir, NdotV, passBuf.lights[0].diffuse, passBuf.lights[0].specular, material, nNormal @insertpiece( brdfExtraParams ) ) @insertpiece( DarkenWithShadowFirstLight );
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += BRDF( passBuf.lights[@n].position, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	finalColour += BRDF( passBuf.lights[@n].position, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) );@end
@end

@property( hlms_lights_point || hlms_lights_spot )	float3 lightDir;
	float fDistance;
	float3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional_non_caster )
	lightDir = passBuf.lights[@n].position - inPs.pos;
	fDistance= length( lightDir );
	if( fDistance <= passBuf.lights[@n].attenuation.x )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * atten;
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
	lightDir = passBuf.lights[@n].position - inPs.pos;
	fDistance= length( lightDir );
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - passBuf.lights[@n].position ), passBuf.lights[@n].spotDirection );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - passBuf.lights[@n].position ), zAxis( passBuf.lights[@n].spotQuaternion ) );@end
	if( fDistance <= passBuf.lights[@n].attenuation.x && spotCosAngle >= passBuf.lights[@n].spotParams.y )
	{
		lightDir *= 1.0 / fDistance;
	@property( hlms_lights_spot_textured )
		float3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], inPs.pos );
		float spotAtten = texture( texSpotLight, normalize( posInLightSpace ).xy ).x; //TODO
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = saturate( (spotCosAngle - passBuf.lights[@n].spotParams.y) * passBuf.lights[@n].spotParams.x );
		spotAtten = pow( spotAtten, passBuf.lights[@n].spotParams.z );
	@end
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )

@property( envprobe_map || ambient_hemisphere )
	float3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;
	
	@property( envprobe_map )
		float3 envColourS = texEnvProbeMap.SampleLevel( samplerStates[@value(num_textures)], mul( reflDir, passBuf.invViewMatCubemap ), ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );
		float3 envColourD = texEnvProbeMap.SampleLevel( samplerStates[@value(num_textures)], mul( nNormal, passBuf.invViewMatCubemap ), 11.0 ).xyz @insertpiece( ApplyEnvMapScale );
		@property( !hw_gamma_read )	//Gamma to linear space
			envColourS = envColourS * envColourS;
			envColourD = envColourD * envColourD;
		@end
	@end
	@property( ambient_hemisphere )
		float ambientWD = dot( passBuf.ambientHemisphereDir.xyz, nNormal ) * 0.5 + 0.5;
		float ambientWS = dot( passBuf.ambientHemisphereDir.xyz, reflDir ) * 0.5 + 0.5;

		@property( envprobe_map )
			envColourS	+= lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			envColourD	+= lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end @property( !envprobe_map )
			float3 envColourS = lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			float3 envColourD = lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end
	@end

	@insertpiece( BRDF_EnvMap )
@end

@property( !hw_gamma_write )
	//Linear to Gamma space
	outColour.xyz	= sqrt( finalColour );
@end @property( hw_gamma_write )
	outColour.xyz	= finalColour;
@end

@property( hlms_alphablend )
	@property( use_texture_alpha )
		outColour.w		= material.F0.w * diffuseCol.w;
	@end @property( !use_texture_alpha )
		outColour.w		= material.F0.w;
	@end
@end @property( !hlms_alphablend )
	outColour.w		= 1.0;@end
	
@end @property( !hlms_normal && !hlms_qtangent )
	outColour = float4( 1.0, 1.0, 1.0, 1.0 );
@end

	@insertpiece( custom_ps_posExecution )

@property( !hlms_render_depth_only )
	return outColour;
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
@property( diffuse_map )	uint diffuseIdx;@end
@property( detail_weight_map )	uint weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )uint detailMapIdx@n;@end @end

@property( diffuse_map || detail_maps_diffuse )float diffuseCol;@end
	@property( !lower_gpu_overhead )
		uint materialId	= worldMaterialIdx[inPs.drawId].x & 0x1FFu;
		material = materialArray[materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray[0];
	@end
@property( diffuse_map )	diffuseIdx			= material.indices0_3.x & 0x0000FFFFu;@end
@property( detail_weight_map )	weightMapIdx		= material.indices0_3.z & 0x0000FFFFu;@end
@property( detail_map0 )	detailMapIdx0		= material.indices0_3.z >> 16u;@end
@property( detail_map1 )	detailMapIdx1		= material.indices0_3.w & 0x0000FFFFu;@end
@property( detail_map2 )	detailMapIdx2		= material.indices0_3.w >> 16u;@end
@property( detail_map3 )	detailMapIdx3		= material.indices4_7.x & 0x0000FFFFu;@end

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
	float detailCol@n	= textureMaps[@value(detail_map@n_idx)].Sample( samplerStates[@value(detail_map@n_idx)], float3( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ) ).w;
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

