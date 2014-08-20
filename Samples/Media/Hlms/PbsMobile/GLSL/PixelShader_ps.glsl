@property( GL3+ )#version 330@end
@property( !GL3+ )#define in varying@end
/*#ifdef GL_ES
precision _mediump float;
#endif*/
#define FRAG_COLOR		0

@property( GL3+ )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
	@piece( texture2D )texture@end
	@piece( textureCube )texture@end
	@piece( textureCubeLod )textureLod@end

	#define _lowp		highp
	#define _mediump	highp
@end
@property( !GL3+ )
#define outColour gl_FragColor
	@property( hlms_high_quality )
		#define _lowp		highp
		#define _mediump	highp
		precision highp float;
	@end @property( !hlms_high_quality )
		#define _lowp		lowp
		#define _mediump	mediump
		precision mediump float;
	@end

	@piece( texture2D )texture2D@end
	@piece( textureCube )textureCube@end
	@piece( textureCubeLod )textureCube@end
@end

@property( !hlms_shadowcaster )
@property( hlms_normal )
in _mediump vec3 psPos;
in _mediump vec3 psNormal;
_mediump vec3 nNormal;
@property( normal_map )in _mediump vec3 psTangent;
_mediump vec3 vTangent;@end
@end
@property( hlms_pssm_splits )in highp float psDepth;@end
@foreach( hlms_uv_count, n )
in _mediump vec@value( hlms_uv_count@n ) psUv@n;@end
@foreach( hlms_num_shadow_maps, n )
in highp vec4 psPosL@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per pass
@property( hlms_num_shadow_maps )uniform _mediump vec2 invShadowMapSize[@value(hlms_num_shadow_maps)];
uniform _mediump float pssmSplitPoints[@value(hlms_pssm_splits)];@end
@property( hlms_lights_spot )uniform _mediump vec3 lightPosition[@value(hlms_lights_spot)];
uniform _mediump vec3 lightDiffuse[@value(hlms_lights_spot)];
uniform _mediump vec3 lightSpecular[@value(hlms_lights_spot)];
@property(hlms_lights_attenuation)uniform _mediump vec3 attenuation[@value(hlms_lights_attenuation)];@end
@property(hlms_lights_spotparams)uniform _mediump vec3 spotDirection[@value(hlms_lights_spotparams)];
uniform _mediump vec3 spotParams[@value(hlms_lights_spotparams)];@end
@property( envprobe_map )uniform highp mat3 invViewMatCubemap; //This uniform depends on each renderable, but is the same for every pass@end
@end

//Uniforms that change per entity
uniform _lowp float roughness;
/* kD is already divided by PI to make it energy conserving.
  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
*/
uniform _lowp vec3 kD;
uniform _lowp vec3 kS;
@property( fresnel_scalar )@piece( FresnelType )_lowp vec3@end@end
@property( !fresnel_scalar ) @piece( FresnelType )_lowp float@end @end
//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
uniform @insertpiece( FresnelType ) F0;
@property( uv_atlas )uniform _mediump vec3 atlasOffsets[@value( uv_atlas )];@end
@property( normal_weight )uniform _lowp float normalWeights[@value( normal_weight )];@end
@property( detail_weights )uniform _lowp vec4 cDetailWeights;@end
@property( detail_offsetsD )uniform _mediump vec4 detailOffsetScaleD[@value( detail_offsetsD )];@end
@property( detail_offsetsN )uniform _mediump vec4 detailOffsetScaleN[@value( detail_offsetsN )];@end
// END UNIFORM DECLARATION

@property( detail_offsetsD )
	@foreach( detail_maps_diffuse, n )
		@property( detail_offsetsD@n )
			@piece( offsetDetailD@n ) * detailOffsetScaleD[@value(currOffsetDetailD)].zw + detailOffsetScaleD[@counter(currOffsetDetailD)].xy@end
		@end
	@end
@end
@property( detail_offsetsN )
	@foreach( detail_maps_normal, n )
		@property( detail_offsetsN@n )
			@piece( offsetDetailN@n ) * detailOffsetScaleN[@value(currOffsetDetailN)].zw + detailOffsetScaleN[@counter(currOffsetDetailN)].xy@end
		@end
	@end
@end

