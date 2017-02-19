//Based on GPUOpen's samples SeparableFilter11
//https://github.com/GPUOpen-LibrariesAndSDKs/SeparableFilter11
//For better understanding, read "Efficient Compute Shader Programming" from Bill Bilodeau
//http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/Efficient%20Compute%20Shader%20Programming.pps

//TL;DR:
//	* Each thread works on 4 pixels at a time (for VLIW hardware, i.e. Radeon HD 5000 & 6000 series).
//	* 256 pixels per threadgroup. Each threadgroup works on 2 rows of 128 pixels each.
//	  That means 32x2 threads = 64. 64 threads x 4 pixels per thread = 256

@piece( data_type )float3@end
@piece( lds_data_type )float3@end
@piece( lds_definition )
	threadgroup float3 g_f3LDS[ 2 ] [ @value( samples_per_threadgroup ) ]
@end
@piece( extra_params )
	float srcLodIdx;
	uint dstLodIdx;
@end

@piece( image_sample )
	return inputImage.sample( inputSampler, f2SamplePosition, level( p.srcLodIdx ) ).xyz;
@end

//Overwrite these so that num_thread_groups gets correctly calculated by accounting LOD.
@pset( texture0_width, width_with_lod )
@pset( texture0_height, height_with_lod )

@pset( downscale_lq, 1 )

@piece( image_store )
	@property( downscale_lq )
		@foreach( 2, iPixel )
			outputImage.write( float4( outColour[ @iPixel ], 1.0 ), uint2( i2Center +  @iPixel * i2Inc ),
							   p.dstLodIdx );@end
	@end @property( !downscale_lq )
		@foreach( 2, iPixel )
			outputImage.write( float4( (outColour[ @iPixel * 2 ] + outColour[ @iPixel * 2 + 1 ]) * 0.5, 1.0 ),
							   uint2( i2Center +  @iPixel * i2Inc ),
							   p.dstLodIdx );@end
	@end
@end
