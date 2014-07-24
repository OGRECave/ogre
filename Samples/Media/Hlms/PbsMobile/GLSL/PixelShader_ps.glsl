@property( GL3+ )#version 330@end
@property( !GL3+ )#define in varying@end
/*#ifdef GL_ES
precision mediump float;
#endif*/
#define FRAG_COLOR		0
@property( !hlms_shadowcaster )
@property( GL3+ )layout(location = FRAG_COLOR, index = 0) out vec4 outColour;@end
@property( !GL3+ )#define outColour gl_FragColor@end

@property( hlms_normal )
in mediump vec3 psPos;
in mediump vec3 psNormal;
mediump vec3 nNormal;
@property( normal_map )in mediump vec3 psTangent;
mediump vec3 vTangent;@end
@end
@property( hlms_pssm_splits )in float psDepth;@end
@foreach( hlms_uv_count, n )
in mediump vec@value( hlms_uv_count@n ) psUv@n;@end
@foreach( hlms_num_shadow_maps, n )
in highp vec4 psPosL@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per pass
@property( hlms_num_shadow_maps )uniform mediump vec2 invShadowMapSize[@value(hlms_num_shadow_maps)];
uniform mediump float pssmSplitPoints[@value(hlms_pssm_splits)];@end
@property( hlms_lights_spot )uniform mediump vec3 lightPosition[@value(hlms_lights_spot)];
uniform mediump vec3 lightDiffuse[@value(hlms_lights_spot)];
uniform mediump vec3 lightSpecular[@value(hlms_lights_spot)];
@property(hlms_lights_attenuation)uniform mediump vec3 attenuation[@value(hlms_lights_attenuation)];@end
@property(hlms_lights_spotparams)uniform mediump vec3 spotDirection[@value(hlms_lights_spotparams)];
uniform mediump vec3 spotParams[@value(hlms_lights_spotparams)];@end
@property( envprobe_map )uniform highp mat3 invViewMat; //This uniform depends on each renderable, but is the same for every pass@end
@end

//Uniforms that change per entity
uniform lowp float roughness;
/* kD is already divided by PI to make it energy conserving.
  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
*/
uniform lowp vec3 kD;
uniform lowp vec3 kS;
@property( fresnel_scalar )@piece( FresnelType )lowp vec3@end@end
@property( !fresnel_scalar ) @piece( FresnelType )lowp float@end @end
//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
uniform @insertpiece( FresnelType ) F0;
@property( uv_atlas )uniform mediump vec3 atlasOffsets[@value( uv_atlas )];@end
// END UNIFORM DECLARATION

@property( !specular_map )#define ROUGHNESS roughness@end
@property( diffuse_map )uniform lowp sampler2D	texDiffuseMap;@end
@property( normal_map )uniform lowp sampler2D	texNormalMap;@end
@property( specular_map )uniform lowp sampler2D	texSpecularMap;@end
@property( envprobe_map )
@property( hlms_cube_arrays_supported )uniform lowp samplerCube	texEnvProbeMap;@end
@property( !hlms_cube_arrays_supported )uniform lowp sampler2D	texEnvProbeMap;@end @end
@property( detail_weight_map )uniform lowp sampler2D	texDetailWeightMap;@end
@property( detail_maps_diffuse )uniform lowp sampler2D	texDetailMap[@value( detail_maps_diffuse )];@end
@property( detail_maps_normals )uniform lowp sampler2D	texDetailNormalMap[@value( detail_maps_normals )];@end

@property( diffuse_map )lowp vec4 diffuseCol;
@piece( SampleDiffuseMap )	diffuseCol = texture2D( texDiffuseMap, psUv@value(uv_diffuse).xy * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy );
@property( !hw_gamma_read )	//Gamma to linear space
	diffuseCol = diffuseCol * diffuseCol;@end @end
@piece( MulDiffuseMapValue )* diffuseCol.xyz@end@end
@property( specular_map )lowp vec4 specularCol;
lowp float ROUGHNESS;
@piece( SampleSpecularMap )	specularCol = texture2D( texSpecularMap, psUv@value(uv_specular).xy * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy );
	ROUGHNESS = roughness * specularCol.w;@end
@piece( MulSpecularMapValue )* specularCol.xyz@end@end

