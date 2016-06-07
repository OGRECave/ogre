@property( false )
@insertpiece( SetCrossPlatformSettings )

layout(std140) uniform;
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

uniform sampler2D terrainNormals;

in block
{
@insertpiece( Terra_VStoPS_block )
} inPs;

void main()
{
	outColour = vec4( inPs.uv0.xy, 0.0, 1.0 );
}

@end
@property( !false )
@insertpiece( SetCrossPlatformSettings )
@property( !GL430 )
@property( hlms_tex_gather )#extension GL_ARB_texture_gather: require@end
@end
@property( hlms_amd_trinary_minmax )#extension GL_AMD_shader_trinary_minmax: require@end

layout(std140) uniform;
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

@property( hlms_vpos )
in vec4 gl_FragCoord;
@end

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( TerraMaterialDecl )
@insertpiece( TerraInstanceDecl )
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
in block
{
@insertpiece( Terra_VStoPS_block )
} inPs;

uniform sampler2D terrainNormals;
uniform sampler2D terrainShadows;

@property( hlms_forward3d )
/*layout(binding = 1) */uniform usamplerBuffer f3dGrid;
/*layout(binding = 2) */uniform samplerBuffer f3dLightList;@end
@property( num_textures )uniform sampler2DArray textureMaps[@value( num_textures )];@end
@property( envprobe_map )uniform samplerCube	texEnvProbeMap;@end

vec4 diffuseCol;
@insertpiece( FresnelType ) F0;
float ROUGHNESS;

vec3 nNormal;

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

@insertpiece( DeclareBRDF )

@piece( DarkenWithShadowFirstLight )* fShadow@end
@property( hlms_num_shadow_maps )@piece( DarkenWithShadow ) * getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@value(CurrentShadowMap), pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize )@end @end

