@property( hlms_forward3d )
@piece( forward3dLighting )
	float f3dMinDistance	= passBuf.f3dData.x;
	float f3dInvMaxDistance	= passBuf.f3dData.y;
	float f3dNumSlicesSub1	= passBuf.f3dData.z;
	uint cellsPerTableOnGrid0= asuint( passBuf.f3dData.w );

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
	uint sampleOffset = offset +
						uint(floor( gl_FragCoord.y * passBuf.f3dGridHWW[slice].y ) * passBuf.f3dGridHWW[slice].z) +
						uint(floor( gl_FragCoord.x * passBuf.f3dGridHWW[slice].x ) * lightsPerCell);

	uint numLightsInGrid = f3dGrid.Load( int(sampleOffset) ).x;

	for( uint i=0u; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = f3dGrid.Load( int(sampleOffset + i + 1u) ).x;

		//Get the light
		float4 posAndType = f3dLightList.Load( int(idx) );

		float3 lightDiffuse	= f3dLightList.Load( int(idx + 1u) ).xyz;
		float3 lightSpecular= f3dLightList.Load( int(idx + 2u) ).xyz;
		float3 attenuation	= f3dLightList.Load( int(idx + 3u) ).xyz;

		float3 lightDir	= posAndType.xyz - inPs.pos;
		float fDistance	= length( lightDir );

		if( fDistance <= attenuation.x )
		{
			lightDir *= 1.0 / fDistance;
			float atten = 1.0 / (1.0 + (attenuation.y + attenuation.z * fDistance) * fDistance );

			if( posAndType.w == 1.0 )
			{
				//Point light
				float3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse, lightSpecular, material, nNormal @insertpiece( brdfExtraParams ) );
				finalColour += tmpColour * atten;
			}
			else
			{
				//spotParams.x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
				//spotParams.y = cos( OuterAngle / 2 )
				//spotParams.z = falloff

				//Spot light
				float3 spotDirection	= f3dLightList.Load( int(idx + 4u) ).xyz;
				float3 spotParams		= f3dLightList.Load( int(idx + 5u) ).xyz;

				float spotCosAngle = dot( normalize( inPs.pos - posAndType.xyz ), spotDirection.xyz );

				float spotAtten = clamp( (spotCosAngle - spotParams.y) * spotParams.x, 0.0, 1.0 );
				spotAtten = pow( spotAtten, spotParams.z );
				atten *= spotAtten;

				if( spotCosAngle >= spotParams.y )
				{
					float3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse, lightSpecular, material, nNormal @insertpiece( brdfExtraParams ) );
					finalColour += tmpColour * atten;
				}
			}
		}
	}

	@property( hlms_forward3d_debug )
		float occupancy = (numLightsInGrid / passBuf.f3dGridHWW[0].w);
		float3 occupCol = float3( 0.0, 0.0, 0.0 );
		if( occupancy < 1.0 / 3.0 )
			occupCol.z = occupancy;
		else if( occupancy < 2.0 / 3.0 )
			occupCol.y = occupancy;
		else
			occupCol.x = occupancy;

		finalColour.xyz = mix( finalColour.xyz, occupCol.xyz, 0.95f );
	@end
@end
@end
