@property( hlms_forwardplus )

@pmul( skip_header_in_lighting, hlms_prepass, hlms_enable_decals )

@property( hlms_forwardplus_fine_light_mask )
	@piece( andObjLightMaskFwdPlusCmp )&& ((objLightMask & floatBitsToUint( lightDiffuse.w )) != 0u)@end
@end

@property( hlms_enable_decals )
@piece( DeclDecalsSamplers )
	@property( hlms_decals_diffuse )uniform sampler2DArray decalsDiffuseTex;@end
	@property( hlms_decals_normals )uniform sampler2DArray decalsNormalsTex;@end
	@property( hlms_decals_diffuse == hlms_decals_emissive )
		#define decalsEmissiveTex decalsDiffuseTex
	@end
	@property( hlms_decals_diffuse != hlms_decals_emissive )
		uniform sampler2DArray decalsEmissiveTex;
	@end
@end
@end

/// The header is automatically inserted. Whichever subsystem needs it first, will call it
@piece( forward3dHeader )
	@property( hlms_forwardplus_covers_entire_target )
		#define FWDPLUS_APPLY_OFFSET_Y(v) (v)
		#define FWDPLUS_APPLY_OFFSET_X(v) (v)
	@end

	@property( hlms_forwardplus_fine_light_mask && !hlms_fine_light_mask )
		uint objLightMask = instance.worldMaterialIdx[inPs.drawId].z;
	@end
	@property( hlms_forwardplus == forward3d )
		float f3dMinDistance	= passBuf.f3dData.x;
		float f3dInvMaxDistance	= passBuf.f3dData.y;
		float f3dNumSlicesSub1	= passBuf.f3dData.z;
		uint cellsPerTableOnGrid0= floatBitsToUint( passBuf.f3dData.w );

		// See C++'s Forward3D::getSliceAtDepth
		/*float fSlice = 1.0 - clamp( (-inPs.pos.z + f3dMinDistance) * f3dInvMaxDistance, 0.0, 1.0 );
		fSlice = (fSlice * fSlice) * (fSlice * fSlice);
		fSlice = (fSlice * fSlice);
		fSlice = floor( (1.0 - fSlice) * f3dNumSlicesSub1 );*/
		float fSlice = clamp( (-inPs.pos.z + f3dMinDistance) * f3dInvMaxDistance, 0.0, 1.0 );
		fSlice = floor( fSlice * f3dNumSlicesSub1 );
		uint slice = uint( fSlice );

		//TODO: Profile performance: derive this mathematically or use a lookup table?
		uint offset = cellsPerTableOnGrid0 * (((1u << (slice << 1u)) - 1u) / 3u);

		float lightsPerCell = passBuf.f3dGridHWW[0].w;

		@property( !hlms_forwardplus_covers_entire_target )
			#define FWDPLUS_APPLY_OFFSET_Y(v) (v - passBuf.f3dViewportOffset.y)
			#define FWDPLUS_APPLY_OFFSET_X(v) (v - passBuf.f3dViewportOffset.x)
		@end

		//passBuf.f3dGridHWW[slice].x = grid_width / renderTarget->width;
		//passBuf.f3dGridHWW[slice].y = grid_height / renderTarget->height;
		//passBuf.f3dGridHWW[slice].z = grid_width * lightsPerCell;
		//uint sampleOffset = 0;
		@property( hlms_forwardplus_flipY )
			float windowHeight = passBuf.f3dGridHWW[1].w; //renderTarget->height
			uint sampleOffset = offset +
								uint(floor( (windowHeight - FWDPLUS_APPLY_OFFSET_Y(gl_FragCoord.y) ) *
											passBuf.f3dGridHWW[slice].y ) * passBuf.f3dGridHWW[slice].z) +
								uint(floor( FWDPLUS_APPLY_OFFSET_X(gl_FragCoord.x) *
											passBuf.f3dGridHWW[slice].x ) * lightsPerCell);
		@end @property( !hlms_forwardplus_flipY )
			uint sampleOffset = offset +
								uint(floor( FWDPLUS_APPLY_OFFSET_Y(gl_FragCoord.y) *
											passBuf.f3dGridHWW[slice].y ) * passBuf.f3dGridHWW[slice].z) +
								uint(floor( FWDPLUS_APPLY_OFFSET_X(gl_FragCoord.x) *
											passBuf.f3dGridHWW[slice].x ) * lightsPerCell);
		@end
	@end @property( hlms_forwardplus != forward3d )
		float f3dMinDistance	= passBuf.f3dData.x;
		float f3dInvExponentK	= passBuf.f3dData.y;
		float f3dNumSlicesSub1	= passBuf.f3dData.z;

		// See C++'s ForwardClustered::getSliceAtDepth
		float fSlice = log2( max( -inPs.pos.z - f3dMinDistance, 1 ) ) * f3dInvExponentK;
		fSlice = floor( min( fSlice, f3dNumSlicesSub1 ) );
		uint sliceSkip = uint( fSlice * @value( fwd_clustered_width_x_height ) );

		@property( !hlms_forwardplus_covers_entire_target )
			#define FWDPLUS_APPLY_OFFSET_Y(v) (v - passBuf.fwdScreenToGrid.w)
			#define FWDPLUS_APPLY_OFFSET_X(v) (v - passBuf.fwdScreenToGrid.z)
		@end

		uint sampleOffset = sliceSkip +
							uint(floor( FWDPLUS_APPLY_OFFSET_X(gl_FragCoord.x) * passBuf.fwdScreenToGrid.x ));
		@property( hlms_forwardplus_flipY )
			float windowHeight = passBuf.f3dData.w; //renderTarget->height
			sampleOffset += uint(floor( (windowHeight - FWDPLUS_APPLY_OFFSET_Y(gl_FragCoord.y) ) *
										passBuf.fwdScreenToGrid.y ) *
								 @value( fwd_clustered_width ));
		@end @property( !hlms_forwardplus_flipY )
			sampleOffset += uint(floor( FWDPLUS_APPLY_OFFSET_Y(gl_FragCoord.y) *
										passBuf.fwdScreenToGrid.y ) *
								 @value( fwd_clustered_width ));
		@end

		sampleOffset *= @value( fwd_clustered_lights_per_cell )u;
	@end
