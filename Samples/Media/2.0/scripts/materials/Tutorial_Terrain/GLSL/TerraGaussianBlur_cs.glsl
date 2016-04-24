//Based on GPUOpen's samples SeparableFilter11
//https://github.com/GPUOpen-LibrariesAndSDKs/SeparableFilter11
//For better understanding, read "Efficient Compute Shader Programming" from Bill Bilodeau
//http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/Efficient%20Compute%20Shader%20Programming.pps

//TL;DR:
//	* Each thread works on 4 pixels at a time (for VLIW hardware, i.e. Radeon HD 5000 & 6000 series).
//	* 256 pixels per threadgroup. Each threadgroup works on 2 rows of 128 pixels each.
//	  That means 32x2 threads = 64. 64 threads x 4 pixels per thread = 256

@piece( data_type )vec3@end
@piece( lds_data_type )vec3@end
@piece( lds_definition )shared vec3 g_f3LDS[ 2 ] [ @value( samples_per_threadgroup ) ];@end

@piece( image_sample )
	return textureLod( inputImage, f2SamplePosition, 0 ).xyz;
@end

@piece( image_store )
	@foreach( 4, iPixel )
		imageStore( outputImage, ivec2( i2Center +  @iPixel * i2Inc ), vec4( outColour[ @iPixel ], 1.0 ) );@end
@end
