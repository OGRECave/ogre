@property( hlms_forwardplus )
@property( hlms_forwardplus_fine_light_mask )
	@piece( andObjLightMaskFwdPlusCmp )&& ((objLightMask & floatBitsToUint( lightDiffuse.w )) != 0u)@end
@end
@piece( forward3dLighting )
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

		//passBuf.f3dGridHWW[slice].x = grid_width / renderTarget->width;
		//passBuf.f3dGridHWW[slice].y = grid_height / renderTarget->height;
		//passBuf.f3dGridHWW[slice].z = grid_width * lightsPerCell;
		//uint sampleOffset = 0;
		@property( hlms_forwardplus_flipY )
			float windowHeight = passBuf.f3dGridHWW[1].w; //renderTarget->height
			uint sampleOffset = offset +
								uint(floor( (windowHeight - (gl_FragCoord.y - passBuf.f3dViewportOffset.y) ) * passBuf.f3dGridHWW[slice].y ) * passBuf.f3dGridHWW[slice].z) +
								uint(floor( (gl_FragCoord.x - passBuf.f3dViewportOffset.x) * passBuf.f3dGridHWW[slice].x ) * lightsPerCell);
		@end @property( !hlms_forwardplus_flipY )
			uint sampleOffset = offset +
								uint(floor( (gl_FragCoord.y - passBuf.f3dViewportOffset.y) * passBuf.f3dGridHWW[slice].y ) * passBuf.f3dGridHWW[slice].z) +
								uint(floor( (gl_FragCoord.x - passBuf.f3dViewportOffset.x) * passBuf.f3dGridHWW[slice].x ) * lightsPerCell);
		@end
	@end @property( hlms_forwardplus != forward3d )
		float f3dMinDistance	= passBuf.f3dData.x;
		float f3dInvExponentK	= passBuf.f3dData.y;
		float f3dNumSlicesSub1	= passBuf.f3dData.z;

		// See C++'s ForwardClustered::getSliceAtDepth
		float fSlice = log2( max( -inPs.pos.z - f3dMinDistance, 1 ) ) * f3dInvExponentK;
		fSlice = floor( min( fSlice, f3dNumSlicesSub1 ) );
		uint sliceSkip = uint( fSlice * @value( fwd_clustered_width_x_height ) );

		uint sampleOffset = sliceSkip +
							uint(floor( (gl_FragCoord.x - passBuf.fwdScreenToGrid.z) * passBuf.fwdScreenToGrid.x ));
		@property( hlms_forwardplus_flipY )
			float windowHeight = passBuf.f3dData.w; //renderTarget->height
			sampleOffset += uint(floor( (windowHeight - (gl_FragCoord.y - passBuf.fwdScreenToGrid.w) ) * passBuf.fwdScreenToGrid.y ) *
								 @value( fwd_clustered_width ));
		@end @property( !hlms_forwardplus_flipY )
			sampleOffset += uint(floor( (gl_FragCoord.y - passBuf.fwdScreenToGrid.w) * passBuf.fwdScreenToGrid.y ) *
								 @value( fwd_clustered_width ));
		@end

		sampleOffset *= @value( fwd_clustered_lights_per_cell )u;
	@end

	uint numLightsInGrid = bufferFetch( f3dGrid, int(sampleOffset) ).x;

	@property( hlms_forwardplus_debug )uint totalNumLightsInGrid = numLightsInGrid;@end

	for( uint i=0u; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + 3u) ).x;

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
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + 3u) ).x;

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
		uint idx = bufferFetch( f3dGrid, int(sampleOffset + i + 3u) ).x;

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
