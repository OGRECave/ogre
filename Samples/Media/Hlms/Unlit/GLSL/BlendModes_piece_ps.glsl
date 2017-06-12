//Reset t to 0 just in case (values are preserved from previous stages)
@pset( t, 0 )

@piece( NormalNonPremul )
	//Normal Non Premultiplied @counter(t)
	outColour.xyz = mix( outColour.xyz, topImage@value(t).xyz, topImage@value(t).a );
	outColour.w = mix( outColour.w, 1.0, topImage@value(t).w );
@end

@piece( NormalPremul )
	//Normal Premultiplied @counter(t)
	outColour.xyz = (1.0 - topImage@value(t).a) * outColour.xyz + topImage@value(t).xyz;
	outColour.w = mix( outColour.w, 1.0, topImage@value(t).w );
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

// Get the indexes to the textureMaps[] array using template code. We had to add 1
// to the actual value otherwise property( diffuse_map ) fails when the index is 0
@foreach( diffuse_map, n )
	@sub( diffuse_map@n_idx, diffuse_map@n, 1 ) @end

@piece( diffuseIdx0 )material.indices0_3.x & 0x0000FFFFu@end
@piece( diffuseIdx1 )material.indices0_3.x >> 16u@end
@piece( diffuseIdx2 )material.indices0_3.y & 0x0000FFFFu@end
@piece( diffuseIdx3 )material.indices0_3.y >> 16u@end
@piece( diffuseIdx4 )material.indices0_3.z & 0x0000FFFFu@end
@piece( diffuseIdx5 )material.indices0_3.z >> 16u@end
@piece( diffuseIdx6 )material.indices0_3.w & 0x0000FFFFu@end
@piece( diffuseIdx7 )material.indices0_3.w >> 16u@end

@foreach( diffuse_map, n )
	@property( diffuse_map@n_array )
		@piece( SamplerOrigin@n )textureMapsArray[@value(diffuse_map@n_idx)]@end
	@end @property( !diffuse_map@n_array )
		@piece( SamplerOrigin@n )textureMaps[@value(diffuse_map@n_idx)]@end
	@end
	@property( !diffuse_map@n_reflection )
		@property( diffuse_map@n_array )
			@piece( SamplerUV@n )vec3( inPs.uv@value( uv_diffuse@n ).@insertpiece( uv_diffuse_swizzle@n ), @insertpiece( diffuseIdx@n ) )@end
		@end @property( !diffuse_map@n_array )
			@piece( SamplerUV@n )inPs.uv@value( uv_diffuse@n ).@insertpiece( uv_diffuse_swizzle@n )@end
		@end
	@end @property( diffuse_map@n_reflection )
		@property( !hlms_forwardplus_flipY )
			@property( diffuse_map@n_array )
				@piece( SamplerUV@n )vec3( gl_FragCoord.x * passBuf.invWindowSize.x, 1.0 - gl_FragCoord.y * passBuf.invWindowSize.y, @insertpiece( diffuseIdx@n ) )@end
			@end @property( !diffuse_map@n_array )
				@piece( SamplerUV@n )vec2( gl_FragCoord.x * passBuf.invWindowSize.x, 1.0 - gl_FragCoord.y * passBuf.invWindowSize.y )@end
			@end
		@end @property( hlms_forwardplus_flipY )
			@property( diffuse_map@n_array )
				@piece( SamplerUV@n )vec3( gl_FragCoord.xy * passBuf.invWindowSize.xy, @insertpiece( diffuseIdx@n ) )@end
			@end @property( !diffuse_map@n_array )
				@piece( SamplerUV@n )vec2( gl_FragCoord.xy * passBuf.invWindowSize.xy )@end
			@end
		@end
	@end
@end
