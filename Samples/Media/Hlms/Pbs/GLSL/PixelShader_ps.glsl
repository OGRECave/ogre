@insertpiece( SetCrossPlatformSettings )
@property( !GL430 )
@property( hlms_tex_gather )#extension GL_ARB_texture_gather: require@end
@end
@property( hlms_amd_trinary_minmax )#extension GL_AMD_shader_trinary_minmax: require@end
@insertpiece( SetCompatibilityLayer )

layout(std140) uniform;
#define FRAG_COLOR		0
@property( !hlms_render_depth_only )
	@property( !hlms_shadowcaster )
		@property( !hlms_prepass )
			layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
		@end @property( hlms_prepass )
			layout(location = 0) out vec4 outNormals;
			layout(location = 1) out vec2 outShadowRoughness;
		@end
	@end @property( hlms_shadowcaster )
	layout(location = FRAG_COLOR, index = 0) out float outColour;
	@end
@end

@property( hlms_use_prepass )
	@property( !hlms_use_prepass_msaa )
		uniform sampler2D gBuf_normals;
		uniform sampler2D gBuf_shadowRoughness;
	@end @property( hlms_use_prepass_msaa )
		uniform sampler2DMS gBuf_normals;
		uniform sampler2DMS gBuf_shadowRoughness;
		uniform sampler2DMS gBuf_depthTexture;
	@end

	@property( hlms_use_ssr )
		uniform sampler2D ssrTexture;
	@end
@end

@insertpiece( DeclPlanarReflTextures )

@property( hlms_vpos )
in vec4 gl_FragCoord;
@end

@property( two_sided_lighting )
	@property( hlms_forwardplus_flipY )
		@piece( two_sided_flip_normal )* (gl_FrontFacing ? -1.0 : 1.0)@end
	@end @property( !hlms_forwardplus_flipY )
		@piece( two_sided_flip_normal )* (gl_FrontFacing ? 1.0 : -1.0)@end
	@end
@end

// START UNIFORM DECLARATION
@property( !hlms_shadowcaster || alpha_test )
	@property( !hlms_shadowcaster )
		@insertpiece( PassDecl )
	@end
	@insertpiece( MaterialDecl )
	@insertpiece( InstanceDecl )
	@insertpiece( PccManualProbeDecl )
@end
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
@property( !hlms_shadowcaster || !hlms_shadow_uses_depth_texture || alpha_test || exponential_shadow_maps )
in block
{
@insertpiece( VStoPS_block )
} inPs;
@end

@property( !hlms_shadowcaster )

@property( hlms_forwardplus )
/*layout(binding = 1) */uniform usamplerBuffer f3dGrid;
/*layout(binding = 2) */uniform samplerBuffer f3dLightList;
@end
@property( irradiance_volumes )
	uniform sampler3D irradianceVolume;
@end

@property( !roughness_map )#define ROUGHNESS material.kS.w@end
@property( num_textures )uniform sampler2DArray textureMaps[@value( num_textures )];@end
@property( use_envprobe_map )uniform samplerCube	texEnvProbeMap;@end

@property( diffuse_map )	uint diffuseIdx;@end
@property( normal_map_tex )	uint normalIdx;@end
@property( specular_map )	uint specularIdx;@end
@property( roughness_map )	uint roughnessIdx;@end
@property( detail_weight_map )	uint weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )uint detailMapIdx@n;@end @end
@foreach( 4, n )
	@property( detail_map_nm@n )uint detailNormMapIdx@n;@end @end
@property( use_envprobe_map )	uint envMapIdx;@end

vec4 diffuseCol;
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

@property( (hlms_normal || hlms_qtangent) && !hlms_prepass )
@insertpiece( DeclareBRDF )
@insertpiece( DeclareBRDF_InstantRadiosity )
@end

@property( use_parallax_correct_cubemaps )
@insertpiece( DeclParallaxLocalCorrect )
@end

@insertpiece( DeclShadowMapMacros )
@insertpiece( DeclShadowSamplers )
@insertpiece( DeclShadowSamplingFuncs )

