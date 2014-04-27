@piece( NormalNonPremul )
	outColour.xyz = mix( outColour.xyz, topImage@value(t).xyz, topImage@value(t).a );
@end

@piece( NormalPremul )
	outColour.xyz = (1.0f - topImage@value(t).a) * outColour.xyz + topImage@value(t).xyz;
@end

@piece( Add )
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz + topImage@value(t).xyz, vec3(1.0f) ),
						 topImage@value(t).a );
@end

@piece( Subtract )
	outColour.xyz = mix( outColour.xyz,
						 max( outColour.xyz - topImage@value(t).xyz, vec3(0.0f) ),
						 topImage@value(t).a );
@end

@piece( Multiply )
	outColour.xyz = mix( outColour.xyz,
						 outColour.xyz * topImage@value(t).xyz,
						 topImage@value(t).a );
@end

@piece( Multiply2x )
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz * topImage@value(t).xyz * 2.0f, vec3(1.0f) ),
						 topImage@value(t).a );
@end

@piece( Screen )
	outColour.xyz = mix( outColour.xyz,
						 1.0f - (1.0f - outColour.xyz) * (1.0f - topImage@value(t).xyz),
						 topImage@value(t).a );
@end

@piece( Overlay )
	outColour.xyz = mix( outColour.xyz,
						 outColour.xyz * ( outColour.xyz + 2.0f * topImage@value(t).xyz * (1.0f - outColour.xyz) ),
						 topImage@value(t).a );
@end

@piece( Lighten )
	outColour.xyz = mix( outColour.xyz,
						 max( outColour.xyz, topImage@value(t).xyz ),
						 topImage@value(t).a );
@end

@piece( Darken )
	outColour.xyz = mix( outColour.xyz,
						 min( outColour.xyz, topImage@value(t).xyz ),
						 topImage@value(t).a );
@end

@piece( GrainExtract )
	outColour.xyz = mix( outColour.xyz,
						 (outColour.xyz - topImage@value(t).xyz) + 0.5f,
						 topImage@value(t).a );
@end

@piece( GrainMerge )
	outColour.xyz = mix( outColour.xyz,
						 (outColour.xyz + topImage@value(t).xyz) - 0.5f,
						 topImage@value(t).a );
@end

@piece( Difference )
	outColour.xyz = mix( outColour.xyz,
						 abs(outColour.xyz - topImage@value(t).xyz),
						 topImage@value(t).a );
@end
