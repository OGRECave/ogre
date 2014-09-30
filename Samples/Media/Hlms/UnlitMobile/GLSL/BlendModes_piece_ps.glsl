@piece( NormalNonPremul )
	//Normal Non Premultiplied @counter(t)
	outColour.xyz = mix( outColour.xyz, topImage@value(t).xyz, topImage@value(t).a );
@end

@piece( NormalPremul )
	//Normal Premultiplied @counter(t)
	outColour.xyz = (1.0 - topImage@value(t).a) * outColour.xyz + topImage@value(t).xyz;
@end

@piece( Add )
	//Add @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz + topImage@value(t).xyz, vec3(1.0) ),
						 topImage@value(t).a );
@end

@piece( Subtract )
	//Subtract @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 max( outColour.xyz - topImage@value(t).xyz, vec3(0.0) ),
						 topImage@value(t).a );
@end

@piece( Multiply )
	//Multiply @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 outColour.xyz * topImage@value(t).xyz,
						 topImage@value(t).a );
@end

@piece( Multiply2x )
	//Multiply2x @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz * topImage@value(t).xyz * 2.0, vec3(1.0) ),
						 topImage@value(t).a );
@end

@piece( Screen )
	//Screen @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 1.0 - (1.0 - outColour.xyz) * (1.0 - topImage@value(t).xyz),
						 topImage@value(t).a );
@end

@piece( Overlay )
	//Overlay @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 outColour.xyz * ( outColour.xyz + 2.0 * topImage@value(t).xyz * (1.0 - outColour.xyz) ),
						 topImage@value(t).a );
@end

@piece( Lighten )
	//Lighten @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 max( outColour.xyz, topImage@value(t).xyz ),
						 topImage@value(t).a );
@end

@piece( Darken )
	//Darken @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz, topImage@value(t).xyz ),
						 topImage@value(t).a );
@end

@piece( GrainExtract )
	//GrainExtract @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 (outColour.xyz - topImage@value(t).xyz) + 0.5f,
						 topImage@value(t).a );
@end

@piece( GrainMerge )
	//GrainMerge @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 (outColour.xyz + topImage@value(t).xyz) - 0.5f,
						 topImage@value(t).a );
@end

@piece( Difference )
	//Difference @counter(t)
	outColour.xyz = mix( outColour.xyz,
						 abs(outColour.xyz - topImage@value(t).xyz),
						 topImage@value(t).a );
@end

@foreach( 16, n )
@property( uv_atlas@n ) @piece( atlasOffset@n )* atlasOffsets[@n].z + atlasOffsets[@n].xy @end @end @end
