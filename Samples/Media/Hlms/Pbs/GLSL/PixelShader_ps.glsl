@insertpiece( SetCrossPlatformSettings )
@property( !GL430 )
@property( hlms_tex_gather )#extension GL_ARB_texture_gather: require@end
@end
@property( hlms_amd_trinary_minmax )#extension GL_AMD_shader_trinary_minmax: require@end

layout(std140) uniform;
#define FRAG_COLOR		0
@property( !hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
@end @property( hlms_shadowcaster && !hlms_shadow_uses_depth_texture )
layout(location = FRAG_COLOR, index = 0) out float outColour;
@end

@property( hlms_vpos )
in vec4 gl_FragCoord;
@end

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
in block
{
@insertpiece( VStoPS_block )
} inPs;

@property( !hlms_shadowcaster )

@property( hlms_forward3d )
/*layout(binding = 1) */uniform usamplerBuffer f3dGrid;
/*layout(binding = 2) */uniform samplerBuffer f3dLightList;@end
@property( !roughness_map )#define ROUGHNESS material.kS.w@end
@property( num_textures )uniform sampler2DArray textureMaps[@value( num_textures )];@end
@property( envprobe_map )uniform samplerCube	texEnvProbeMap;@end

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

@property( diffuse_map || detail_maps_diffuse )vec4 diffuseCol;@end
@property( specular_map && !metallic_workflow && !fresnel_workflow )vec3 specularCol;@end
@property( metallic_workflow || (specular_map && fresnel_workflow) )@insertpiece( FresnelType ) F0;@end
@property( roughness_map )float ROUGHNESS;@end

Material material;
@property( hlms_normal || hlms_qtangent )vec3 nNormal;@end

@property( normal_map )
@property( hlms_qtangent )
@piece( tbnApplyReflection ) * inPs.biNormalReflection@end
@end
@end

@property( hlms_num_shadow_maps )
@property( hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2DShadow@end @end
@property( !hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2D@end @end
uniform @insertpiece( SAMPLER2DSHADOW ) texShadowMap[@value(hlms_num_shadow_maps)];

float getShadow( @insertpiece( SAMPLER2DSHADOW ) shadowMap, vec4 psPosLN, vec4 invShadowMapSize )
{
	float fDepth = psPosLN.z;
	vec2 uv = psPosLN.xy / psPosLN.w;

	float retVal = 0;

@property( pcf_3x3 || pcf_4x4 )
	vec2 offsets[@value(pcf_iterations)] =
	{
	@property( pcf_3x3 )
		vec2( 0, 0 ),	//0, 0
		vec2( 1, 0 ),	//1, 0
		vec2( 0, 1 ),	//1, 1
		vec2( 0, 0 ) 	//1, 1
	@end
	@property( pcf_4x4 )
		vec2( 0, 0 ),	//0, 0
		vec2( 1, 0 ),	//1, 0
		vec2( 1, 0 ),	//2, 0

		vec2(-2, 1 ),	//0, 1
		vec2( 1, 0 ),	//1, 1
		vec2( 1, 0 ),	//2, 1

		vec2(-2, 1 ),	//0, 2
		vec2( 1, 0 ),	//1, 2
		vec2( 1, 0 )	//2, 2
	@end
	};

	float row[2];
	row[0] = 0;
	row[1] = 0;
@end

	vec2 fW;
	vec4 c;

	@foreach( pcf_iterations, n )
		@property( pcf_3x3 || pcf_4x4 )uv += offsets[@n] * invShadowMapSize.xy;@end

		@property( !hlms_shadow_uses_depth_texture )
		// 2x2 PCF
		//The 0.00196 is a magic number that prevents floating point
		//precision problems ("1000" becomes "999.999" causing fW to
		//be 0.999 instead of 0, hence ugly pixel-sized dot artifacts
		//appear at the edge of the shadow).
		fW = fract( uv * invShadowMapSize.zw + 0.00196 );

		@property( !hlms_tex_gather )
			c.w = texture(shadowMap, uv ).r;
			c.z = texture(shadowMap, uv + vec2( invShadowMapSize.x, 0.0 ) ).r;
			c.x = texture(shadowMap, uv + vec2( 0.0, invShadowMapSize.y ) ).r;
			c.y = texture(shadowMap, uv + vec2( invShadowMapSize.x, invShadowMapSize.y ) ).r;
		@end @property( hlms_tex_gather )
			c = textureGather( shadowMap, uv + invShadowMapSize.xy * 0.5 );
		@end

		c = step( fDepth, c );

		@property( !pcf_3x3 && !pcf_4x4 )
			//2x2 PCF: It's slightly faster to calculate this directly.
			retVal += mix(
						mix( c.w, c.z, fW.x ),
						mix( c.x, c.y, fW.x ),
						fW.y );
		@end @property( pcf_3x3 || pcf_4x4 )
			row[0] += mix( c.w, c.z, fW.x );
			row[1] += mix( c.x, c.y, fW.x );
		@end
		@end @property( hlms_shadow_uses_depth_texture )
			retVal += texture( shadowMap, vec3( uv, fDepth ) ).r;
		@end
	@end

	@property( (pcf_3x3 || pcf_4x4) && !hlms_shadow_uses_depth_texture )
		//NxN PCF: It's much faster to leave the final mix out of the loop (when N > 2).
		retVal = mix( row[0], row[1], fW.y );
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
vec3 qmul( vec4 q, vec3 v )
{
	return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map_tex )vec3 getTSNormal( vec3 uv )
{
	vec3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = texture( textureMaps[@value( normal_map_tex_idx )], uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = texture( textureMaps[@value( normal_map_tex_idx )], uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( max( 0, 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}
@end
@property( normal_weight_tex )#define normalMapWeight uintBitsToFloat( material.indices4_7.w )@end
@property( detail_maps_normal )vec3 getTSDetailNormal( sampler2DArray normalMap, vec3 uv )
{
	vec3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = texture( normalMap, uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = texture( normalMap, uv ).xw * 2.0 - 1.0;
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
@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@value(CurrentShadowMap), pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize )@end @end

void main()
{
    @insertpiece( custom_ps_preExecution )
@property( hlms_normal || hlms_qtangent )
	uint materialId	= instance.worldMaterialIdx[inPs.drawId].x & 0x1FFu;
	material = materialArray.m[materialId];
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
		vec4 detailWeights = @insertpiece( SamplerDetailWeightMap );
		@property( detail_weights )detailWeights *= material.cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )vec4 detailWeights = material.cDetailWeights;@end
		@property( !detail_weights )vec4 detailWeights = vec4( 1.0, 1.0, 1.0, 1.0 );@end
	@end
@end

	/// Sample detail maps and weight them against the weight map in the next foreach loop.
@foreach( detail_maps_diffuse, n )@property( detail_map@n )
	vec4 detailCol@n	= texture( textureMaps[@value(detail_map@n_idx)], vec3( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ) );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol@n.xyz = detailCol@n.xyz * detailCol@n.xyz;@end
	detailWeights.@insertpiece(detail_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_swizzle@n);@end
@end

@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value. If there are no diffuse or detail maps,
	/// we must not access diffuseCol at all, but rather use material.kD directly (see piece( kD ) ).
	@property( !diffuse_map && detail_maps_diffuse )diffuseCol = vec4( 0.0, 0.0, 0.0, 0.0 );@end

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
	vec3 geomNormal = normalize( inPs.normal );
	vec3 vTangent = normalize( inPs.tangent );

	//Get the TBN matrix
	vec3 vBinormal	= normalize( cross( geomNormal, vTangent )@insertpiece( tbnApplyReflection ) );
	mat3 TBN		= mat3( vTangent, vBinormal, geomNormal );

	@property( normal_map_tex )nNormal = getTSNormal( vec3( inPs.uv@value(uv_normal).xy, normalIdx ) );@end
	@property( normal_weight_tex )
		// Apply the weight to the main normal map
		nNormal = mix( vec3( 0.0, 0.0, 1.0 ), nNormal, normalMapWeight );
	@end
@end

@property( hlms_pssm_splits )
    float fShadow = 1.0;
    if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@foreach( hlms_pssm_splits, n, 1 )	else if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@n, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end @end @property( !hlms_pssm_splits && hlms_num_shadow_maps && hlms_lights_directional )
    float fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
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
	vec3 vDetail = @insertpiece( SampleDetailMapNm@n );
	nNormal.xy	@insertpiece( detail_nm_op_sum ) vDetail.xy;
	nNormal.z	@insertpiece( detail_nm_op_mul ) vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul );@end
@foreach( detail_maps_normal, n, second_valid_detail_map_nm )@property( detail_map_nm@n )
	vDetail = @insertpiece( SampleDetailMapNm@n );
	nNormal.xy	+= vDetail.xy;
	nNormal.z	*= vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n) @insertpiece( detail@n_nm_weight_mul );@end @end

@property( normal_map )
	nNormal = normalize( TBN * nNormal );
@end

	//Everything's in Camera space
@property( hlms_lights_spot || ambient_hemisphere || envprobe_map || hlms_forward3d )
	vec3 viewDir	= normalize( -inPs.pos );
	float NdotV		= clamp( dot( nNormal, viewDir ), 0.0, 1.0 );@end

@property( !ambient_fixed )
	vec3 finalColour = vec3(0);
@end @property( ambient_fixed )
	vec3 finalColour = pass.ambientUpperHemi.xyz * @insertpiece( kD ).xyz;
@end

	@insertpiece( custom_ps_preLights )

@property( !custom_disable_directional_lights )
@property( hlms_lights_directional )
	finalColour += BRDF( pass.lights[0].position, viewDir, NdotV, pass.lights[0].diffuse, pass.lights[0].specular ) @insertpiece(DarkenWithShadowFirstLight);
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += BRDF( pass.lights[@n].position, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	finalColour += BRDF( pass.lights[@n].position, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular );@end
@end

@property( hlms_lights_point || hlms_lights_spot )	vec3 lightDir;
	float fDistance;
	vec3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional_non_caster )
	lightDir = pass.lights[@n].position - inPs.pos;
	fDistance= length( lightDir );
	if( fDistance <= pass.lights[@n].attenuation.x )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = BRDF( lightDir, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular )@insertpiece( DarkenWithShadow );
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
		vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], inPs.pos );
		float spotAtten = texture( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = clamp( (spotCosAngle - pass.lights[@n].spotParams.y) * pass.lights[@n].spotParams.x, 0.0, 1.0 );
		spotAtten = pow( spotAtten, pass.lights[@n].spotParams.z );
	@end
		tmpColour = BRDF( lightDir, viewDir, NdotV, pass.lights[@n].diffuse, pass.lights[@n].specular )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (pass.lights[@n].attenuation.y + pass.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )

@property( envprobe_map || ambient_hemisphere )
	vec3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;

	@property( envprobe_map )
		vec3 envColourS = textureLod( texEnvProbeMap, reflDir * pass.invViewMatCubemap, ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
		vec3 envColourD = textureLod( texEnvProbeMap, nNormal * pass.invViewMatCubemap, 11.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
		@property( !hw_gamma_read )	//Gamma to linear space
			envColourS = envColourS * envColourS;
			envColourD = envColourD * envColourD;
		@end
	@end
	@property( ambient_hemisphere )
		float ambientWD = dot( pass.ambientHemisphereDir.xyz, nNormal ) * 0.5 + 0.5;
		float ambientWS = dot( pass.ambientHemisphereDir.xyz, reflDir ) * 0.5 + 0.5;

		@property( envprobe_map )
			envColourS	+= mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWD );
			envColourD	+= mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWS );
		@end @property( !envprobe_map )
			vec3 envColourS = mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWD );
			vec3 envColourD = mix( pass.ambientLowerHemi.xyz, pass.ambientUpperHemi.xyz, ambientWS );
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
	outColour = vec4( 1.0, 1.0, 1.0, 1.0 );
@end

	@insertpiece( custom_ps_posExecution )
}
@end
@property( hlms_shadowcaster )
	@property( hlms_shadow_uses_depth_texture && !alpha_test )
		@set( hlms_disable_stage, 1 )
	@end

@property( alpha_test )
	Material material;
	@property( diffuse_map || detail_maps_diffuse )float diffuseCol;@end
	@property( num_textures )uniform sampler2DArray textureMaps[@value( num_textures )];@end
	@property( diffuse_map )uint diffuseIdx;@end
	@property( detail_weight_map )uint weightMapIdx;@end
	@foreach( 4, n )
		@property( detail_map@n )uint detailMapIdx@n;@end @end
@end

void main()
{
	@insertpiece( custom_ps_preExecution )

@property( alpha_test )
	uint materialId	= instance.worldMaterialIdx[inPs.drawId].x & 0x1FFu;
	material = materialArray.m[materialId];
@property( diffuse_map )	diffuseIdx			= material.indices0_3.x & 0x0000FFFFu;@end
@property( detail_weight_map )	weightMapIdx		= material.indices0_3.z & 0x0000FFFFu;@end
@property( detail_map0 )	detailMapIdx0		= material.indices0_3.z >> 16u;@end
@property( detail_map1 )	detailMapIdx1		= material.indices0_3.w & 0x0000FFFFu;@end
@property( detail_map2 )	detailMapIdx2		= material.indices0_3.w >> 16u;@end
@property( detail_map3 )	detailMapIdx3		= material.indices4_7.x & 0x0000FFFFu;@end

@property( detail_maps_diffuse || detail_maps_normal )
	//Prepare weight map for the detail maps.
	@property( detail_weight_map )
		vec4 detailWeights = @insertpiece( SamplerDetailWeightMap );
		@property( detail_weights )detailWeights *= material.cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )vec4 detailWeights = material.cDetailWeights;@end
		@property( !detail_weights )vec4 detailWeights = vec4( 1.0, 1.0, 1.0, 1.0 );@end
	@end
@end

	/// Sample detail maps and weight them against the weight map in the next foreach loop.
@foreach( detail_maps_diffuse, n )@property( detail_map@n )
	float detailCol@n	= texture( textureMaps[@value(detail_map@n_idx)], vec3( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ) ).w;
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

	/// Apply the material's alpha over the textures
@property( TODO_REFACTOR_ACCOUNT_MATERIAL_ALPHA )	diffuseCol.xyz *= material.kD.xyz;@end

	@property( diffuse_map || detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol )
		discard;
	@end @property( !diffuse_map && !detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) 1.0 )
		discard;
	@end
@end /// !alpha_test

@property( !hlms_shadow_uses_depth_texture )
	@property( GL3+ )outColour = inPs.depth;@end
	@property( !GL3+ )gl_FragColor.x = inPs.depth;@end
@end

	@insertpiece( custom_ps_posExecution )
}
@end