void main()
{
    @insertpiece( custom_ps_preExecution )
@property( hlms_normal || hlms_qtangent )
	@property( !lower_gpu_overhead )
		uint materialId	= instance.worldMaterialIdx[inPs.drawId].x & 0x1FFu;
		material = materialArray.m[materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray.m[0];
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
@property( use_envprobe_map )	envMapIdx			= material.indices4_7.z >> 16u;@end

	@insertpiece( DeclareObjLightMask )

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

@property( !hlms_prepass || alpha_test )
	@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value. If there are no diffuse or detail maps,
	/// we must not access diffuseCol at all, but rather use material.kD directly (see piece( kD ) ).
	@property( !diffuse_map )diffuseCol = material.bgDiffuse;@end

	/// Blend the detail diffuse maps with the main diffuse.
	@foreach( detail_maps_diffuse, n )
		@insertpiece( blend_mode_idx@n ) @add( t, 1 ) @end

		/// Apply the material's diffuse over the textures
		@property( !transparent_mode )
			diffuseCol.xyz *= material.kD.xyz;
		@end @property( transparent_mode )
			diffuseCol.xyz *= material.kD.xyz * diffuseCol.w * diffuseCol.w;
		@end

	@property( alpha_test )
		if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol.a )
			discard;
	@end
@end

@property( !hlms_use_prepass )
	@property( !normal_map )
		// Geometric normal
		nNormal = normalize( inPs.normal ) @insertpiece( two_sided_flip_normal );
	@end @property( normal_map )
		//Normal mapping.
		vec3 geomNormal = normalize( inPs.normal ) @insertpiece( two_sided_flip_normal );
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

	@insertpiece( DoDirectionalShadowMaps )

	@insertpiece( SampleRoughnessMap )

@end @property( hlms_use_prepass )
	ivec2 iFragCoord = ivec2( gl_FragCoord.x,
							  @property( !hlms_forwardplus_flipY )passBuf.windowHeight.x - @end
							  gl_FragCoord.y );

	@property( hlms_use_prepass_msaa )
		//SV_Coverage/gl_SampleMaskIn is always before depth & stencil tests,
		//so we need to perform the test ourselves
		//See http://www.yosoygames.com.ar/wp/2017/02/beware-of-sv_coverage/
		float msaaDepth;
		int subsampleDepthMask;
		float pixelDepthZ;
		float pixelDepthW;
		float pixelDepth;
		int intPixelDepth;
		int intMsaaDepth;
		//Unfortunately there are precision errors, so we allow some ulp errors.
		//200 & 5 are arbitrary, but were empirically found to be very good values.
		int ulpError = int( lerp( 200.0, 5.0, gl_FragCoord.z ) );
		@foreach( hlms_use_prepass_msaa, n )
			pixelDepthZ = interpolateAtSample( inPs.zwDepth.x, @n );
			pixelDepthW = interpolateAtSample( inPs.zwDepth.y, @n );
			pixelDepth = pixelDepthZ / pixelDepthW;
			msaaDepth = texelFetch( gBuf_depthTexture, iFragCoord.xy, @n );
			intPixelDepth = floatBitsToInt( pixelDepth );
			intMsaaDepth = floatBitsToInt( msaaDepth );
			subsampleDepthMask = int( (abs( intPixelDepth - intMsaaDepth ) <= ulpError) ? 0xffffffffu : ~(1u << @nu) );
			//subsampleDepthMask = int( (pixelDepth <= msaaDepth) ? 0xffffffffu : ~(1u << @nu) );
			gl_SampleMaskIn &= subsampleDepthMask;
		@end

		gl_SampleMaskIn[0] = gl_SampleMaskIn[0] == 0u ? 1u : gl_SampleMaskIn[0];

		int gBufSubsample = findLSB( gl_SampleMaskIn[0] );
	@end @property( !hlms_use_prepass_msaa )
		//On non-msaa RTTs gBufSubsample is the LOD level.
		int gBufSubsample = 0;
	@end

	nNormal = normalize( texelFetch( gBuf_normals, iFragCoord, gBufSubsample ).xyz * 2.0 - 1.0 );
	vec2 shadowRoughness = texelFetch( gBuf_shadowRoughness, iFragCoord, gBufSubsample ).xy;

	float fShadow = shadowRoughness.x;

	@property( roughness_map )
		ROUGHNESS = shadowRoughness.y * 0.98 + 0.02; /// ROUGHNESS is a constant otherwise
	@end
@end

@insertpiece( SampleSpecularMap )

@property( !hlms_prepass )
	//Everything's in Camera space
@property( hlms_lights_spot || use_envprobe_map || hlms_use_ssr || use_planar_reflections || ambient_hemisphere || hlms_forwardplus )
	vec3 viewDir	= normalize( -inPs.pos );
	float NdotV		= clamp( dot( nNormal, viewDir ), 0.0, 1.0 );
@end

@property( !ambient_fixed )
	vec3 finalColour = vec3(0);
@end @property( ambient_fixed )
	vec3 finalColour = passBuf.ambientUpperHemi.xyz * @insertpiece( kD ).xyz;
@end

	@insertpiece( custom_ps_preLights )

@property( !custom_disable_directional_lights )
@property( hlms_lights_directional )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[0].position.xyz, viewDir, NdotV, passBuf.lights[0].diffuse, passBuf.lights[0].specular ) @insertpiece(DarkenWithShadowFirstLight);
@end
@foreach( hlms_lights_directional, n, 1 )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[@n].position.xyz, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[@n].position.xyz, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular );@end
@end

