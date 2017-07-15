@property( !metallic_workflow && (!specular_map || !fresnel_workflow) )
	@property( !transparent_mode )
		@piece( F0 )material.F0@end
	@end @property( transparent_mode )
		//Premultiply F0.xyz with the alpha from the texture, but only in transparent mode.
		@piece( F0 )(material.F0.@insertpiece( FresnelSwizzle ) * diffuseCol.w)@end
	@end
@end @property( metallic_workflow || (specular_map && fresnel_workflow) )
	@piece( F0 )F0@end
@end

@property( !fresnel_scalar )
	@piece( maxR1F0 )max( 1.0 - ROUGHNESS, @insertpiece( F0 ).x )@end
@end @property( fresnel_scalar )
	@piece( maxR1F0 )max( (1.0 - ROUGHNESS).xxx, @insertpiece( F0 ).xyz )@end
@end

//For mortals:
//	getSpecularFresnel	= F0 + pow( 1.0 - VdotH, 5.0 ) * (1.0 - F0)
//	getDiffuseFresnel	= 1.0 - F0 + pow( 1.0 - NdotL, 5.0 ) * F0
//	getSpecularFresnelWithRoughness = F0 + pow( 1.0 - VdotH, 5.0 ) * (max(ROUGHNESS, (1.0 - F0)) - F0)
//	getDiffuseFresnelWithRoughness = max(ROUGHNESS, (1.0 - F0) - F0 + pow( 1.0 - NdotL, 5.0 ) * F0
@piece( getSpecularFresnel )@insertpiece( F0 ).@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (1.0 - @insertpiece( F0 ).@insertpiece( FresnelSwizzle ))@end
@piece( getDiffuseFresnel )1.0 - @insertpiece( F0 ).@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * @insertpiece( F0 ).@insertpiece( FresnelSwizzle )@end

@piece( getSpecularFresnelWithRoughness )@insertpiece( F0 ).@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (@insertpiece( maxR1F0 ) - @insertpiece( F0 ).@insertpiece( FresnelSwizzle ))@end
@piece( getDiffuseFresnelWithRoughness )@insertpiece( maxR1F0 ) - @insertpiece( F0 ).@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * @insertpiece( F0 ).@insertpiece( FresnelSwizzle )@end

@property( !fresnel_scalar )
	@piece( getMaxFresnelS )fresnelS@end
@end @property( fresnel_scalar )
	@property(  hlms_amd_trinary_minmax )@piece( getMaxFresnelS )max3( fresnelS.x, fresnelS.y, fresnelS.z )@end @end
	@property( !hlms_amd_trinary_minmax )@piece( getMaxFresnelS )max( fresnelS.x, max( fresnelS.y, fresnelS.z ) )@end @end
@end

@property( BRDF_BlinnPhong )
@piece( DeclareBRDF )
//Blinn-Phong
vec3 BRDF( vec3 lightDir, vec3 viewDir, float NdotV, vec3 lightDiffuse, vec3 lightSpecular )
{
	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );	//Diffuse (Lambert)
	float NdotH = clamp( dot( nNormal, halfWay ), 0.001, 1.0 );	//Specular
	@property( !legacy_math_brdf )
		float VdotH = clamp( dot( viewDir, halfWay ), 0.001, 1.0 ); //Fresnel

		//Fresnel term (Schlick's approximation)
		@insertpiece( FresnelType ) fresnelS = @insertpiece( getSpecularFresnel );
		@property( fresnel_separate_diffuse )
			@insertpiece( FresnelType ) fresnelD = @insertpiece( getDiffuseFresnel );
		@end @property( !fresnel_separate_diffuse )
			float fresnelD = 1.0f - @insertpiece( getMaxFresnelS );
		@end
	@end

	@property( !roughness_is_shininess )
		float shininess = exp2( 10.0 * (1.0 - ROUGHNESS) + 1.0 ) * 0.25;
	@end @property( roughness_is_shininess )
		float shininess = ROUGHNESS;
	@end
	float blinnPhong = pow( NdotH, shininess );

	@property( !legacy_math_brdf )
		//Normalize Blinn-Phong using (n + 8) / (8 * pi)
		//Note this factor is an approximation. The real normalization is
		//*much* more expensive. See:
		//http://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
		blinnPhong *= (shininess + 8.0) / (8.0 * 3.141592654);

		//Avoid very small denominators, they go to NaN or cause aliasing artifacts
		@insertpiece( FresnelType ) Rs = ( fresnelS * blinnPhong ) / max( 4.0 * NdotV * NdotL, 0.01 );
	@end @property( legacy_math_brdf )
		float Rs = blinnPhong;
		float fresnelD = 1.0;
	@end

	return NdotL * (@insertpiece( kS ).xyz * lightSpecular * Rs +
					@insertpiece( kD ).xyz * lightDiffuse * fresnelD);
}
@end
@end

