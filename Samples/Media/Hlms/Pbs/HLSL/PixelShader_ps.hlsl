@insertpiece( SetCrossPlatformSettings )
@insertpiece( DeclareUvModifierMacros )

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
struct PS_INPUT
{
@insertpiece( VStoPS_block )
};

@property( !hlms_shadowcaster )

@property( hlms_use_prepass )
	@property( !hlms_use_prepass_msaa )
		Texture2D<unorm float4> gBuf_normals			: register(t@value(gBuf_normals));
		Texture2D<unorm float2> gBuf_shadowRoughness	: register(t@value(gBuf_shadowRoughness));
	@end @property( hlms_use_prepass_msaa )
		Texture2DMS<unorm float4> gBuf_normals			: register(t@value(gBuf_normals));
		Texture2DMS<unorm float2> gBuf_shadowRoughness	: register(t@value(gBuf_shadowRoughness));
		Texture2DMS<float> gBuf_depthTexture			: register(t@value(gBuf_depthTexture));
	@end

	@property( hlms_use_ssr )
		Texture2D<float4> ssrTexture : register(t@value(ssrTexture));
	@end
@end

@insertpiece( DeclPlanarReflTextures )

@property( two_sided_lighting )
@piece( two_sided_flip_normal )* (gl_FrontFacing ? 1.0 : -1.0)@end
@end

@property( hlms_forwardplus )
Buffer<uint> f3dGrid : register(t1);
Buffer<float4> f3dLightList : register(t2);@end

@property( irradiance_volumes )
	Texture3D<float4>	irradianceVolume		: register(t@value(irradianceVolumeTexUnit));
	SamplerState		irradianceVolumeSampler	: register(s@value(irradianceVolumeTexUnit));
@end

@property( !roughness_map )#define ROUGHNESS material.kS.w@end
@property( num_textures )Texture2DArray textureMaps[@value( num_textures )] : register(t@value(textureRegStart));@end
@property( use_envprobe_map )TextureCube	texEnvProbeMap : register(t@value(envMapReg));
SamplerState envMapSamplerState : register(s@value(envMapReg));@end

@foreach( numSamplerStates, n )
	SamplerState samplerState@n : register(s@counter(samplerStateStart));@end

