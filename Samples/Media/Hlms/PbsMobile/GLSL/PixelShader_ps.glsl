#version 330
/*#ifdef GL_ES
precision mediump float;
#endif*/
#define FRAG_COLOR		0
@property( !hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

@property( hlms_normal )
in vec3 psPos;
in vec3 psNormal;
vec3 nNormal;
@property( normal_map )in vec3 psTangent;
vec3 nTangent;@end
@end
@property( hlms_pssm_splits )in float psDepth;@end
@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) psUv@n;@end
@foreach( hlms_num_shadow_maps, n )
in vec4 psPosL@n;@end

// START UNIFORM DECLARATION
//Uniforms that change per pass
@property( hlms_num_shadow_maps )uniform vec2 invShadowMapSize[@value(hlms_num_shadow_maps)];
uniform float pssmSplitPoints[@value(hlms_pssm_splits)];@end
@property( hlms_lights_spot )uniform vec3 lightPosition[@value(hlms_lights_spot)];
uniform vec3 lightDiffuse[@value(hlms_lights_spot)];
uniform vec3 lightSpecular[@value(hlms_lights_spot)];
@property(hlms_lights_attenuation)uniform vec3 attenuation[@value(hlms_lights_attenuation)];@end
@property(hlms_lights_spotparams)uniform vec3 spotDirection[@value(hlms_lights_spotparams)];
uniform vec3 spotParams[@value(hlms_lights_spotparams)];@end
@property( envprobe_map )uniform mat3 invViewMat; //This uniform depends on each renderable, but is the same for every pass@end
@end

//Uniforms that change per entity
uniform float roughness;
/* kD is already divided by PI to make it energy conserving.
  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
*/
uniform vec3 kD;
uniform vec3 kS;
@property( hlms_fresnel_scalar )@piece( FresnelType )vec3@end@end
@property( !hlms_fresnel_scalar ) @piece( FresnelType )float@end @end
//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
uniform @insertpiece( FresnelType ) F0;
// END UNIFORM DECLARATION

@property( !specular_map )#define ROUGHNESS roughness@end
@property( diffuse_map )uniform sampler2DArray	texDiffuseMap;@end
@property( normal_map )uniform sampler2DArray	texNormalMap;@end
@property( specular_map )uniform sampler2DArray	texSpecularMap;@end
@property( envprobe_map )
@property( hlms_cube_arrays_supported ) uniform samplerCubeArray	texEnvProbeMap;@end
@property( !hlms_cube_arrays_supported ) uniform sampler2DArray	texEnvProbeMap;@end @end

@property( diffuse_map )vec4 diffuseCol;
@piece( SampleDiffuseMap )	diffuseCol = texture( texDiffuseMap, psUv0 );@end
@piece( MulDiffuseMapValue )* diffuseCol.xyz@end@end
@property( specular_map )vec4 specularCol;
float ROUGHNESS;
@piece( SampleSpecularMap )	specularCol = texture( texSpecularMap, psUv0 );
	ROUGHNESS = roughness * specularCol.w;@end
@piece( MulSpecularMapValue )* specularCol.xyz@end@end