@end

@piece( forward3dLighting )
	@property( !skip_header_in_lighting )@insertpiece( forward3dHeader )@end

	finalColour += finalDecalEmissive;

	uint numLightsInGrid = bufferFetch( f3dGrid, int(sampleOffset) ).x;

	@property( hlms_forwardplus_debug )uint totalNumLightsInGrid = numLightsInGrid;@end

	for( uint i=0u; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + @value( hlms_reserved_slots )u) ).x;

		//Get the light
		vec4 posAndType = bufferFetch( f3dLightList, int(idx) );

	@property( !hlms_forwardplus_fine_light_mask )
		vec3 lightDiffuse	= bufferFetch( f3dLightList, int(idx + 1u) ).xyz;
	@end @property( hlms_forwardplus_fine_light_mask )
		vec4 lightDiffuse	= bufferFetch( f3dLightList, int(idx + 1u) ).xyzw;
	@end
		vec3 lightSpecular	= bufferFetch( f3dLightList, int(idx + 2u) ).xyz;
		vec4 attenuation	= bufferFetch( f3dLightList, int(idx + 3u) ).xyzw;

		vec3 lightDir	= posAndType.xyz - inPs.pos;
		float fDistance	= length( lightDir );

		if( fDistance <= attenuation.x @insertpiece( andObjLightMaskFwdPlusCmp ) )
		{
			lightDir *= 1.0 / fDistance;
			float atten = 1.0 / (0.5 + (attenuation.y + attenuation.z * fDistance) * fDistance );
			@property( hlms_forward_fade_attenuation_range )
				atten *= max( (attenuation.x - fDistance) * attenuation.w, 0.0f );
			@end

			//Point light
			vec3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse.xyz, lightSpecular );
			finalColour += tmpColour * atten;
		}
	}

	uint prevLightCount = numLightsInGrid;
	numLightsInGrid		= bufferFetch( f3dGrid, int(sampleOffset + 1u) ).x;

	@property( hlms_forwardplus_debug )totalNumLightsInGrid += numLightsInGrid;@end

	for( uint i=prevLightCount; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + @value( hlms_reserved_slots )u) ).x;

		//Get the light
		vec4 posAndType = bufferFetch( f3dLightList, int(idx) );

	@property( !hlms_forwardplus_fine_light_mask )
		vec3 lightDiffuse	= bufferFetch( f3dLightList, int(idx + 1u) ).xyz;
	@end @property( hlms_forwardplus_fine_light_mask )
		vec4 lightDiffuse	= bufferFetch( f3dLightList, int(idx + 1u) ).xyzw;
	@end
		vec3 lightSpecular	= bufferFetch( f3dLightList, int(idx + 2u) ).xyz;
		vec4 attenuation	= bufferFetch( f3dLightList, int(idx + 3u) ).xyzw;
		vec3 spotDirection	= bufferFetch( f3dLightList, int(idx + 4u) ).xyz;
		vec3 spotParams		= bufferFetch( f3dLightList, int(idx + 5u) ).xyz;

		vec3 lightDir	= posAndType.xyz - inPs.pos;
		float fDistance	= length( lightDir );

		if( fDistance <= attenuation.x @insertpiece( andObjLightMaskFwdPlusCmp ) )
		{
			lightDir *= 1.0 / fDistance;
			float atten = 1.0 / (0.5 + (attenuation.y + attenuation.z * fDistance) * fDistance );
			@property( hlms_forward_fade_attenuation_range )
				atten *= max( (attenuation.x - fDistance) * attenuation.w, 0.0f );
			@end

			//spotParams.x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
			//spotParams.y = cos( OuterAngle / 2 )
			//spotParams.z = falloff

			//Spot light
			float spotCosAngle = dot( normalize( inPs.pos - posAndType.xyz ), spotDirection.xyz );

			float spotAtten = clamp( (spotCosAngle - spotParams.y) * spotParams.x, 0.0, 1.0 );
			spotAtten = pow( spotAtten, spotParams.z );
			atten *= spotAtten;

			if( spotCosAngle >= spotParams.y )
			{
				vec3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse.xyz, lightSpecular );
				finalColour += tmpColour * atten;
			}
		}
	}