@property( normal_map )
@property( hlms_qtangent )
@piece( tbnApplyReflection ) * inPs.biNormalReflection@end
@end
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
	tsNormal.xy = textureMaps[@value( normal_map_tex_idx )].Sample( samplerState@value( normal_map_tex_idx ), uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = textureMaps[@value( normal_map_tex_idx )].Sample( samplerState@value( normal_map_tex_idx ), uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( max( 0, 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}
@end
@property( normal_weight_tex )#define normalMapWeight material.emissive.w@end
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

@insertpiece( DeclOutputType )

@insertpiece( custom_ps_functions )

@insertpiece( output_type ) main( PS_INPUT inPs
@property( hlms_vpos ), float4 gl_FragCoord : SV_Position@end
@property( two_sided_lighting ), bool gl_FrontFacing : SV_IsFrontFace@end
@property( hlms_use_prepass_msaa && hlms_use_prepass ), uint gl_SampleMask : SV_Coverage@end )
{
	PS_OUTPUT outPs;
	@insertpiece( custom_ps_preExecution )

	Material material;
	
@property( diffuse_map )	uint diffuseIdx;@end
@property( normal_map_tex )	uint normalIdx;@end
@property( specular_map )	uint specularIdx;@end
@property( roughness_map )	uint roughnessIdx;@end
@property( detail_weight_map )	uint weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )uint detailMapIdx@n;@end @end
@foreach( 4, n )
	@property( detail_map_nm@n )uint detailNormMapIdx@n;@end @end
@property( emissive_map )	uint emissiveMapIdx;@end
@property( use_envprobe_map )	uint envMapIdx;@end

float4 diffuseCol;
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
@property( detail_weight_map )	weightMapIdx	= material.indices0_3.z & 0x0000FFFFu;@end
@property( detail_map0 )	detailMapIdx0		= material.indices0_3.z >> 16u;@end
@property( detail_map1 )	detailMapIdx1		= material.indices0_3.w & 0x0000FFFFu;@end
@property( detail_map2 )	detailMapIdx2		= material.indices0_3.w >> 16u;@end
@property( detail_map3 )	detailMapIdx3		= material.indices4_7.x & 0x0000FFFFu;@end
@property( detail_map_nm0 )	detailNormMapIdx0	= material.indices4_7.x >> 16u;@end
@property( detail_map_nm1 )	detailNormMapIdx1	= material.indices4_7.y & 0x0000FFFFu;@end
@property( detail_map_nm2 )	detailNormMapIdx2	= material.indices4_7.y >> 16u;@end
@property( detail_map_nm3 )	detailNormMapIdx3	= material.indices4_7.z & 0x0000FFFFu;@end
@property( emissive_map )	emissiveMapIdx		= material.indices4_7.z >> 16u;@end
@property( use_envprobe_map )	envMapIdx		= material.indices4_7.w & 0x0000FFFFu;@end

	@insertpiece( DeclareObjLightMask )

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
	float4 detailCol@n	= textureMaps[@value(detail_map@n_idx)].Sample(
								samplerState@value(detail_map@n_idx),
								float3( UV_DETAIL@n( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetail@n ) ),
										detailMapIdx@n ) );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol@n.xyz = detailCol@n.xyz * detailCol@n.xyz;@end
	detailWeights.@insertpiece(detail_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_swizzle@n);@end
@end

@property( !hlms_prepass || alpha_test )
	@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value.
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
		float3 geomNormal = normalize( inPs.normal ) @insertpiece( two_sided_flip_normal );
		float3 vTangent = normalize( inPs.tangent );

		//Get the TBN matrix
		float3 vBinormal	= normalize( cross( geomNormal, vTangent )@insertpiece( tbnApplyReflection ) );
		float3x3 TBN		= float3x3( vTangent, vBinormal, geomNormal );

		@property( normal_map_tex )nNormal = getTSNormal( float3( UV_NORMAL( inPs.uv@value(uv_normal).xy ),
																  normalIdx ) );@end
		@property( normal_weight_tex )
			// Apply the weight to the main normal map
			nNormal = lerp( float3( 0.0, 0.0, 1.0 ), nNormal, normalMapWeight );
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

	@insertpiece( DoDirectionalShadowMaps )

	@insertpiece( SampleRoughnessMap )

@end @property( hlms_use_prepass )
	int2 iFragCoord = int2( gl_FragCoord.xy );

	@property( hlms_use_prepass_msaa )
		uint gl_SampleMaskIn = gl_SampleMask;
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
		int ulpError = int( lerp( 200, 5, gl_FragCoord.z ) );
		@foreach( hlms_use_prepass_msaa, n )
			pixelDepthZ = EvaluateAttributeAtSample( inPs.zwDepth.x, @n );
			pixelDepthW = EvaluateAttributeAtSample( inPs.zwDepth.y, @n );
			pixelDepth = pixelDepthZ / pixelDepthW;
			msaaDepth = gBuf_depthTexture.Load( iFragCoord.xy, @n );
			intPixelDepth = asint( pixelDepth );
			intMsaaDepth = asint( msaaDepth );
			subsampleDepthMask = int( (abs( intPixelDepth - intMsaaDepth ) <= ulpError) ? 0xffffffffu : ~(1u << @nu) );
			//subsampleDepthMask = int( (pixelDepth <= msaaDepth) ? 0xffffffffu : ~(1u << @nu) );
			gl_SampleMaskIn &= subsampleDepthMask;
		@end

		gl_SampleMaskIn = gl_SampleMaskIn == 0 ? 1 : gl_SampleMaskIn;

		int gBufSubsample = firstbitlow( gl_SampleMaskIn );

		nNormal = normalize( gBuf_normals.Load( iFragCoord, gBufSubsample ).xyz * 2.0 - 1.0 );
		float2 shadowRoughness = gBuf_shadowRoughness.Load( iFragCoord, gBufSubsample ).xy;
	@end @property( !hlms_use_prepass_msaa )
		nNormal = normalize( gBuf_normals.Load( int3( iFragCoord, 0 ) ).xyz * 2.0 - 1.0 );
		float2 shadowRoughness = gBuf_shadowRoughness.Load( int3( iFragCoord, 0 ) ).xy;
	@end

	float fShadow = shadowRoughness.x;

	@property( roughness_map )
		ROUGHNESS = shadowRoughness.y * 0.98 + 0.02; /// ROUGHNESS is a constant otherwise
	@end
@end

	@insertpiece( SampleSpecularMap )

@property( !hlms_prepass )
	//Everything's in Camera space
@property( hlms_lights_spot || use_envprobe_map || hlms_use_ssr || use_planar_reflections || ambient_hemisphere || hlms_forwardplus )
	float3 viewDir	= normalize( -inPs.pos );
	float NdotV		= saturate( dot( nNormal, viewDir ) );
@end

@property( !ambient_fixed )
	float3 finalColour = float3(0, 0, 0);
@end @property( ambient_fixed )
	float3 finalColour = passBuf.ambientUpperHemi.xyz * @insertpiece( kD ).xyz;
@end

	@insertpiece( custom_ps_preLights )

@property( !custom_disable_directional_lights )
@property( hlms_lights_directional )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[0].position.xyz, viewDir, NdotV, passBuf.lights[0].diffuse, passBuf.lights[0].specular, material, nNormal @insertpiece( brdfExtraParams ) ) @insertpiece( DarkenWithShadowFirstLight );
@end
@foreach( hlms_lights_directional, n, 1 )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[@n].position.xyz, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	@insertpiece( ObjLightMaskCmp )
		finalColour += BRDF( passBuf.lights[@n].position.xyz, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) );@end
