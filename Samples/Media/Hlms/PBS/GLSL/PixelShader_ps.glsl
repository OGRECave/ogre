#version 330

/*#ifdef GL_ES
precision mediump float;
#endif*/
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

@property( hlms_normal )
in vec3 psPos;
in vec3 psNormal;
@property( normal_map )in vec3 psTangent;@end
@end
@property( hlms_pssm_splits )in float psDepth;@end
@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) psUv@n;@end
@foreach( hlms_num_shadow_maps, n )
in vec4 psPosL@n;@end

@property( hlms_lights_spot )uniform vec4 lightPosition[@value(hlms_lights_spot)];
uniform vec3 lightDiffuse[@value(hlms_lights_spot)];
uniform vec3 lightSpecular[@value(hlms_lights_spot)];
uniform vec4 attenuation[@value(hlms_lights_attenuation)];
uniform vec3 spotDirection[@value(hlms_lights_spotparams)];
uniform vec3 spotParams[@value(hlms_lights_spotparams)];
@end

@property( hlms_fresnel_scalar )@piece( FresnelType )vec3@end@end
@property( !hlms_fresnel_scalar ) @piece( FresnelType )float@end @end
@property( !specular_map )#define ROUGHNESS roughness@end

uniform float roughness;

/* kD is already divided by PI to make it energy conserving.
  (forumla is finalDiffuse = NdotL * surfaceDiffuse / PI)
*/
uniform vec3 kD;
uniform vec3 kS;

//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
uniform @insertpiece( FresnelType ) F0;

@property( diffuse_map )uniform sampler2DArray	texDiffuseMap;@end
@property( normal_map )uniform sampler2DArray	texNormalMap;@end
@property( specular_map )uniform sampler2DArray	texSpecularMap;@end
@property( envprobe_map )
@property( hlms_cube_arrays_supported ) uniform samplerCubeArray	texEnvProbeMap;@end
@property( !hlms_cube_arrays_supported ) uniform sampler2DArray	texEnvProbeMap;@end @end

@property( diffuse_map )@piece( MulDiffuseMapValue )* diffuseCol.xyz@end@end
@property( specular_map )@piece( MulSpecularMapValue )* specularCol.xyz@end@end

@property( hlms_num_shadow_maps )
@property( hlms_shadow_usues_depth_texture )#define SAMPLER2DSHADOW sampler2DShadow@end
@property( !hlms_shadow_usues_depth_texture )#define SAMPLER2DSHADOW sampler2D@end
uniform vec2 invShadowMapSize[@value(hlms_num_shadow_maps)];
uniform float pssmSplitPoints[@value(hlms_pssm_splits)];
uniform SAMPLER2DSHADOW texShadowMap[@value(hlms_num_shadow_maps)];

float getShadow( SAMPLER2DSHADOW shadowMap, vec4 psPosLN, vec2 invShadowMapSize )
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

vec3 cookTorrance( int lightIdx, vec3 viewDir, float NdotV )
{
	vec3 lightDir = (psPos - lightPosition[lightIdx].xyz) * lightPosition[lightIdx].w;

	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = dot( psNormal, lightDir );
	float NdotH = dot( psNormal, halfWay );
	float VdotH = dot( viewDir, halfWay );

@property( diffuse_map )	vec4 diffuseCol = texture( texDiffuseMap, psUv0 );@end
@property( specular_map )
	vec4 specularCol = texture( texSpecularMap, psUv0 );
	float ROUGHNESS = roughness * specularCol.w;
@end
	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness term (Beckmann distribution)
	//Formula:
	//	Where alpha = NdotH and m = roughness
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( -tan(alpha)^2 / m^2 ) ]
	//	R = [ 1 / (m^2 x cos(alpha)^4 ] x [ e^( ( cos(alpha)^2 - 1 )  /  (m^2 cos(alpha)^2 ) ]

	float roughness_a = 1.0f / ( sqR * pow( NdotH, 4.0f ) );//( 1 / (m^2 x cos(alpha)^4 )
	float roughness_b = NdotH * NdotH - 1.0f;	//( cos(alpha)^2 - 1 )
	float roughness_c = sqR * NdotH * NdotH;	//( m^2 cos(alpha)^2 )

	float R = roughness_a * exp( roughness_b / roughness_c );

	vec3 reflDir = (-2.0f * dot( viewDir, halfWay )) * halfWay + viewDir;
@property( envprobe_map )	//Should we be use R instead for selecting the mip?
	vec3 envColour = textureLod( texEnvProbeMap, reflDir, ROUGHNESS ).xyz;@end

	//Geometric term
	float shared_geo = 2.0f * NdotH / VdotH;
	float geo_b	= shared_geo * NdotV;
	float geo_c	= shared_geo * NdotL;
	float G	 	= min( 1.0f, min( geo_b, geo_c ) );

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	lerp( 1, (1 - V*H)^5, F0 )
	@insertpiece( FresnelType ) fresnel = F0 + pow( 1.0f - VdotH, 5.0f ) * (1.0f - F0);

