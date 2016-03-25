//Reset t to 0 just in case (values are preserved from previous stages)
@pset( t, 0 )

@property( !hlms_shadowcaster )
@piece( NormalNonPremul )
	//Normal Non Premultiplied @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz, detailCol@value(t).xyz, detailCol@value(t).a );
	diffuseCol.w = lerp( diffuseCol.w, 1.0, detailCol@value(t).w );
@end

@piece( NormalPremul )
	//Normal Premultiplied @value(t)
	diffuseCol.xyz = (1.0 - detailCol@value(t).a) * diffuseCol.xyz + detailCol@value(t).xyz;
	diffuseCol.w = lerp( diffuseCol.w, 1.0, detailCol@value(t).w );
@end

@piece( Add )
	//Add @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   min( diffuseCol.xyz + detailCol@value(t).xyz, float3(1.0, 1.0, 1.0) ),
						   detailCol@value(t).a );
@end

@piece( Subtract )
	//Subtract @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   max( diffuseCol.xyz - detailCol@value(t).xyz, float3(0.0, 0.0, 0.0) ),
						   detailCol@value(t).a );
@end

@piece( Multiply )
	//Multiply @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   diffuseCol.xyz * detailCol@value(t).xyz,
						   detailCol@value(t).a );
@end

@piece( Multiply2x )
	//Multiply2x @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   min( diffuseCol.xyz * detailCol@value(t).xyz * 2.0, float3(1.0, 1.0, 1.0) ),
						   detailCol@value(t).a );
@end

@piece( Screen )
	//Screen @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   1.0 - (1.0 - diffuseCol.xyz) * (1.0 - detailCol@value(t).xyz),
						   detailCol@value(t).a );
@end

@piece( Overlay )
	//Overlay @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   diffuseCol.xyz * ( diffuseCol.xyz + 2.0 * detailCol@value(t).xyz * (1.0 - diffuseCol.xyz) ),
						   detailCol@value(t).a );
@end

@piece( Lighten )
	//Lighten @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   max( diffuseCol.xyz, detailCol@value(t).xyz ),
						   detailCol@value(t).a );
@end

@piece( Darken )
	//Darken @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   min( diffuseCol.xyz, detailCol@value(t).xyz ),
						   detailCol@value(t).a );
@end

@piece( GrainExtract )
	//GrainExtract @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   (diffuseCol.xyz - detailCol@value(t).xyz) + 0.5f,
						   detailCol@value(t).a );
@end

@piece( GrainMerge )
	//GrainMerge @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   (diffuseCol.xyz + detailCol@value(t).xyz) - 0.5f,
						   detailCol@value(t).a );
@end

@piece( Difference )
	//Difference @value(t)
	diffuseCol.xyz = lerp( diffuseCol.xyz,
						   abs(diffuseCol.xyz - detailCol@value(t).xyz),
						   detailCol@value(t).a );
@end
@end @property( hlms_shadowcaster )

@piece( NormalNonPremul )
	//Normal Non Premultiplied @value(t)
	diffuseCol = lerp( diffuseCol, 1.0, detailCol@value(t) );
@end

@piece( NormalPremul )
	//Normal Premultiplied @value(t)
	diffuseCol = lerp( diffuseCol, 1.0, detailCol@value(t) );
@end

@end