@end

@property( hlms_lights_point || hlms_lights_spot )	float3 lightDir;
	float fDistance;
	float3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional_non_caster )
	lightDir = passBuf.lights[@n].position.xyz - inPs.pos;
	fDistance= length( lightDir );
	if( fDistance <= passBuf.lights[@n].attenuation.x @insertpiece( andObjLightMaskCmp ) )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadowPoint );
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
		float3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], inPs.pos );
		float spotAtten = texture( texSpotLight, normalize( posInLightSpace ).xy ).x; //TODO
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = saturate( (spotCosAngle - passBuf.lights[@n].spotParams.y) * passBuf.lights[@n].spotParams.x );
		spotAtten = pow( spotAtten, passBuf.lights[@n].spotParams.z );
	@end
		tmpColour = BRDF( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (0.5 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )
@insertpiece( applyIrradianceVolumes )

@property( emissive_map || emissive_constant )
	@insertpiece( SampleEmissiveMap )
	finalColour += emissiveCol.xyz;
@end

@property( use_envprobe_map || hlms_use_ssr || use_planar_reflections || ambient_hemisphere )
	float3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;
	
	@property( use_envprobe_map )
		@property( use_parallax_correct_cubemaps )
			float3 envColourS;
			float3 envColourD;
			float3 posInProbSpace = toProbeLocalSpace( inPs.pos, @insertpiece( pccProbeSource ) );
			float probeFade = getProbeFade( posInProbSpace, @insertpiece( pccProbeSource ) );
			if( probeFade > 0 )
			{
				float3 reflDirLS = localCorrect( reflDir, posInProbSpace, @insertpiece( pccProbeSource ) );
				float3 nNormalLS = localCorrect( nNormal, posInProbSpace, @insertpiece( pccProbeSource ) );
				envColourS = texEnvProbeMap.SampleLevel( envMapSamplerState,
														 reflDirLS, ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;
				envColourD = texEnvProbeMap.SampleLevel( envMapSamplerState,
														 nNormalLS, 11.0 ).xyz @insertpiece( ApplyEnvMapScale );// * 0.0152587890625;

				envColourS = envColourS * saturate( probeFade * 200.0 );
				envColourD = envColourD * saturate( probeFade * 200.0 );
			}
			else
			{
				//TODO: Fallback to a global cubemap.
				envColourS = float3( 0, 0, 0 );
				envColourD = float3( 0, 0, 0 );
			}
		@end @property( !use_parallax_correct_cubemaps )
			float3 envColourS = texEnvProbeMap.SampleLevel( envMapSamplerState, mul( reflDir, passBuf.invViewMatCubemap ), ROUGHNESS * 12.0 ).xyz @insertpiece( ApplyEnvMapScale );
			float3 envColourD = texEnvProbeMap.SampleLevel( envMapSamplerState, mul( nNormal, passBuf.invViewMatCubemap ), 11.0 ).xyz @insertpiece( ApplyEnvMapScale );
		@end
		@property( !hw_gamma_read )	//Gamma to linear space
			envColourS = envColourS * envColourS;
			envColourD = envColourD * envColourD;
		@end
	@end

	@property( hlms_use_ssr )
		//TODO: SSR pass should be able to combine global & local cubemap.
		float4 ssrReflection = ssrTexture.Load( int3( iFragCoord, 0 ) ).xyzw;
		@property( use_envprobe_map )
			envColourS = lerp( envColourS.xyz, ssrReflection.xyz, ssrReflection.w );
		@end @property( !use_envprobe_map )
			float3 envColourS = ssrReflection.xyz * ssrReflection.w;
			float3 envColourD = float3( 0, 0, 0 );
		@end
	@end

	@insertpiece( DoPlanarReflectionsPS )

	@property( ambient_hemisphere )
		float ambientWD = dot( passBuf.ambientHemisphereDir.xyz, nNormal ) * 0.5 + 0.5;
		float ambientWS = dot( passBuf.ambientHemisphereDir.xyz, reflDir ) * 0.5 + 0.5;

		@property( use_envprobe_map || hlms_use_ssr || use_planar_reflections )
			envColourS	+= lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			envColourD	+= lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end @property( !use_envprobe_map && !hlms_use_ssr && !use_planar_reflections )
			float3 envColourS = lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWD );
			float3 envColourD = lerp( passBuf.ambientLowerHemi.xyz, passBuf.ambientUpperHemi.xyz, ambientWS );
		@end
	@end

	@insertpiece( BRDF_EnvMap )
@end
@end ///!hlms_prepass

@property( !hlms_prepass )
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

	@property( debug_pssm_splits )
		outPs.colour0.xyz = lerp( outPs.colour0.xyz, debugPssmSplit.xyz, 0.2f );
	@end
@end @property( hlms_prepass )
	outPs.normals			= float4( nNormal * 0.5 + 0.5, 1.0 );
	@property( hlms_pssm_splits )
		outPs.shadowRoughness	= float2( fShadow, (ROUGHNESS - 0.02) * 1.02040816 );
	@end @property( !hlms_pssm_splits )
		outPs.shadowRoughness	= float2( 1.0, (ROUGHNESS - 0.02) * 1.02040816 );
	@end
@end

	@property( hlms_use_prepass_msaa && false )
		//Useful debug stuff for debugging precision issues.
		/*float testD = gBuf_depthTexture.Load( iFragCoord.xy, 0 );
		outPs.colour0.xyz = testD * testD * testD * testD * testD * testD * testD * testD;*/
		/*float3 col3 = lerp( outPs.colour0.xyz, float3( 1, 1, 1 ), 0.85 );
		outPs.colour0.xyz = 0;
		if( gl_SampleMaskIn & 0x1 )
			outPs.colour0.x = col3.x;
		if( gl_SampleMaskIn & 0x2 )
			outPs.colour0.y = col3.y;
		if( gl_SampleMaskIn & 0x4 )
			outPs.colour0.z = col3.z;
		if( gl_SampleMaskIn & 0x8 )
			outPs.colour0.w = 1.0;*/
		/*outPs.colour0.x = pixelDepth;
		outPs.colour0.y = msaaDepth;
		outPs.colour0.z = 0;
		outPs.colour0.w = 1;*/
	@end

	@insertpiece( custom_ps_posExecution )

@property( !hlms_render_depth_only )
	return outPs;
@end
}
@end
@property( hlms_shadowcaster )

