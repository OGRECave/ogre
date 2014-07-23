@piece( NormalNonPremul )
	//Normal Non Premultiplied @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz, detailCol@value(t).xyz, detailCol@value(t).a );@add( t, 1 )
@end

@piece( NormalPremul )
	//Normal Premultiplied @value(t)
	diffuseCol.xyz = (1.0 - detailCol@value(t).a) * diffuseCol.xyz + detailCol@value(t).xyz;@add( t, 1 )
@end

@piece( Add )
	//Add @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  min( diffuseCol.xyz + detailCol@value(t).xyz, vec3(1.0) ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Subtract )
	//Subtract @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  max( diffuseCol.xyz - detailCol@value(t).xyz, vec3(0.0) ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Multiply )
	//Multiply @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  diffuseCol.xyz * detailCol@value(t).xyz,
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Multiply2x )
	//Multiply2x @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  min( diffuseCol.xyz * detailCol@value(t).xyz * 2.0, vec3(1.0) ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Screen )
	//Screen @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  1.0 - (1.0 - diffuseCol.xyz) * (1.0 - detailCol@value(t).xyz),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Overlay )
	//Overlay @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  diffuseCol.xyz * ( diffuseCol.xyz + 2.0 * detailCol@value(t).xyz * (1.0 - diffuseCol.xyz) ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Lighten )
	//Lighten @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  max( diffuseCol.xyz, detailCol@value(t).xyz ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Darken )
	//Darken @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  min( diffuseCol.xyz, detailCol@value(t).xyz ),
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( GrainExtract )
	//GrainExtract @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  (diffuseCol.xyz - detailCol@value(t).xyz) + 0.5f,
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( GrainMerge )
	//GrainMerge @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  (diffuseCol.xyz + detailCol@value(t).xyz) - 0.5f,
						  detailCol@value(t).a );@add( t, 1 )
@end

@piece( Difference )
	//Difference @value(t)
	diffuseCol.xyz = mix( diffuseCol.xyz,
						  abs(diffuseCol.xyz - detailCol@value(t).xyz),
						  detailCol@value(t).a );@add( t, 1 )
@end