@property( hlms_num_shadow_maps )
@property( hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2DShadow@end @end
@property( !hlms_shadow_uses_depth_texture )@piece( SAMPLER2DSHADOW )sampler2D@end @end
uniform @insertpiece( SAMPLER2DSHADOW ) texShadowMap[@value(hlms_num_shadow_maps)];

float getShadow( @insertpiece( SAMPLER2DSHADOW ) shadowMap, vec4 psPosLN, vec2 invShadowMapSize )
{
@property( !hlms_shadow_usues_depth_texture )
	const float fDepth = psPosLN.z;
	const vec2 uv = psPosLN.xy / psPosLN.w;
	const vec3 o = vec3( invShadowMapSize, -invShadowMapSize.x ) * 0.3;

	// 2x2 PCF
	float c =	(fDepth <= textureLod(shadowMap, uv - o.xy, 0).r) ? 1 : 0; // top left
	c +=		(fDepth <= textureLod(shadowMap, uv - o.zy, 0).r) ? 1 : 0; // top right
	c +=		(fDepth <= textureLod(shadowMap, uv + o.zy, 0).r) ? 1 : 0; // bottom left
	c +=		(fDepth <= textureLod(shadowMap, uv + o.xy, 0).r) ? 1 : 0; // bottom right

	return c * 0.25;@end
@property( hlms_shadow_usues_depth_texture )
	return textureLod( shadowMap, psPosLN.xyz, 0 ).x;@end
}
@end

@property( hlms_lights_spot_textured )vec3 zAxis( vec4 qQuat )
{
	Real fTy  = 2.0f * qQuat.y;
	Real fTz  = 2.0f * qQuat.z;
	Real fTwy = fTy * qQuat.w;
	Real fTwz = fTz * qQuat.w;
	Real fTxy = fTy * qQuat.x;
	Real fTxz = fTz * qQuat.x;
	Real fTyy = fTy * qQuat.y;
	Real fTzz = fTz * qQuat.z;

	return vec3( 1.0f-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}
vec3 qmul( vec4 q, vec3 v )
{
	return v + 2.0f * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}
@end

@property( normal_map )vec3 getNormalMap( vec3 vNormal, vec3 vTangent, sampler2D normalMap, vec2 uv0 )
{
	//Get the TBN matrix
	vec3 vBinormal	= cross( vTangent, vNormal );
	mat3 TBN = transpose( mat3( vTangent, vBinormal, vNormal ) );

	//Normal texture must be in U8V8 format!
	vec3 tsNormal;
	tsNormal.xy = texture( normalMap, uv0 ).xy;
	tsNormal.z	= sqrt( 1.0f - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return normalize( mul( TBN, tsNormal ) );
}
@end

vec3 cookTorrance( vec3 lightDir, vec3 viewDir, float NdotV, vec3 lightDiffuse, vec3 lightSpecular )
{
	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0f, 1.0f );
	float NdotH = clamp( dot( nNormal, halfWay ), 0.0f, 1.0f );
	float VdotH = clamp( dot( viewDir, halfWay ), 0.001f, 1.0f );

	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness term (Beckmann distribution)
	//Formula:
	//	Where alpha = NdotH and m = roughness
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( -tan(alpha)^2 / m^2 ) ]
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( ( cos(alpha)^2 - 1 )  /  (m^2 cos(alpha)^2 ) ]
	float NdotH_sq = NdotH * NdotH;
	float roughness_a = 1.0f / ( 3.141592654f * sqR * NdotH_sq * NdotH_sq );//( 1 / (m^2 x cos(alpha)^4 )
	float roughness_b = NdotH_sq - 1.0f;	//( cos(alpha)^2 - 1 )
	float roughness_c = sqR * NdotH_sq;		//( m^2 cos(alpha)^2 )

	//Avoid Inf * 0 = NaN; we need Inf * 0 = 0
	float R = min( roughness_a, 65504.0f ) * exp( roughness_b / roughness_c );

	//Geometric term
	float shared_geo = 2.0f * NdotH / VdotH;
	float geo_b	= shared_geo * NdotV;
	float geo_c	= shared_geo * NdotL;
	float G	 	= min( 1.0f, min( geo_b, geo_c ) );

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	//	fresnelD = lerp( (1 - N*L)^5, 1, 1 - F0 )
	@insertpiece( FresnelType ) fresnelS = F0 + pow( 1.0f - VdotH, 5.0f ) * (1.0f - F0);
	@insertpiece( FresnelType ) fresnelD = 1.0f - F0 + pow( 1.0f - NdotL, 5.0f ) * F0;

	//Avoid very small denominators, they go to NaN or cause aliasing artifacts
	@insertpiece( FresnelType ) Rs = ( fresnelS * (R * G)  ) / max( 4.0f * NdotV * NdotL, 0.01f );

	return NdotL * (kS * lightSpecular * Rs @insertpiece( MulSpecularMapValue ) +
					kD * lightDiffuse * fresnelD @insertpiece( MulDiffuseMapValue ));
}

@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@value(CurrentShadowMap), invShadowMapSize[@counter(CurrentShadowMap)] )@end @end

