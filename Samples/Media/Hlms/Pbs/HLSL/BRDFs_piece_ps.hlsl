@property( BRDF_CookTorrance )
@piece( DeclareBRDF )
//Cook-Torrance
float3 BRDF( float3 lightDir, float3 viewDir, float NdotV, float3 lightDiffuse, float3 lightSpecular, Material material, float3 nNormal @insertpiece( brdfExtraParamDefs ) )
{
	float3 halfWay= normalize( lightDir + viewDir );
	float NdotL = saturate( dot( nNormal, lightDir ) );
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
	float G	 	= min( 1.0, min( geo_b, geo_c ) );

	//Fresnel term (Schlick's approximation)
	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	//	fresnelD = lerp( (1 - N*L)^5, 1, 1 - F0 ) [See s2010_course_note_practical_implementation_at_triace.pdf]
	@insertpiece( FresnelType ) fresnelS = material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (1.0 - material.F0.@insertpiece( FresnelSwizzle ));
@property( fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = 1.0 - material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * material.F0.@insertpiece( FresnelSwizzle );
@end @property( !fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = 1.0 - fresnelS;@end

	//Avoid very small denominators, they go to NaN or cause aliasing artifacts
	@insertpiece( FresnelType ) Rs = ( fresnelS * (R * G)  ) / max( 4.0 * NdotV * NdotL, 0.01 );

	return NdotL * (material.kS.xyz * lightSpecular * Rs @insertpiece( MulSpecularMapValue ) +
					material.kD.xyz * lightDiffuse * fresnelD @insertpiece( MulDiffuseMapValue ));
}
@end
@end

@property( BRDF_Default )
@piece( DeclareBRDF )
//Default BRDF
float3 BRDF( float3 lightDir, float3 viewDir, float NdotV, float3 lightDiffuse, float3 lightSpecular, Material material, float3 nNormal @insertpiece( brdfExtraParamDefs ) )
{
	float3 halfWay= normalize( lightDir + viewDir );
	float NdotL = saturate( dot( nNormal, lightDir ) );
	float NdotH = saturate( dot( nNormal, halfWay ) );
	float VdotH = saturate( dot( viewDir, halfWay ) );

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
	@insertpiece( FresnelType ) fresnelS = material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (1.0 - material.F0.@insertpiece( FresnelSwizzle ));

	//We should divide Rs by PI, but it was done inside G for performance
	float3 Rs = ( fresnelS * (R * G) ) * material.kS.xyz * lightSpecular @insertpiece( MulSpecularMapValue );

	//Diffuse BRDF (*Normalized* Disney, see course_notes_moving_frostbite_to_pbr.pdf
	//"Moving Frostbite to Physically Based Rendering" Sebastien Lagarde & Charles de Rousiers)
	float energyBias	= ROUGHNESS * 0.5;
	float energyFactor	= lerp( 1.0, 1.0 / 1.51, ROUGHNESS );
	float fd90			= energyBias + 2.0 * VdotH * VdotH * ROUGHNESS;
	float lightScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotL, 5.0 );
	float viewScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotV, 5.0 );

@property( fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = 1.0 - material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * material.F0.@insertpiece( FresnelSwizzle );
@end @property( !fresnel_separate_diffuse )
	@insertpiece( FresnelType ) fresnelD = 1.0 - fresnelS;@end

	//We should divide Rd by PI, but it is already included in kD
	float3 Rd = (lightScatter * viewScatter * fresnelD) * material.kD.xyz * lightDiffuse @insertpiece( MulDiffuseMapValue );

	return NdotL * (Rs + Rd);
}
@end
@end

@piece( BRDF_EnvMap )
	float NdotL = saturate( dot( nNormal, reflDir ) );
	float VdotH = saturate( dot( viewDir, normalize( reflDir + viewDir ) ) );
	@insertpiece( FresnelType ) fresnelS = material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - VdotH, 5.0 ) * (1.0 - material.F0.@insertpiece( FresnelSwizzle ));

	@property( !fresnel_separate_diffuse )
		finalColour += lerp( envColourD * material.kD.xyz, envColourS * material.kS.xyz, fresnelS );
	@end @property( fresnel_separate_diffuse )
		@insertpiece( FresnelType ) fresnelD = 1.0 - material.F0.@insertpiece( FresnelSwizzle ) + pow( 1.0 - NdotL, 5.0 ) * material.F0.@insertpiece( FresnelSwizzle );
		finalColour += envColourD * material.kD.xyz + envColourS * material.kS.xyz * fresnelS;
	@end
@end