@property( hlms_num_shadow_maps )
@property( hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2DShadow@end @end
@property( !hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2D@end @end
uniform @insertpiece( SAMPLER2DSHADOW ) texShadowMap[@value(hlms_num_shadow_maps)];

lowp float getShadow( @insertpiece( SAMPLER2DSHADOW ) shadowMap, highp vec4 psPosLN, mediump vec2 invShadowMapSize )
{
@property( !hlms_shadow_usues_depth_texture )
	highp float fDepth = psPosLN.z;
	mediump vec2 uv = psPosLN.xy / psPosLN.w;
	highp vec3 o = vec3( invShadowMapSize, -invShadowMapSize.x ) * 0.3;

	// 2x2 PCF
	highp float c =	(fDepth <= texture2D(shadowMap, uv - o.xy).r) ? 1 : 0; // top left
	c +=		(fDepth <= texture2D(shadowMap, uv - o.zy).r) ? 1 : 0; // top right
	c +=		(fDepth <= texture2D(shadowMap, uv + o.zy).r) ? 1 : 0; // bottom left
	c +=		(fDepth <= texture2D(shadowMap, uv + o.xy).r) ? 1 : 0; // bottom right

	return c * 0.25;@end
@property( hlms_shadow_usues_depth_texture )
	return texture2D( shadowMap, psPosLN.xyz, 0 ).x;@end
}
@end

@property( hlms_lights_spot_textured )mediump vec3 zAxis( mediump vec4 qQuat )
{
	Real fTy  = 2.0 * qQuat.y;
	Real fTz  = 2.0 * qQuat.z;
	Real fTwy = fTy * qQuat.w;
	Real fTwz = fTz * qQuat.w;
	Real fTxy = fTy * qQuat.x;
	Real fTxz = fTz * qQuat.x;
	Real fTyy = fTy * qQuat.y;
	Real fTzz = fTz * qQuat.z;

	return mediump vec3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}
mediump vec3 qmul( mediump vec4 q, mediump vec3 v )
{
	return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map )mediump vec3 getTSNormal( lowp sampler2D normalMap, lowp vec2 uv0 )
{
	mediump vec3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = texture2D( normalMap, uv0 * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = texture2D( normalMap, uv0  * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).xw * 2.0 + 1.0;
@end
	tsNormal.z	= sqrt( 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y );

	return tsNormal;
}
@end

mediump vec3 cookTorrance( mediump vec3 lightDir, mediump vec3 viewDir, lowp float NdotV, mediump vec3 lightDiffuse, mediump vec3 lightSpecular )
{
	mediump vec3 halfWay= normalize( lightDir + viewDir );
	lowp float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	lowp float NdotH = clamp( dot( nNormal, halfWay ), 0.0, 1.0 );
	lowp float VdotH = clamp( dot( viewDir, halfWay ), 0.001, 1.0 );

	lowp float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness term (Beckmann distribution)
	//Formula:
	//	Where alpha = NdotH and m = roughness
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( -tan(alpha)^2 / m^2 ) ]
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( ( cos(alpha)^2 - 1 )  /  (m^2 cos(alpha)^2 ) ]
	mediump float NdotH_sq = NdotH * NdotH;
	highp float roughness_a = 1.0 / ( 3.141592654 * sqR * NdotH_sq * NdotH_sq );//( 1 / (m^2 x cos(alpha)^4 )
	highp float roughness_b = NdotH_sq - 1.0;	//( cos(alpha)^2 - 1 )
	highp float roughness_c = sqR * NdotH_sq;		//( m^2 cos(alpha)^2 )

	//Avoid Inf * 0 = NaN; we need Inf * 0 = 0
	highp float R = min( roughness_a, 65504.0 ) * exp( roughness_b / roughness_c );

	//Geometric term
	highp float shared_geo = 2.0 * NdotH / VdotH;
	highp float geo_b	= shared_geo * NdotV;
	highp float geo_c	= shared_geo * NdotL;
	highp float G	 	= min( 1.0, min( geo_b, geo_c ) );

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	//	fresnelD = lerp( (1 - N*L)^5, 1, 1 - F0 )
	@insertpiece( FresnelType ) fresnelS = F0 + pow( 1.0 - VdotH, 5.0 ) * (1.0 - F0);
	@insertpiece( FresnelType ) fresnelD = 1.0 - F0 + pow( 1.0 - NdotL, 5.0 ) * F0;

	//Avoid very small denominators, they go to NaN or cause aliasing artifacts
	@insertpiece( FresnelType ) Rs = ( fresnelS * (R * G)  ) / max( 4.0 * NdotV * NdotL, 0.01 );

	return NdotL * (kS * lightSpecular * Rs @insertpiece( MulSpecularMapValue ) +
					kD * lightDiffuse * fresnelD @insertpiece( MulDiffuseMapValue ));
}

@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@value(CurrentShadowMap), invShadowMapSize[@counter(CurrentShadowMap)] )@end @end

void main()
{
@property( detail_maps_diffuse || detail_maps_normals )
	@property( detail_weight_map )
		lowp vec4 detailWeights = texture2D( texDetailWeightMap, psUv@value(uv_detail_weight).xy );
	@end @property( !detail_weight_map )
		lowp vec4 detailWeights = vec4( 1.0 );
	@end
	//Group all texture loads together to help the GPU hide the
	//latency (bad GL ES2 drivers won't optimize this automatically)
@end

@foreach( detail_maps_diffuse, n )
	lowp vec4 detailCol@n	= texture2D( texDetailMap[@n], psUv@value(uv_detail@n).xy );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol.xyz = detailCol.xyz * detailCol.xyz;@end
	detailWeights.@insertpiece(detail_diffuse_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_diffuse_swizzle@n);
@end
@foreach( detail_maps_normal, n )
	mediump vec3 vDetail@n	= getTSNormal( texDetailNormalMap[@n], psUv@value(uv_detail_nm@n).xy ) * detailWeights.@insertpiece(detail_normal_swizzle@n);@end

@property( !normal_map )
	nNormal = normalize( psNormal );
@end @property( normal_map )
	mediump vec3 geomNormal = normalize( psNormal );
	vTangent = normalize( psTangent );

	//Get the TBN matrix
	mediump vec3 vBinormal	= cross( vTangent, geomNormal );
	mediump mat3 TBN		= transpose( mat3( vTangent, vBinormal, geomNormal ) );

	nNormal = getTSNormal( texNormalMap, psUv@value(uv_normal).xy );
@end

@property( hlms_pssm_splits )
	lowp float fShadow = 1.0;
	if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL0, invShadowMapSize[@counter(CurrentShadowMap)] );@end
@foreach( hlms_pssm_splits, n, 1 )	else if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@n, invShadowMapSize[@counter(CurrentShadowMap)] );
@end

