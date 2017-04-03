@property( irradiance_volumes )
@piece( applyIrradianceVolumes )
	vec3 worldNormal = nNormal.xyz * mat3(passBuf.invView);
	vec3 worldPos = (vec4( inPs.pos.xyz, 1.0 ) * passBuf.invView).xyz;

	vec3 irradiancePos = worldPos.xyz * passBuf.irradianceSize.xyz - passBuf.irradianceOrigin.xyz;
	//Floor irradiancePos.y and put the fractional part so we can lerp.
	irradiancePos.y -= 0.5f;	//Texel centers are at center. Move it to origin.
	float origYPos;
	float fIrradianceYWeight = modf( irradiancePos.y, origYPos );
	origYPos *= 6.0;
	origYPos += 0.5f;	//Make sure we sample at center (so HW doesn't do linear
						//filtering on the Y axis. We'll do that manually)

	vec3 isNegative = vec3( lessThan( worldNormal.xyz, vec3( 0, 0, 0 ) ) );

	vec3 tmpAmbientSample;

	//We need to make 3 samples (actually 6), one for each axis.
	/* The code is basically doing:
	vec3 cAmbientCube[6];
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

	float irradianceTexInvHeight = passBuf.irradianceSize.w;

	irradiancePos.y		= (origYPos + isNegative.x) * irradianceTexInvHeight;
	vec3 xAmbientSample	= texture( irradianceVolume, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= texture( irradianceVolume, irradiancePos ).xyz;

	xAmbientSample = mix( xAmbientSample, tmpAmbientSample, fIrradianceYWeight );

	irradiancePos.y		= (origYPos + (2.0f + isNegative.y)) * irradianceTexInvHeight;
	vec3 yAmbientSample	= texture( irradianceVolume, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= texture( irradianceVolume, irradiancePos ).xyz;

	yAmbientSample = mix( yAmbientSample, tmpAmbientSample, fIrradianceYWeight );

	irradiancePos.y		= (origYPos + (4.0f + isNegative.z)) * irradianceTexInvHeight;
	vec3 zAmbientSample = texture( irradianceVolume, irradiancePos ).xyz;
	irradiancePos.y		+= 6.0f * irradianceTexInvHeight;
	tmpAmbientSample	= texture( irradianceVolume, irradiancePos ).xyz;

	zAmbientSample = mix( zAmbientSample, tmpAmbientSample, fIrradianceYWeight );


	vec3 worldNormalSq = worldNormal.xyz * worldNormal.xyz;
	vec3 ambientTerm =	worldNormalSq.x * xAmbientSample.xyz +
						worldNormalSq.y * yAmbientSample.xyz +
						worldNormalSq.z * zAmbientSample.xyz;
	ambientTerm *= passBuf.irradianceOrigin.w; //irradianceOrigin.w = irradianceMaxPower

	finalColour.xyz += ambientTerm.xyz * @insertpiece( kD ).xyz;
@end
@end