void main()
{
	nNormal = normalize( psNormal );
@property( normal_map )
	nTangent = normalize( psTangent );
	nNormal = getNormalMap( nNormal, nTangent, texNormalMap, psUv0 );
@end

@property( hlms_pssm_splits )
	float fShadow = 1.0f;
	if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL0, invShadowMapSize[@counter(CurrentShadowMap)] );@end
@foreach( hlms_pssm_splits, n, 1 )	else if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@n, invShadowMapSize[@counter(CurrentShadowMap)] );
@end

@insertpiece( SampleDiffuseMap )
@insertpiece( SampleSpecularMap )

	//Everything's in Camera space, we use Cook-Torrance lighting
@property( hlms_lights_spot || envprobe_map )
	vec3 viewDir	= normalize( -psPos );
	float NdotV		= clamp( dot( nNormal, viewDir ), 0.0f, 1.0f );@end

	vec3 finalColour = vec3(0);
@property( hlms_lights_directional )
	finalColour += cookTorrance( lightPosition[0], viewDir, NdotV, lightDiffuse[0], lightSpecular[0] );
@property( hlms_num_shadow_maps )	finalColour *= fShadow;	//1st directional light's shadow@end
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += cookTorrance( lightPosition[@n], viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );@end

@property( hlms_lights_point || hlms_lights_spot )	vec3 lightDir;
	float fDistance;
	vec3 tmpColour;
	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional )
	lightDir = lightPosition[@n] - psPos;
	fDistance= length( lightDir );
	if( fDistance <= attenuation[@value(atten)].x )
	{
		lightDir *= 1.0f / fDistance;
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		float atten = 1.0f / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
		finalColour += tmpColour * atten;
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = 1.0f / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
	lightDir = lightPosition[@n] - psPos;
	fDistance= length( lightDir );
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n] ), spotDirection[@value(spot_params)] );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n] ), zAxis( spotQuaternion[@value(spot_params)] ) );@end
	if( fDistance <= attenuation[@value(atten)].x && spotCosAngle >= spotParams[@value(spot_params)].y )
	{
		lightDir *= 1.0f / fDistance;
	@property( hlms_lights_spot_textured )
		vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], psPos );
		float spotAtten = textureLod( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = clamp( (spotCosAngle - spotParams[@value(spot_params)].y) * spotParams[@value(spot_params)].x, 0.0f, 1.0f );
		spotAtten = pow( spotAtten, spotParams[@counter(spot_params)].z );
	@end
		tmpColour = cookTorrance( lightDir, viewDir, NdotV, lightDiffuse[@n], lightSpecular[@n] )@insertpiece( DarkenWithShadow );
		float atten = 1.0f / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * fDistance * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}@end

@property( envprobe_map )
	vec3 reflDir = 2.0f * dot( viewDir, nNormal ) * nNormal - viewDir;
	vec3 envColour = textureLod( texEnvProbeMap, invViewMat * reflDir, ROUGHNESS * 12.0f ).xyz;
	envColour = envColour * envColour; //TODO: Cubemap Gamma correction broken in GL3+
	finalColour += cookTorrance( reflDir, viewDir, NdotV, envColour, envColour * (ROUGHNESS * ROUGHNESS) );@end

	outColour.xyz	= finalColour;
	outColour.w		= 1.0f;
}
@end
@property( hlms_shadowcaster )
in float psDepth;
layout(location = FRAG_COLOR, index = 0) out float outColour;
void main()
{
	outColour = psDepth;
}
@end