@property( !roughness_map )#define ROUGHNESS roughness@end
@property( diffuse_map )uniform _lowp sampler2D	texDiffuseMap;@end
@property( normal_map_tex )uniform _lowp sampler2D	texNormalMap;@end
@property( specular_map )uniform _lowp sampler2D	texSpecularMap;@end
@property( roughness_map )uniform _lowp sampler2D	texRoughnessMap;@end
@property( envprobe_map )
@property( !hlms_cube_arrays_supported )uniform _lowp samplerCube	texEnvProbeMap;@end
@property( !hlms_cube_arrays_supported && false )uniform _lowp sampler2D	texEnvProbeMap;@end @end
@property( detail_weight_map )uniform _lowp sampler2D	texDetailWeightMap;@end
@property( detail_maps_diffuse )uniform _lowp sampler2D	texDetailMap[@value( detail_maps_diffuse )];@end
@property( detail_maps_normal )uniform _lowp sampler2D	texDetailNormalMap[@value( detail_maps_normal )];@end

@property( !diffuse_map && detail_maps_diffuse )_lowp vec4 diffuseCol;@end
@property( diffuse_map )_lowp vec4 diffuseCol;
@piece( SampleDiffuseMap )	diffuseCol = @insertpiece(texture2D)( texDiffuseMap, psUv@value(uv_diffuse).xy * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy );
@property( !hw_gamma_read )	//Gamma to linear space
	diffuseCol = diffuseCol * diffuseCol;@end @end
@piece( MulDiffuseMapValue )* diffuseCol.xyz@end@end
@property( specular_map )_lowp vec3 specularCol;
@piece( SampleSpecularMap )	specularCol = @insertpiece(texture2D)( texSpecularMap, psUv@value(uv_specular).xy * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).xyz;@end
@piece( MulSpecularMapValue )* specularCol@end@end
@property( roughness_map )_lowp float ROUGHNESS;
@piece( SampleRoughnessMap )ROUGHNESS = roughness * @insertpiece(texture2D)( texRoughnessMap, psUv@value(uv_roughness).xy * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).x;@end
@end

