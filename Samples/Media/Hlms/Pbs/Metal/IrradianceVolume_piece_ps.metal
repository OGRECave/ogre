@property( irradiance_volumes )
@piece( applyIrradianceVolumes )
	float3 worldNormal = nNormal.xyz * toMat3x3( pass.invView ) );
	float3 worldPos = ( float4( inPs.pos.xyz, 1.0 ) * pass.invView ).xyz;

	float3 irradiancePos = worldPos.xyz * pass.irradianceSize.xyz - pass.irradianceOrigin.xyz;
	//Floor irradiancePos.y and put the fractional part so we can lerp.
	irradiancePos.y -= 0.5f;	//Texel centers are at center. Move it to origin.
	float origYPos;
	float fIrradianceYWeight = modf( irradiancePos.y, origYPos );
	origYPos *= 6.0;
	origYPos += 0.5f;	//Make sure we sample at center (so HW doesn't do linear
						//filtering on the Y axis. We'll do that manually)

	float3 isNegative = (float3)( worldNormal.xyz < float3( 0, 0, 0 ) );

	float3 tmpAmbientSample;

	//We need to make 3 samples (actually 6), one for each axis.
	/* The code is basically doing:
	float3 cAmbientCube[6];
	int3 isNegative = ( worldNormal < 0.0 );
	float3 linearColor;
	linearColor =	worldNormalSq.x * cAmbientCube[isNegative.x] +
					worldNormalSq.y * cAmbientCube[isNegative.y+2] +
					worldNormalSq.z * cAmbientCube[isNegative.z+4];

	We have 6 colour values per voxel. But GPUs can only store 1 colour value per cell.
	So we 6x the height to workaround that limitation. This also means we loose the ability
	to do HW bilinear filtering around the Y axis; therefore we do it ourselves manually.
	Because of this, instead of doing 3 samples, we end up doing 6 (in order to perform
	filtering around the Y axis)
	**/

	float irradianceTexInvHeight = pass.irradianceSize.w;

	irradiancePos.y		= (origYPos + isNegative.x) * irradianceTexInvHeight;
	float3 xAmbientSample	= irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;

	xAmbientSample = lerp( xAmbientSample, tmpAmbientSample, fIrradianceYWeight );

	irradiancePos.y		= (origYPos + (2.0f + isNegative.y)) * irradianceTexInvHeight;
	float3 yAmbientSample	= irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;

	yAmbientSample = lerp( yAmbientSample, tmpAmbientSample, fIrradianceYWeight );

	irradiancePos.y		= (origYPos + (4.0f + isNegative.z)) * irradianceTexInvHeight;
	float3 zAmbientSample = irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= irradianceVolume.sample( irradianceVolumeSampler, irradiancePos ).xyz;

	zAmbientSample = lerp( zAmbientSample, tmpAmbientSample, fIrradianceYWeight );


	float3 worldNormalSq = worldNormal.xyz * worldNormal.xyz;
	float3 ambientTerm =	worldNormalSq.x * xAmbientSample.xyz +
							worldNormalSq.y * yAmbientSample.xyz +
							worldNormalSq.z * zAmbientSample.xyz;
	ambientTerm *= pass.irradianceOrigin.w; //irradianceOrigin.w = irradianceMaxPower

	if( irradiancePos.x < 0 || irradiancePos.x > 1 ||
		irradiancePos.z < 0 || irradiancePos.z > 1 ||
		irradiancePos.y <= (6.0f * irradianceTexInvHeight) || irradiancePos.y >= 1 )
	{
		//Metal does not support border colour addressing mode.
		ambientTerm = float3( 0 );
	}

	finalColour.xyz += ambientTerm.xyz * @insertpiece( kD ).xyz;
@end
@end