@property( hlms_enable_vpls )
	prevLightCount	= numLightsInGrid;
	numLightsInGrid	= bufferFetch( f3dGrid, int(sampleOffset + 2u) ).x;

	@property( hlms_forwardplus_debug )totalNumLightsInGrid += numLightsInGrid;@end

	for( uint i=prevLightCount; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + @value( hlms_reserved_slots )u) ).x;

		//Get the light
		vec4 posAndType = bufferFetch( f3dLightList, int(idx) );

		vec3 lightDiffuse	= bufferFetch( f3dLightList, int(idx + 1u) ).xyz;
		vec4 attenuation	= bufferFetch( f3dLightList, int(idx + 3u) ).xyzw;

		vec3 lightDir	= posAndType.xyz - inPs.pos;
		float fDistance	= length( lightDir );

		if( fDistance <= attenuation.x )
		{
			//lightDir *= 1.0 / fDistance;
			float atten = 1.0 / (0.5 + (attenuation.y + attenuation.z * fDistance) * fDistance );
			@property( hlms_forward_fade_attenuation_range )
				atten *= max( (attenuation.x - fDistance) * attenuation.w, 0.0f );
			@end

			//vec3 lightDir2	= posAndType.xyz - inPs.pos;
			//lightDir2 *= 1.0 / max( 1, fDistance );
			//lightDir2 *= 1.0 / fDistance;

			finalColour += BRDF_IR( lightDir, lightDiffuse ) * atten;
		}
	}
@end

