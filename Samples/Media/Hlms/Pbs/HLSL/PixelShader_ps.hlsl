
// START UNIFORM DECLARATION
@property( !hlms_shadowcaster )
@insertpiece( PassDecl )
@insertpiece( MaterialDecl )
@insertpiece( InstanceDecl )
@end
struct PS_INPUT
{
@insertpiece( VStoPS_block )
};
// END UNIFORM DECLARATION

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

float getShadow( Texture2D shadowMap, float4 psPosLN, float2 invShadowMapSize )
{
@property( !hlms_shadow_uses_depth_texture )
	float fDepth = psPosLN.z;
	float2 uv = psPosLN.xy / psPosLN.w;
	float3 o = float3( invShadowMapSize, -invShadowMapSize.x ) * 0.3;

	float c = shadowMap.SampleCmpLevelZero( shadowSampler, uv.xy - o.xy, fDepth );
	return c;@end
@property( hlms_shadow_uses_depth_texture )
	return texture( shadowMap, psPosLN.xyz, 0 ).x;@end
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
	tsNormal.z	= sqrt( 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y );

	return tsNormal;
}
@end
@property( detail_maps_normal )float3 getTSDetailNormal( SamplerState samplerState, Texture2D normalMap, float3 uv )
{
	float3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = normalMap.Sample( samplerState, uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = normalMap.Sample( samplerState, uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y );

	return tsNormal;
}

	@property( normal_weight_tex )#define normalMapWeight material.F0.w@end
	@foreach( 4, n )
		@property( normal_weight_detail@n )
			@piece( detail@n_nm_weight_mul ) * material.normalWeights.@insertpiece( detail_swizzle@n )]@end
		@end
	@end
@end

@property( hlms_normal || hlms_qtangent )
float3 cookTorrance( float3 lightDir, float3 viewDir, float NdotV, float3 lightDiffuse, float3 lightSpecular, Material material, float3 nNormal @insertpiece( brdfExtraParamDefs ) )
{
	float3 halfWay= normalize( lightDir + viewDir );
	float NdotL = saturate( dot( nNormal, lightDir ) );
	float NdotH = clamp( dot( nNormal, halfWay ), 0.001, 1.0 );
	float VdotH = clamp( dot( viewDir, halfWay ), 0.001, 1.0 );

	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness term (Beckmann distribution)
	//Formula:
	//	Where alpha = NdotH and m = roughness
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( -tan(alpha)^2 / m^2 ) ]
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( ( cos(alpha)^2 - 1 )  /  (m^2 cos(alpha)^2 ) ]
	float NdotH_sq = NdotH * NdotH;
	float roughness_a = 1.0 / ( 3.141592654 * sqR * NdotH_sq * NdotH_sq );//( 1 / (m^2 x cos(alpha)^4 )
	float roughness_b = NdotH_sq - 1.0;	//( cos(alpha)^2 - 1 )
	float roughness_c = sqR * NdotH_sq;		//( m^2 cos(alpha)^2 )

	//Avoid Inf * 0 = NaN; we need Inf * 0 = 0
	float R = min( roughness_a, 65504.0 ) * exp( roughness_b / roughness_c );

	//Geometric term
	float shared_geo = 2.0 * NdotH / VdotH;
	float geo_b	= shared_geo * NdotV;
	float geo_c	= shared_geo * NdotL;
	float G	 	= min( 1.0, min( geo_b, geo_c ) );

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	//	fresnelD = lerp( (1 - N*L)^5, 1, 1 - F0 )
	@insertpiece( FresnelType ) fresnelS = material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (1.0 - material.F0.@insertpiece( FresnelSwizzle ));
	@insertpiece( FresnelType ) fresnelD = 1.0 - material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * material.F0.@insertpiece( FresnelSwizzle );

	//Avoid very small denominators, they go to NaN or cause aliasing artifacts
	@insertpiece( FresnelType ) Rs = ( fresnelS * (R * G)  ) / max( 4.0 * NdotV * NdotL, 0.01 );

	return NdotL * (material.kS.xyz * lightSpecular * Rs @insertpiece( MulSpecularMapValue ) +
					material.kD.xyz * lightDiffuse * fresnelD @insertpiece( MulDiffuseMapValue ));
}
@end

@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@value(CurrentShadowMap), passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize )@end @end

float4 main( PS_INPUT inPs
@property( hlms_vpos ), float4 gl_FragCoord : SV_Position@end ) : SV_Target0
{
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
@property( specular_map )float3 specularCol;@end
@property( roughness_map )float ROUGHNESS;@end
	
@property( hlms_normal || hlms_qtangent )	float3 nNormal;@end
	
@property( hlms_normal || hlms_qtangent )
        uint materialId	= worldMaterialIdx[inPs.drawId].x & 0x1FFu;
	material = materialArray[materialId];
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

@property( detail_maps_diffuse || detail_maps_normal )
	@property( detail_weight_map )
		float4 detailWeights = @insertpiece( SamplerDetailWeightMap );
		@property( detail_weights )detailWeights *= material.cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )float4 detailWeights = material.cDetailWeights;@end
		@property( !detail_weights )float4 detailWeights = float4( 1.0, 1.0, 1.0, 1.0 );@end
	@end