@property( BRDF_CookTorrance )
@piece( DeclareBRDF )
//Cook-Torrance
vec3 BRDF( vec3 lightDir, vec3 viewDir, float NdotV, vec3 lightDiffuse, vec3 lightSpecular )
{
	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	float NdotH = clamp( dot( nNormal, halfWay ), 0.001, 1.0 );
	float VdotH = clamp( dot( viewDir, halfWay ), 0.001, 1.0 );

	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness/Distribution/NDF term (Beckmann distribution)
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

	//Geometric/Visibility term (Cook Torrance)
	float shared_geo = 2.0 * NdotH / VdotH;
	float geo_b	= shared_geo * NdotV;
	float geo_c	= shared_geo * NdotL;
	@property( !hlms_amd_trinary_minmax )float G	 	= min( 1.0, min( geo_b, geo_c ) );@end
	@property(  hlms_amd_trinary_minmax )float G	 	= min3( 1.0, geo_b, geo_c );@end

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	//	fresnelD = lerp( (1 - N*L)^5, 1, 1 - F0 ) [See s2010_course_note_practical_implementation_at_triace.pdf]
	@insertpiece( FresnelType ) fresnelS = @insertpiece( getSpecularFresnel );
@property( fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = @insertpiece( getDiffuseFresnel );
@end @property( !fresnel_separate_diffuse )
	float fresnelD = 1.0f - @insertpiece( getMaxFresnelS );@end

	//Avoid very small denominators, they go to NaN or cause aliasing artifacts
	@insertpiece( FresnelType ) Rs = ( fresnelS * (R * G)  ) / max( 4.0 * NdotV * NdotL, 0.01 );

	return NdotL * (@insertpiece( kS ).xyz * lightSpecular * Rs +
					@insertpiece( kD ).xyz * lightDiffuse * fresnelD);
}
@end
@end

@property( BRDF_Default )
@piece( DeclareBRDF )
//Default BRDF
vec3 BRDF( vec3 lightDir, vec3 viewDir, float NdotV, vec3 lightDiffuse, vec3 lightSpecular )
{
	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	float NdotH = clamp( dot( nNormal, halfWay ), 0.0, 1.0 );
	float VdotH = clamp( dot( viewDir, halfWay ), 0.0, 1.0 );

	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness/Distribution/NDF term (GGX)
	//Formula:
	//	Where alpha = roughness
	//	R = alpha^2 / [ PI * [ ( NdotH^2 * (alpha^2 - 1) ) + 1 ]^2 ]
	float f = ( NdotH * sqR - NdotH ) * NdotH + 1.0;
	float R = sqR / (f * f + 1e-6f);

	//Geometric/Visibility term (Smith GGX Height-Correlated)
@property( GGX_height_correlated )
	float Lambda_GGXV = NdotL * sqrt( (-NdotV * sqR + NdotV) * NdotV + sqR );
	float Lambda_GGXL = NdotV * sqrt( (-NdotL * sqR + NdotL) * NdotL + sqR );

	float G = 0.5 / (( Lambda_GGXV + Lambda_GGXL + 1e-6f ) * 3.141592654);
@end @property( !GGX_height_correlated )
	float gL = NdotL * (1-sqR) + sqR;
	float gV = NdotV * (1-sqR) + sqR;
	float G = 1.0 / (( gL * gV + 1e-4f ) * 4 * 3.141592654);
@end

	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	@insertpiece( FresnelType ) fresnelS = @insertpiece( getSpecularFresnel );

	//We should divide Rs by PI, but it was done inside G for performance
	vec3 Rs = ( fresnelS * (R * G) ) * @insertpiece( kS ).xyz * lightSpecular;

	//Diffuse BRDF (*Normalized* Disney, see course_notes_moving_frostbite_to_pbr.pdf
	//"Moving Frostbite to Physically Based Rendering" Sebastien Lagarde & Charles de Rousiers)
	float energyBias	= ROUGHNESS * 0.5;
	float energyFactor	= mix( 1.0, 1.0 / 1.51, ROUGHNESS );
	float fd90			= energyBias + 2.0 * VdotH * VdotH * ROUGHNESS;
	float lightScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotL, 5.0 );
	float viewScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotV, 5.0 );