@property( envprobe_map )	vec3 Rs = ( envColour * fresnel * R * G  ) / (4.0f * NdotV * NdotL);@end
@property( !envprobe_map )	@insertpiece( FresnelType ) Rs = ( fresnel * R * G  ) / (4.0f * NdotV * NdotL);@end

	return max(0.0f, NdotL) * (kS[lightIdx] * lightSpecular[lightIdx] * Rs @insertpiece( MulSpecularMapValue ) +
							   kD[lightIdx] * lightDiffuse[lightIdx] @insertpiece( MulDiffuseMapValue ));
}

@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@value(CurrentShadowMap), invShadowMapSize[@counter(CurrentShadowMap)] )@end @end

void main()
{
	psNormal = normalize( psNormal );
@property( normal_map )
	psTangent = normalize( psTangent );
	psNormal = getNormalMap( psNormal, psTangent, texNormalMap, psUv0 );
@end

@property( hlms_pssm_splits )
	float fShadow = 1.0f;
	if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL0, invShadowMapSize[@counter(CurrentShadowMap)] );@end
@foreach( hlms_pssm_splits, n, 1 )	else if( psDepth <= pssmSplitPoints[@value(CurrentShadowMap)] )
		fShadow = getShadow( texShadowMap[@value(CurrentShadowMap)], psPosL@n, invShadowMapSize[@counter(CurrentShadowMap)] );
@end

	//Everything's in Camera space, we use Cook-Torrance lighting
@property( !hlms_lights_point && hlms_lights_spot )	vec3 viewDir	= normalize( psPos );@end
@property( hlms_lights_point || hlms_lights_spot )
	float fDistance	= length( psPos );
	float sqDistance= fDistance * fDistance;
	vec3 viewDir	= psPos * (1.0f / fDistance);@end
	float NdotV		= dot( psNormal, viewDir );

	vec3 finalColour = vec3(0);
@property( hlms_lights_directional )
	finalColour += cookTorrance( 0, viewDir, NdotV );
@property( hlms_num_shadow_maps )	finalColour *= fShadow;	//1st directional light's shadow@end
@end
@foreach( hlms_lights_directional, n, 1 )
	finalColour += cookTorrance( @n, viewDir, NdotV )@insertpiece( DarkenWithShadow );@end

@property( hlms_lights_point || hlms_lights_spot )	vec3 tmpColour;@end
@property( hlms_lights_point || hlms_lights_spot )	float spotCosAngle;@end

	//Point lights
@foreach( hlms_lights_point, n, hlms_lights_directional )
	if( fDistance <= attenuation[@value(atten)].x )
	{
		tmpColour = cookTorrance( @n, viewDir, NdotV )@insertpiece( DarkenWithShadow );
		finalColour += tmpColour * ( 1.0f / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * sqDistance ) );
	}@end

	//Spot lights
	//spotParams[@value(spot_params)].x = cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[@value(spot_params)].y = cos( OuterAngle / 2 )
	//spotParams[@value(spot_params)].z = falloff
@foreach( hlms_lights_spot, n, hlms_lights_point )
@property( !hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n].xyz ), spotDirection[@value(spot_params)] );@end
@property( hlms_lights_spot_textured )	spotCosAngle = dot( normalize( psPos - lightPosition[@n].xyz ), zAxis( spotQuaternion[@value(spot_params)] ) );@end
	if( fDistance <= attenuation[@value(atten)].x && spotCosAngle >= spotParams[@value(spot_params)].y )
	{
	@property( hlms_lights_spot_textured )
		vec3 posInLightSpace = qmul( spotQuaternion[@value(spot_params)], psPos );
		float spotAtten = textureLod( texSpotLight, normalize( posInLightSpace ).xy ).x;
	@end
	@property( !hlms_lights_spot_textured )
		float spotAtten = (spotCosAngle - spotParams[@value(spot_params)].y) * spotParams[@value(spot_params)].x;
		spotAtten = pow( spotAtten, spotParams[@counter(spot_params)].z );
	@end
		tmpColour = cookTorrance( @n, viewDir, NdotV )@insertpiece( DarkenWithShadow ) * spotAtten;
		finalColour += tmpColour * ( 1.0f / (1.0 + attenuation[@value(atten)].y * fDistance + attenuation[@counter(atten)].z * sqDistance ) );
	}@end

	outColour.xyz	= finalColour;
	outColour.w		= 1.0f;
}