@property( hlms_lights_point || hlms_lights_spot )	vec3 lightDir;
	float fDistance;
	vec3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional_non_caster )
	lightDir = passBuf.lights[@n].position.xyz - inPs.pos;
	fDistance= length( lightDir );
	if( fDistance <= passBuf.lights[@n].attenuation.x @insertpiece( andObjLightMaskCmp ) )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular )@insertpiece( DarkenWithShadowPoint );
		float atten = 1.0 / (0.5 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * atten;
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
	lightDir = passBuf.lights[@n].position.xyz - inPs.pos;
	fDistance= length( lightDir );
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - passBuf.lights[@n].position.xyz ), passBuf.lights[@n].spotDirection );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( inPs.pos - passBuf.lights[@n].position.xyz ), zAxis( passBuf.lights[@n].spotQuaternion ) );@end
	if( fDistance <= passBuf.lights[@n].attenuation.x && spotCosAngle >= passBuf.lights[@n].spotParams.y @insertpiece( andObjLightMaskCmp ) )
	{
		lightDir *= 1.0 / fDistance;
	@property( hlms_lights_spot_textured )
		vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], inPs.pos );
		float spotAtten = texture( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = clamp( (spotCosAngle - passBuf.lights[@n].spotParams.y) * passBuf.lights[@n].spotParams.x, 0.0, 1.0 );
		spotAtten = pow( spotAtten, passBuf.lights[@n].spotParams.z );
	@end
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (0.5 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )
@insertpiece( applyIrradianceVolumes )

@property( use_envprobe_map || hlms_use_ssr || use_planar_reflections || ambient_hemisphere )
	vec3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;

	@property( use_envprobe_map )
		@property( use_parallax_correct_cubemaps )
			vec3 envColourS;
			vec3 envColourD;
			vec3 posInProbSpace = toProbeLocalSpace( inPs.pos, @insertpiece( pccProbeSource ) );
			float probeFade = getProbeFade( posInProbSpace, @insertpiece( pccProbeSource ) );
			if( probeFade > 0 )
			{
				vec3 reflDirLS = localCorrect( reflDir, posInProbSpace, @insertpiece( pccProbeSource ) );
				vec3 nNormalLS = localCorrect( nNormal, posInProbSpace, @insertpiece( pccProbeSource ) );
				envColourS = textureLod( texEnvProbeMap,
										 reflDirLS, ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
				envColourD = textureLod( texEnvProbeMap,
										 nNormalLS, 11.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;

				envColourS = envColourS * clamp( probeFade * 200.0, 0.0, 1.0 );
				envColourD = envColourD * clamp( probeFade * 200.0, 0.0, 1.0 );
			}
			else
			{
				//TODO: Fallback to a global cubemap.
				envColourS = vec3( 0, 0, 0 );
				envColourD = vec3( 0, 0, 0 );
			}
		@end @property( !use_parallax_correct_cubemaps )
			vec3 envColourS = textureLod( texEnvProbeMap, reflDir * passBuf.invViewMatCubemap, ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
			vec3 envColourD = textureLod( texEnvProbeMap, nNormal * passBuf.invViewMatCubemap, 11.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
		@end
		@property( !hw_gamma_read )	//Gamma to linear space
			envColourS = envColourS * envColourS;
			envColourD = envColourD * envColourD;
		@end
	@end

	@property( hlms_use_ssr )
		//TODO: SSR pass should be able to combine global & local cubemap.
		vec4 ssrReflection = texelFetch( ssrTexture, iFragCoord, 0 ).xyzw;
		@property( use_envprobe_map )
			envColourS = mix( envColourS.xyz, ssrReflection.xyz, ssrReflection.w );
		@end @property( !use_envprobe_map )
			vec3 envColourS = ssrReflection.xyz * ssrReflection.w;
			vec3 envColourD = vec3( 0, 0, 0 );
		@end
	@end

	@insertpiece( DoPlanarReflectionsPS )

	@property( ambient_hemisphere )
		float ambientWD = dot( passBuf.ambientHemisphereDir.xyz, nNormal ) * 0.5 + 0.5;
		float ambientWS = dot( passBuf.ambientHemisphereDir.xyz, reflDir ) * 0.5 + 0.5;

		@property( use_envprobe_map || hlms_use_ssr || use_planar_reflections )
			envColourS	+= mix( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			envColourD	+= mix( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end @property( !use_envprobe_map && !hlms_use_ssr && !use_planar_reflections )
			vec3 envColourS = mix( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			vec3 envColourD = mix( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end
	@end

	@insertpiece( BRDF_EnvMap )
@end
@end ///!hlms_prepass

@property( !hlms_render_depth_only )
	@property( !hlms_prepass )
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

		@property( debug_pssm_splits )
			outColour.xyz = mix( outColour.xyz, debugPssmSplit.xyz, 0.2f );
		@end
	@end @property( hlms_prepass )
		outNormals			= vec4( nNormal * 0.5 + 0.5, 1.0 );
		@property( hlms_pssm_splits )
			outShadowRoughness	= vec2( fShadow, (ROUGHNESS - 0.02) * 1.02040816 );
		@end @property( !hlms_pssm_splits )
			outShadowRoughness	= vec2( 1.0, (ROUGHNESS - 0.02) * 1.02040816 );
		@end
	@end
@end

	@insertpiece( custom_ps_posExecution )
}
@end
@property( hlms_shadowcaster )

@insertpiece( DeclShadowCasterMacros )

@property( alpha_test )
	Material material;
	float diffuseCol;
	@property( num_textures )uniform sampler2DArray textureMaps[@value( num_textures )];@end
	@property( diffuse_map )uint diffuseIdx;@end
	@property( detail_weight_map )uint weightMapIdx;@end
	@foreach( 4, n )
		@property( detail_map@n )uint detailMapIdx@n;@end @end
@end

@property( hlms_shadowcaster_point || exponential_shadow_maps )
	@insertpiece( PassDecl )
@end

void main()
{
	@insertpiece( custom_ps_preExecution )

@property( alpha_test )
	@property( !lower_gpu_overhead )
		uint materialId	= instance.worldMaterialIdx[inPs.drawId].x & 0x1FFu;
		material = materialArray.m[materialId];
	@end @property( lower_gpu_overhead )
		material = materialArray.m[0];
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
	@property( !diffuse_map )diffuseCol = material.bgDiffuse.w;@end

	/// Blend the detail diffuse maps with the main diffuse.
@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @add( t, 1 ) @end

	/// Apply the material's alpha over the textures
@property( TODO_REFACTOR_ACCOUNT_MATERIAL_ALPHA )	diffuseCol.xyz *= material.kD.xyz;@end

	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol )
		discard;
@end /// !alpha_test

	@insertpiece( DoShadowCastPS )

	@insertpiece( custom_ps_posExecution )
}
@end