@property( hlms_enable_decals )
/// Perform decals *after* sampling the diffuse colour.
@piece( forwardPlusDoDecals )
	@insertpiece( forward3dHeader )

	float3 finalDecalTsNormal = float3( 0.0f, 0.0f, 1.0f );
	float3 finalDecalEmissive = float3( 0.0f, 0.0f, 0.0f );

	numLightsInGrid	= bufferFetch( f3dGrid, int(sampleOffset + @value(hlms_decals_offset)) ).x;

	@property( hlms_forwardplus_debug )totalNumLightsInGrid += numLightsInGrid;@end

	for( uint i=0; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + @value( hlms_reserved_slots )u) ).x;

		float4 invWorldView0	= texelFetch( f3dLightList, int(idx) ).xyzw;
		float4 invWorldView1	= texelFetch( f3dLightList, int(idx + 1u) ).xyzw;
		float4 invWorldView2	= texelFetch( f3dLightList, int(idx + 2u) ).xyzw;
		float4 texIndices		= texelFetch( f3dLightList, int(idx + 3u) ).xyzw;

		float3 localPos;
		localPos.x = dot( invWorldView0.xyzw, float4( inPs.pos.xyz, 1.0f ) );
		localPos.y = dot( invWorldView1.xyzw, float4( inPs.pos.xyz, 1.0f ) );
		localPos.z = dot( invWorldView2.xyzw, float4( inPs.pos.xyz, 1.0f ) );

		float2 decalUV = localPos.xz + 0.5f;

		@property( hlms_decals_diffuse )
			float4 decalDiffuse = OGRE_SampleArray2D( decalsDiffuseTex, decalsSampler,
													  decalUV.xy, floatBitsToUint( texIndices.x ) ).xyzw;
		@end
		@property( hlms_decals_normals && normal_map )
			float2 decalNormals = OGRE_SampleArray2D( decalsNormalsTex, decalsSampler,
													  decalUV.xy, floatBitsToUint( texIndices.y ) ).xy;
		@end
		@property( hlms_decals_emissive )
			float3 decalEmissive = OGRE_SampleArray2D( decalsEmissiveTex, decalsSampler,
													   decalUV.xy, floatBitsToUint( texIndices.z ) ).xyz;
		@end

		@property( hlms_decals_diffuse )
			float decalMask = decalDiffuse.w;
		@end
		@property( !hlms_decals_diffuse )
			float decalMask = 1.0f;
		@end

		//Mask the decal entirely if localPos is outside the debox
		float3 absLocalPos = abs( localPos.xyz );
		decalMask = (absLocalPos.x > 0.5f || absLocalPos.y > 0.5f ||
					 absLocalPos.z > 0.5f) ? 0.0f : decalMask;

		@property( hlms_decals_diffuse )
			diffuseCol.xyz  = lerp( diffuseCol.xyz, decalDiffuse.xyz, decalMask );
		@end
		@property( hlms_decals_normals && normal_map )
			finalDecalTsNormal.xy += decalNormals.xy;
		@end
		@property( hlms_decals_emissive )
			finalDecalEmissive	+= (absLocalPos.x > 0.5f || absLocalPos.y > 0.5f ||
									absLocalPos.z > 0.5f) ? 0.0f : decalEmissive.xyz;
		@end

	}
@end
@property( hlms_decals_normals && normal_map )
	/// Apply decals normal *after* sampling the tangent space normals (and detail normals too).
	/// hlms_decals_normals will be unset if the Renderable cannot support normal maps (has no Tangents)
	@piece( forwardPlusApplyDecalsNormal )
		finalDecalTsNormal.xyz = normalize( finalDecalTsNormal.xyz );
		@property( normal_map_tex || detail_maps_normal )
			nNormal.xy	+= finalDecalTsNormal.xy;
			nNormal.z	*= finalDecalTsNormal.z;
		@end
		@property( !normal_map_tex && !detail_maps_normal )
			nNormal.xyz = finalDecalTsNormal.xyz;
		@end
		//Do not normalize as later normalize( TBN * nNormal ) will take care of it
	@end
@end
@end

	@property( hlms_forwardplus_debug )
		@property( hlms_forwardplus == forward3d )
			float occupancy = (totalNumLightsInGrid / passBuf.f3dGridHWW[0].w);
		@end @property( hlms_forwardplus != forward3d )
			float occupancy = (totalNumLightsInGrid / float( @value( fwd_clustered_lights_per_cell ) ));
		@end
		vec3 occupCol = vec3( 0.0, 0.0, 0.0 );
		if( occupancy < 1.0 / 3.0 )
			occupCol.z = occupancy;
		else if( occupancy < 2.0 / 3.0 )
			occupCol.y = occupancy;
		else
			occupCol.x = occupancy;

		finalColour.xyz = mix( finalColour.xyz, occupCol.xyz, 0.55f ) * 2;
	@end
@end
@end