@end

@foreach( detail_maps_diffuse, n )@property( detail_map@n )
	float4 detailCol@n	= textureMaps[@value(detail_map@n_idx)].Sample( samplerStates[@value(detail_map@n_idx)], float3( inPs.uv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ), detailMapIdx@n ) );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol@n.xyz = detailCol@n.xyz * detailCol@n.xyz;@end
	detailWeights.@insertpiece(detail_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_swizzle@n);@end
@end

@property( alpha_test && !diffuse_map && detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) detailCol0.a )
		discard;@end

@property( !normal_map )
	nNormal = normalize( inPs.normal );
@end @property( normal_map )
	float3 geomNormal = normalize( inPs.normal );
	float3 vTangent = normalize( inPs.tangent );

	//Get the TBN matrix
	float3 vBinormal	= normalize( cross( vTangent, geomNormal )@insertpiece( tbnApplyReflection ) );
	float3x3 TBN		= float3x3( vTangent, vBinormal, geomNormal );

	@property( normal_map_tex )nNormal = getTSNormal( float3( inPs.uv@value(uv_normal).xy, normalIdx ) );@end
	@property( normal_weight_tex )nNormal = lerp( float3( 0.0, 0.0, 1.0 ), nNormal, normalMapWeight );@end
@end

@property( hlms_pssm_splits )
    float fShadow = 1.0;
    if( inPs.depth <= passBuf.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@foreach( hlms_pssm_splits, n, 1 )	else if( inPs.depth <= passBuf.pssmSplitPoints@value(CurrentShadowMap) )
        fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@n, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end @end @property( !hlms_pssm_splits && hlms_num_shadow_maps )
    float fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, passBuf.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end

@insertpiece( SampleDiffuseMap )

@property( alpha_test )
	@property( diffuse_map )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) diffuseCol.a )
		discard;
	@end @property( !diffuse_map && !detail_maps_diffuse )
	if( material.kD.w @insertpiece( alpha_test_cmp_func ) 1.0 )
		discard;
	@end
@end

@insertpiece( SampleSpecularMap )
@insertpiece( SampleRoughnessMap )

	@property( !diffuse_map && detail_maps_diffuse )diffuseCol = float4( 0.0, 0.0, 0.0, 0.0 );@end

@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @end

@property( normal_map_tex )
	@piece( detail_nm_op_sum )+=@end
	@piece( detail_nm_op_mul )*=@end
@end @property( !normal_map_tex )
	@piece( detail_nm_op_sum )=@end
	@piece( detail_nm_op_mul )=@end
@end

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

	//Everything's in Camera space, we use Cook-Torrance lighting
@property( hlms_lights_spot || envprobe_map )
	float3 viewDir	= normalize( -inPs.pos );
	float NdotV		= saturate( dot( nNormal, viewDir ) );@end

	float3 finalColour = float3(0, 0, 0);
@property( hlms_lights_directional )
	finalColour += cookTorrance( passBuf.lights[0].position, viewDir, NdotV, passBuf.lights[0].diffuse, passBuf.lights[0].specular, material, nNormal @insertpiece( brdfExtraParams ) );
@property( hlms_num_shadow_maps )	finalColour *= fShadow;	//1st directional light's shadow@end
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += cookTorrance( passBuf.lights[@n].position, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );@end
@foreach( hlms_lights_directional_non_caster, n, hlms_lights_directional )
	finalColour += cookTorrance( passBuf.lights[@n].position, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) );@end

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
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
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
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, passBuf.lights[@n].diffuse, passBuf.lights[@n].specular, material, nNormal @insertpiece( brdfExtraParams ) )@insertpiece( DarkenWithShadow );
		float atten = 1.0 / (1.0 + (passBuf.lights[@n].attenuation.y + passBuf.lights[@n].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@insertpiece( forward3dLighting )

@property( envprobe_map )
	float3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;
	float3 envColour = texEnvProbeMap.SampleLevel( samplerStates[@value(num_textures)], mul( reflDir, passBuf.invViewMatCubemap ), ROUGHNESS * 12.0 ).xyz;
	@property( !hw_gamma_read )//Gamma to linear space
	envColour = envColour * envColour;@end
	finalColour += cookTorrance( reflDir, viewDir, NdotV, envColour, envColour * (ROUGHNESS * ROUGHNESS), material, nNormal @insertpiece( brdfExtraParams ) );@end

@property( !hw_gamma_write )
	//Linear to Gamma space
	outColour.xyz	= sqrt( finalColour );
@end @property( hw_gamma_write )
	outColour.xyz	= finalColour;
@end
	outColour.w		= 1.0;
@end @property( !hlms_normal && !hlms_qtangent )
	outColour = float4( 1.0, 1.0, 1.0, 1.0 );
@end

	return outColour;
}
@end
@property( hlms_shadowcaster )
float main( PS_INPUT inPs ) : SV_Target0
{
	return inPs.depth;
}
@end