void main()
{
	@insertpiece( custom_ps_preExecution )

	@insertpiece( custom_ps_posMaterialLoad )

//Prepare weight map for the detail maps.
@property( detail_weight_map )
	vec4 detailWeights = texture( textureMaps[@value( detail_weight_map_idx )],
									vec3( inPs.uv0.xy, @value(detail_weight_map_idx_slice) ) );
@end @property( !detail_weight_map )
	vec4 detailWeights = vec4( 1.0, 1.0, 1.0, 1.0 );
@end

@property( diffuse_map )
	diffuseCol = texture( textureMaps[@value( diffuse_map_idx )], vec3( inPs.uv0.xy, @value(diffuse_map_idx_slice) ) );
@end

	/// Sample detail maps
@foreach( 4, n )
	@property( detail_map@n )
		vec3 detailCol@n = texture( textureMaps[@value(detail_map@n_idx)],
								vec3( inPs.uv0.xy * material.detailOffsetScale[@value(currOffsetDetail)].zw +
										material.detailOffsetScale[@value(currOffsetDetail)].xy,
										@value(detail_map@n_idx_slice) ) ).xyz;
	@end @property( !detail_map@n )
		vec3 detailCol@n = vec3( 0, 0, 0 );
	@end

	@property( metalness_map@n )
		float metalness@n = texture( textureMaps[@value( metalness_map@n_idx )],
									vec3( inPs.uv0.xy * material.detailOffsetScale[@value(currOffsetDetail)].zw +
											material.detailOffsetScale[@value(currOffsetDetail)].xy,
											@value( metalness_map@n_idx_slice ) ) ).x;
	@end @property( !metalness_map@n )
		float metalness@n = 0;
	@end

	@property( roughness_map@n )
		float roughness@n = texture( textureMaps[@value( roughness_map@n_idx )],
									vec3( inPs.uv0.xy * material.detailOffsetScale[@value(currOffsetDetail)].zw +
											material.detailOffsetScale[@value(currOffsetDetail)].xy,
											@value( roughness_map@n_idx_slice ) ) ).x;
	@end @property( !roughness_map@n )
		float roughness@n = 0;
	@end

	@add( currOffsetDetail, 1 )
@end

	float metalness =	(metalness0 * detailWeights.x * material.metalness.x +
						 metalness1 * detailWeights.y * material.metalness.y) +
						(metalness2 * detailWeights.z * material.metalness.z +
						 metalness3 * detailWeights.w * material.metalness.w);

	ROUGHNESS =			(roughness0 * detailWeights.x * material.roughness.x +
						 roughness1 * detailWeights.y * material.roughness.y) +
						(roughness2 * detailWeights.z * material.roughness.z +
						 roughness3 * detailWeights.w * material.roughness.w);
	ROUGHNESS = max( ROUGHNESS, 0.02 );

@property( diffuse_map )
	diffuseCol.xyz *=	(detailCol0 * detailWeights.x + detailCol1 * detailWeights.y) +
						(detailCol2 * detailWeights.z + detailCol3 * detailWeights.w);
@end @property( !diffuse_map )
	@property( detail_maps_diffuse )
		diffuseCol.xyz =	(detailCol0 * detailWeights.x + detailCol1 * detailWeights.y) +
							(detailCol2 * detailWeights.z + detailCol3 * detailWeights.w);
	@end @property( !detail_maps_diffuse )
		diffuseCol.xyzw = vec4( 1, 1, 1, 1 );
	@end
@end

	/// Apply the material's diffuse over the textures
	diffuseCol.xyz *= material.kD.xyz;

	//Calculate F0 from metalness, and dim kD as metalness gets bigger.
	F0 = mix( vec3( 0.03f ), @insertpiece( kD ).xyz * 3.14159f, metalness );
	@insertpiece( kD ).xyz = @insertpiece( kD ).xyz - @insertpiece( kD ).xyz * metalness;

@property( !detail_maps_normal )
	// Geometric normal
	nNormal = texture( terrainNormals, inPs.uv0.xy ).xyz * 2.0 - 1.0;
	//nNormal.xz = texture( terrainNormals, inPs.uv0.xy ).xy;
	//nNormal.y = sqrt( max( 1.0 - nNormal.x * nNormal.x - nNormal.z * nNormal.z, 0.0 ) );
	nNormal = nNormal * mat3(pass.view);
@end @property( detail_maps_normal )
	vec3 geomNormal = texture( terrainNormals, inPs.uv0.xy ).xyz * 2.0 - 1.0;
	geomNormal = geomNormal * mat3(pass.view);

	//Get the TBN matrix
	vec3 vBinormal	= normalize( cross( geomNormal, vTangent ) );
	mat3 TBN		= mat3( pass.viewSpaceTangent, vBinormal, geomNormal );
@end

	float fShadow = texture( terrainShadows, inPs.uv0.xy ).x;

@property( hlms_pssm_splits )
    if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
		fShadow *= getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@foreach( hlms_pssm_splits, n, 1 )	else if( inPs.depth <= pass.pssmSplitPoints@value(CurrentShadowMap) )
		fShadow *= getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL@n, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end @end @property( !hlms_pssm_splits && hlms_num_shadow_maps && hlms_lights_directional )
	fShadow *= getShadow( texShadowMap[@value(CurrentShadowMap)], inPs.posL0, pass.shadowRcv[@counter(CurrentShadowMap)].invShadowMapSize );
@end

	/// The first iteration must initialize nNormal instead of try to merge with it.
	/// Blend the detail normal maps with the main normal.
@foreach( second_valid_detail_map_nm, n, first_valid_detail_map_nm )
	vec3 vDetail = @insertpiece( SampleDetailMapNm@n ) * detailWeights.@insertpiece(detail_swizzle@n);
	nNormal.xy	= vDetail.xy;
	nNormal.z	= vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n);@end
@foreach( detail_maps_normal, n, second_valid_detail_map_nm )@property( detail_map_nm@n )
	vDetail = @insertpiece( SampleDetailMapNm@n ) * detailWeights.@insertpiece(detail_swizzle@n);
	nNormal.xy	+= vDetail.xy;
	nNormal.z	*= vDetail.z + 1.0 - detailWeights.@insertpiece(detail_swizzle@n);@end @end

@property( detail_maps_normal )
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
	
	@insertpiece( custom_ps_posExecution )
}
@end
