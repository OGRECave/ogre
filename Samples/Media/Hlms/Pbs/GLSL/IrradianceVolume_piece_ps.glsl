@property( irradiance_volumes )
@piece( applyIrradianceVolumes )
	float irradianceTexHeightDiv6	= pass.irradiancePower.z;
	float irradianceTexInvHeight	= pass.irradiancePower.w;
	vec3 irradiancePos = worldPos.xyz * pass.irradianceOrigin.w - pass.irradianceOrigin.xyz;
	//Floor irradiancePos.y and put the fractional part so we can lerp.
	//irradiancePos.y -= irradianceTexInvHeight * 0.5f; //Texels are centered at 0.5, undo that.
	irradiancePos.y *= irradianceTexHeightDiv6;
	irradiancePos.y -= 0.5f;	//Texel centers are at center. Move it to origin.
	float origYPos;
	float fIrradianceYWeight = modf( irradiancePos.y, origYPos );
	origYPos *= 6.0;
	origYPos += 0.5f;	//Make sure we sample at center (so HW doesn't do linear
						//filtering on the Y axis. We'll do that manually)

	bvec3 isNegative = lessThan( worldNormal.xyz, vec3( 0, 0, 0 ) );

	vec3 tmpAmbientSample;

	//We need to make 3 samples, one for each axis.
	/* The code is basically doing:
	vec3 cAmbientCube[6];
	int3 isNegative = ( worldNormal < 0.0 );
	float3 linearColor;
	linearColor =	worldNormalSq.x * cAmbientCube[isNegative.x] +
					worldNormalSq.y * cAmbientCube[isNegative.y+2] +
					worldNormalSq.z * cAmbientCube[isNegative.z+4];
	*/

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
	vec3 ambientTerm =	worldNormalSq.x * (xAmbientSample.xyz + pass.irradiancePower.x) +
						worldNormalSq.y * (yAmbientSample.xyz + pass.irradiancePower.x) +
						worldNormalSq.z * (zAmbientSample.xyz + pass.irradiancePower.x);
	ambientTerm *= pass.irradiancePower.y;
@end
@end