@insertpiece( DeclShadowCasterMacros )

@property( num_textures )Texture2DArray textureMaps[@value( num_textures )] : register(t@value(textureRegStart));@end
@foreach( numSamplerStates, n )
	SamplerState samplerState@n : register(s@counter(samplerStateStart));@end

@property( hlms_shadowcaster_point || exponential_shadow_maps )
	@insertpiece( PassDecl )
@end

@insertpiece( DeclOutputType )

@insertpiece( output_type ) main( PS_INPUT inPs )
{
@property( !hlms_render_depth_only || exponential_shadow_maps || hlms_shadowcaster_point )
	PS_OUTPUT outPs;
@end
	@insertpiece( custom_ps_preExecution )

	
@property( alpha_test )
	Material material;
@property( diffuse_map )	uint diffuseIdx;@end
@property( detail_weight_map )	uint weightMapIdx;@end
@foreach( 4, n )
	@property( detail_map@n )uint detailMapIdx@n;@end @end

	float diffuseCol;
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
	float detailCol@n	= textureMaps[@value(detail_map@n_idx)].Sample(
									samplerState@value(detail_map@n_sampler),
									float3( @insertpiece(custom_ps_pre_detailmap@n)
											(inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetail@n ))
											@insertpiece(custom_ps_pos_detailmap@n),
											detailMapIdx@n ) ).w;
	detailCol@n = detailWeights.@insertpiece(detail_swizzle@n) * detailCol@n;@end
@end

@insertpiece( SampleDiffuseMap )

	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value.
	@property( !diffuse_map )diffuseCol = material.bgDiffuse.w;@end

	/// Blend the detail diffuse maps with the main diffuse.
@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @add( t, 1 ) @end

	/// Apply the material's diffuse over the textures
@property( TODO_REFACTOR_ACCOUNT_MATERIAL_ALPHA )	diffuseCol.xyz *= material.kD.xyz;@end

	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol )
		discard;
@end /// !alpha_test

	@insertpiece( DoShadowCastPS )

	@insertpiece( custom_ps_posExecution )

@property( !hlms_render_depth_only || exponential_shadow_maps || hlms_shadowcaster_point )
	return outPs;
@end
}
@end