@property( hlms_num_shadow_maps )
@property( hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2DShadow@end @end
@property( !hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2D@end @end
uniform highp @insertpiece( SAMPLER2DSHADOW ) texShadowMap[@value(hlms_num_shadow_maps)];

_lowp float getShadow( highp @insertpiece( SAMPLER2DSHADOW ) shadowMap, highp vec4 psPosLN, _mediump vec2 invShadowMapSize )
{
@property( !hlms_shadow_usues_depth_texture )
	highp float fDepth = psPosLN.z;
	_mediump vec2 uv = psPosLN.xy / psPosLN.w;
	highp vec3 o = vec3( invShadowMapSize, -invShadowMapSize.x ) * 0.3;

	// 2x2 PCF
	highp float c =	(fDepth <= @insertpiece(texture2D)(shadowMap, uv - o.xy).r) ? 1 : 0; // top left
	c +=		(fDepth <= @insertpiece(texture2D)(shadowMap, uv - o.zy).r) ? 1 : 0; // top right
	c +=		(fDepth <= @insertpiece(texture2D)(shadowMap, uv + o.zy).r) ? 1 : 0; // bottom left
	c +=		(fDepth <= @insertpiece(texture2D)(shadowMap, uv + o.xy).r) ? 1 : 0; // bottom right

	return c * 0.25;@end
@property( hlms_shadow_usues_depth_texture )
	return @insertpiece(texture2D)( shadowMap, psPosLN.xyz, 0 ).x;@end
}
@end

@property( hlms_lights_spot_textured )_mediump vec3 zAxis( _mediump vec4 qQuat )
{
	Real fTy  = 2.0 * qQuat.y;
	Real fTz  = 2.0 * qQuat.z;
	Real fTwy = fTy * qQuat.w;
	Real fTwz = fTz * qQuat.w;
	Real fTxy = fTy * qQuat.x;
	Real fTxz = fTz * qQuat.x;
	Real fTyy = fTy * qQuat.y;
	Real fTzz = fTz * qQuat.z;

	return _mediump vec3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}
_mediump vec3 qmul( _mediump vec4 q, _mediump vec3 v )
{
	return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map_tex )_mediump vec3 getTSNormal( _lowp sampler2D normalMap, _lowp vec2 uv )
{
	_mediump vec3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = @insertpiece(texture2D)( normalMap, uv * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = @insertpiece(texture2D)( normalMap, uv  * atlasOffsets[@value(atlas)].z + atlasOffsets[@counter(atlas)].xy ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y );

	return tsNormal;
}
@end
@property( detail_maps_normal )_mediump vec3 getTSDetailNormal( _lowp sampler2D normalMap, _lowp vec2 uv )
{
	_mediump vec3 tsNormal;
@property( signed_int_textures )
	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = @insertpiece(texture2D)( normalMap, uv ).xy;
@end @property( !signed_int_textures )
	//Normal texture must be in LA format!
	tsNormal.xy = @insertpiece(texture2D)( normalMap, uv ).xw * 2.0 - 1.0;
@end
	tsNormal.z	= sqrt( 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y );

	return tsNormal;
}

	@property( normal_weight_tex )
		@set( curr_normal_weightA, 1 )
		@set( curr_normal_weightB, 1 )
	@end
	@foreach( 4, n )
		@property( normal_weight_detail@n )
			@piece( detail@n_nm_weight_mulA ) * normalWeights[@counter(curr_normal_weightA)]@end
			@piece( detail@n_nm_weight_mulB ) * normalWeights[@counter(curr_normal_weightB)]@end
		@end
	@end
@end

_mediump vec3 cookTorrance( _mediump vec3 lightDir, _mediump vec3 viewDir, _lowp float NdotV, _mediump vec3 lightDiffuse, _mediump vec3 lightSpecular )
{
	_mediump vec3 halfWay= normalize( lightDir + viewDir );
	_lowp float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	_lowp float NdotH = clamp( dot( nNormal, halfWay ), 0.001, 1.0 );
	_lowp float VdotH = clamp( dot( viewDir, halfWay ), 0.001, 1.0 );

	_lowp float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness term (Beckmann distribution)
	//Formula:
	//	Where alpha = NdotH and m = roughness
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( -tan(alpha)^2 / m^2 ) ]
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( ( cos(alpha)^2 - 1 )  /  (m^2 cos(alpha)^2 ) ]
	_mediump float NdotH_sq = NdotH * NdotH;
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
@property( detail_maps_diffuse || detail_maps_normal )
	@property( detail_weight_map )
		_lowp vec4 detailWeights = @insertpiece(texture2D)( texDetailWeightMap, psUv@value(uv_detail_weight).xy );
		@property( detail_weights )detailWeights *= cDetailWeights;@end
	@end @property( !detail_weight_map )
		@property( detail_weights )_lowp vec4 detailWeights = cDetailWeights;@end
		@property( !detail_weights )_lowp vec4 detailWeights = vec4( 1.0 );@end
	@end
	//Group all texture loads together to help the GPU hide the
	//latency (bad GL ES2 drivers won't optimize this automatically)
@end

@foreach( detail_maps_diffuse, n )
	_lowp vec4 detailCol@n	= @insertpiece(texture2D)( texDetailMap[@n], psUv@value(uv_detail@n).xy@insertpiece( offsetDetailD@n ) );
	@property( !hw_gamma_read )//Gamma to linear space
		detailCol.xyz = detailCol.xyz * detailCol.xyz;@end
	detailWeights.@insertpiece(detail_diffuse_swizzle@n) *= detailCol@n.w;
	detailCol@n.w = detailWeights.@insertpiece(detail_diffuse_swizzle@n);
@end
@foreach( detail_maps_normal, n )
	_mediump vec3 vDetail@n	= getTSDetailNormal( texDetailNormalMap[@n], psUv@value(uv_detail_nm@n).xy@insertpiece( offsetDetailN@n ) ) * detailWeights.@insertpiece(detail_normal_swizzle@n) @insertpiece( detail@n_nm_weight_mulA );@end

@property( !normal_map )
	nNormal = normalize( psNormal );
@end @property( normal_map )
	_mediump vec3 geomNormal = normalize( psNormal );
	vTangent = normalize( psTangent );

	//Get the TBN matrix
	_mediump vec3 vBinormal	= cross( vTangent, geomNormal );
	_mediump mat3 TBN		= mat3( vTangent, vBinormal, geomNormal );

	@property( normal_map_tex )nNormal = getTSNormal( texNormalMap, psUv@value(uv_normal).xy );@end
	@property( normal_weight_tex )nNormal = mix( vec3( 0.0, 0.0, 1.0 ), nNormal, normalWeights[0] );@end
@end

@property( hlms_pssm_splits )
	_lowp float fShadow = 1.0;
	if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL0, invShadowMapSize[@counter(CurrentShadowMap)] );@end
@foreach( hlms_pssm_splits, n, 1 )	else if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@n, invShadowMapSize[@counter(CurrentShadowMap)] );
@end

@insertpiece( SampleDiffuseMap )
@insertpiece( SampleSpecularMap )
@insertpiece( SampleRoughnessMap )

	@property( !diffuse_map && detail_maps_diffuse )diffuseCol = vec4( 0.0, 0.0, 0.0, 0.0 );@end

@foreach( detail_maps_diffuse, n )
	@insertpiece( blend_mode_idx@n ) @end

@property( normal_map_tex )
	@piece( detail0_nm_op_sum )+=@end
	@piece( detail0_nm_op_mul )*=@end
@end @property( !normal_map_tex )
	@piece( detail0_nm_op_sum )=@end
	@piece( detail0_nm_op_mul )=@end
@end

@property( detail_maps_normal )
	nNormal.xy	@insertpiece( detail0_nm_op_sum ) vDetail0.xy;
	nNormal.z	@insertpiece( detail0_nm_op_mul ) vDetail0.z + 1.0 - detailWeights.@insertpiece(detail_normal_swizzle0) @insertpiece( detail0_nm_weight_mulB );@end
@foreach( detail_maps_normal, n, 1 )
	nNormal.xy	+= vDetail@n.xy;
	nNormal.z	*= vDetail@n.z + 1.0 - detailWeights.@insertpiece(detail_normal_swizzle@n) @insertpiece( detail@n_nm_weight_mulB );@end

@property( normal_map )
	nNormal = normalize( TBN * nNormal );
@end

	//Everything's in Camera space, we use Cook-Torrance lighting
@property( hlms_lights_spot || envprobe_map )
	_mediump vec3 viewDir	= normalize( -psPos );
	_lowp float NdotV		= clamp( dot( nNormal, viewDir ), 0.0, 1.0 );@end

	_mediump vec3 finalColour = vec3(0);
@property( hlms_lights_directional )
	finalColour += cookTorrance( lightPosition[0], viewDir, NdotV, lightDiffuse[0], lightSpecular[0] );
@property( hlms_num_shadow_maps )	finalColour *= fShadow;	//1st directional light's shadow@end
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += cookTorrance( lightPosition[@n], viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );@end

@property( hlms_lights_point || hlms_lights_spot )	_mediump vec3 lightDir;
	_mediump float fDistance;
	_mediump vec3 tmpColour;
	_mediump float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional )
	lightDir = lightPosition[@n] - psPos;
	fDistance= length( lightDir );
	if( fDistance <= attenuation[@value(atten)].x )
	{
		lightDir *= 1.0 / fDistance;
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		_mediump float atten = 1.0 / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
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
		_mediump vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], psPos );
		_lowp float spotAtten = @insertpiece(texture2D)( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		_mediump float spotAtten = clamp( (spotCosAngle - spotParams[@value(spot_params)].y) * spotParams[@value(spot_params)].x, 0.0, 1.0 );
		spotAtten = pow( spotAtten, spotParams[@counter(spot_params)].z );
	@end
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		_mediump float atten = 1.0 / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@property( envprobe_map )
	_mediump vec3 reflDir = 2.0 * dot( viewDir, nNormal ) * nNormal - viewDir;
	_mediump vec3 envColour = @insertpiece(textureCubeLod)( texEnvProbeMap, invViewMatCubemap * reflDir, ROUGHNESS * 12.0 ).xyz;
	@property( !hw_gamma_read )//Gamma to linear space
	envColour = envColour * envColour;@end
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
void main()
{
@property( GL3+ )
	outColour = psDepth;@end
@property( !GL3+ )
	gl_FragColor.x = psDepth;@end
}
@end