@insertpiece( SampleDiffuseMap )
@insertpiece( SampleSpecularMap )

@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @end

@foreach( detail_maps_normal, n )
	nNormal.xy	+= vDetail@n.xy;
	nNormal.z	*= vDetail@n.z + 1.0 - detailWeights.@insertpiece(detail_normal_swizzle@n);@end

@property( normal_map )
	nNormal = normalize( mul( TBN, nNormal ) );
@end

	//Everything's in Camera space, we use Cook-Torrance lighting
@property( hlms_lights_spot || envprobe_map )
	mediump vec3 viewDir	= normalize( -psPos );
	lowp float NdotV		= clamp( dot( nNormal, viewDir ), 0.0, 1.0 );@end

	mediump vec3 finalColour = vec3(0);
@property( hlms_lights_directional )
	finalColour += cookTorrance( lightPosition[0], viewDir, NdotV, lightDiffuse[0], lightSpecular[0] );
@property( hlms_num_shadow_maps )	finalColour *= fShadow;	//1st directional light's shadow@end
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += cookTorrance( lightPosition[@n], viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );@end

@property( hlms_lights_point || hlms_lights_spot )	mediump vec3 lightDir;
	mediump float fDistance;
	mediump vec3 tmpColour;
	mediump float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional )
	lightDir = lightPosition[@n] - psPos;
	fDistance= length( lightDir );
	if( fDistance <= attenuation[@value(atten)].x )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		mediump float atten = 1.0 / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
		finalColour += tmpColour * atten;
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
	lightDir = lightPosition[@n] - psPos;
	fDistance= length( lightDir );
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n] ), spotDirection[@value(spot_params)] );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n] ), zAxis( spotQuaternion[@value(spot_params)] ) );@end
	if( fDistance <= attenuation[@value(atten)].x && spotCosAngle >= spotParams[@value(spot_params)].y )
	{
		lightDir *= 1.0 / fDistance;
	@property( hlms_lights_spot_textured )
		mediump vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], psPos );
		lowp float spotAtten = texture2D( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		mediump float spotAtten = clamp( (spotCosAngle - spotParams[@value(spot_params)].y) * spotParams[@value(spot_params)].x, 0.0, 1.0 );
		spotAtten = pow( spotAtten, spotParams[@counter(spot_params)].z );
	@end
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		mediump float atten = 1.0 / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@property( envprobe_map )
	mediump vec3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;
	mediump vec3 envColour = texture2D( texEnvProbeMap, invViewMat * reflDir, ROUGHNESS * 12.0 ).xyz;
	envColour = envColour * envColour; //TODO: Cubemap Gamma correction broken in GL3+
	finalColour += cookTorrance( reflDir, viewDir, NdotV, envColour, envColour * (ROUGHNESS * ROUGHNESS) );@end

@property( !hw_gamma_write )
	//Linear to Gamma space
	outColour.xyz	= sqrt( finalColour );
@end @property( hw_gamma_write )
	outColour.xyz	= finalColour;
@end
	outColour.w		= 1.0;
}
@end
@property( hlms_shadowcaster )
in highp float psDepth;
layout(location = FRAG_COLOR, index = 0) out float outColour;
void main()
{
@property( GL3+ )
	outColour = psDepth;@end
@property( !GL3+ )
	gl_FragColor.x = psDepth;@end
}
@end