@property( fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = @insertpiece( getDiffuseFresnel );
@end @property( !fresnel_separate_diffuse )
	float fresnelD = 1.0f - @insertpiece( getMaxFresnelS );@end

	//We should divide Rd by PI, but it is already included in kD
	vec3 Rd = (lightScatter * viewScatter * energyFactor * fresnelD) * @insertpiece( kD ).xyz * lightDiffuse;

	return NdotL * (Rs + Rd);
}
@end
@end

@property( hlms_enable_vpls )
@piece( DeclareBRDF_InstantRadiosity )
//Simplified cheap BRDF for Instant Radiosity.
vec3 BRDF_IR( vec3 lightDir, vec3 lightDiffuse )
{
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	@insertpiece( FresnelType ) fresnelD = @insertpiece( getDiffuseFresnel );

	//We should divide Rd by PI, but it is already included in kD
	return NdotL * fresnelD * @insertpiece( kD ).xyz * lightDiffuse;
}
@end
@end

/// Applying Fresnel term to prefiltered cubemap has a bad effect of always showing high specular
/// color at edge, even for rough surface. See https://seblagarde.wordpress.com/2011/08/17/hello-world/
/// and see http://www.ogre3d.org/forums/viewtopic.php?f=25&p=523550#p523544
/// "The same Fresnel term which is appropriate for unfiltered environment maps (i.e. perfectly smooth
/// mirror surfaces) is not appropriate for filtered environment maps since there you are averaging
/// incoming light colors from many directions, but using a single Fresnel value computed for the
///	reflection direction. The correct function has similar values as the regular Fresnel expression
/// at v=n, but at glancing angle it behaves differently. In particular, the lerp(from base specular
/// to white) does not go all the way to white at glancing angles in the case of rough surfaces."
/// So we use getSpecularFresnelWithRoughness instead.
@piece( BRDF_EnvMap )
	float NdotL = clamp( dot( nNormal, reflDir ), 0.0, 1.0 );
	float VdotH = clamp( dot( viewDir, normalize( reflDir + viewDir ) ), 0.0, 1.0 );
	@insertpiece( FresnelType ) fresnelS = @insertpiece( getSpecularFresnelWithRoughness );

	@property( fresnel_separate_diffuse )
		@insertpiece( FresnelType ) fresnelD = @insertpiece( getDiffuseFresnelWithRoughness );
	@end @property( !fresnel_separate_diffuse )
		float fresnelD = 1.0f - @insertpiece( getMaxFresnelS );@end

	finalColour += envColourD * @insertpiece( kD ).xyz * fresnelD +
					envColourS * @insertpiece( kS ).xyz * fresnelS;
@end

@property( hlms_fine_light_mask )
	@piece( DeclareObjLightMask )uint objLightMask = instance.worldMaterialIdx[inPs.drawId].z;@end
	@piece( ObjLightMaskCmp )if( (objLightMask & floatBitsToUint( passBuf.lights[@counter(fineMaskLightIdx)].position.w )) != 0u )@end
	@piece( andObjLightMaskCmp )&& ((objLightMask & floatBitsToUint( passBuf.lights[@counter(fineMaskLightIdx)].position.w )) != 0u)@end
@end
