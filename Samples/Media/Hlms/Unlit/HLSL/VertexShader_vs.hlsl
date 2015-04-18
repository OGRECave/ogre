@insertpiece( SetCrossPlatformSettings )

float4x4 UNPACK_MAT4( Buffer<float4> matrixBuf, uint pixelIdx )
{
	float4 row1 = matrixBuf.Load( int((pixelIdx) << 2u) );
	float4 row2 = matrixBuf.Load( int(((pixelIdx) << 2u) + 1u) );
	float4 row3 = matrixBuf.Load( int(((pixelIdx) << 2u) + 2u) );
	float4 row4 = matrixBuf.Load( int(((pixelIdx) << 2u) + 3u) );

	return float4x4( row1, row2, row3, row4 );
}

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( InstanceDecl )
Buffer<float4> worldMatBuf : register(t0);
@property( texture_matrix )Buffer<float4> animationMatrixBuf : register(t1);@end
// END UNIFORM DECLARATION

struct VS_INPUT
{
	float4 vertex : POSITION;
@property( hlms_colour )	float4 colour : COLOR0;@end
@foreach( hlms_uv_count, n )
	float@value( hlms_uv_count@n ) uv@n : TEXCOORD@n;@end
	uint drawId : DRAWID;
};

struct PS_INPUT
{
@insertpiece( VStoPS_block )
	float4 gl_Position : SV_Position;
};

PS_INPUT main( VS_INPUT input )
{
	PS_INPUT outVs;

	//uint drawId = 1;
	float4x4 worldViewProj;
	worldViewProj = UNPACK_MAT4( worldMatBuf, input.drawId );

@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = mul( worldViewProj, input.vertex );
@end

@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w		= 1.0f;
	outVs.gl_Position.xyz	= mul( worldViewProj, input.vertex ).xyz;
	float L = length( outVs.gl_Position.xyz );
	outVs.gl_Position.z		+= 1.0f;
	outVs.gl_Position.xy	/= outVs.gl_Position.z;
	outVs.gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);
@end

@property( !hlms_shadowcaster )
@property( hlms_colour )	outVs.colour = input.colour;@end

@property( texture_matrix )	float4x4 textureMatrix;@end

@foreach( out_uv_count, n )
	@property( out_uv@_texture_matrix )textureMatrix = UNPACK_MAT4( animationMatrixBuf, (materialIdx[input.drawId].x << 4u) + @value( out_uv@n_tex_unit ) );@end
	outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) =
@property( out_uv@_texture_matrix )
			mul( textureMatrix, input.uv@value( out_uv@n_source_uv ).xy );
@end @property( !out_uv@_texture_matrix )
			input.uv@value( out_uv@n_source_uv ).xy;@end @end

	outVs.drawId = input.drawId;

@end @property( hlms_shadowcaster )
	float shadowConstantBias = asfloat( materialIdx[input.drawId].y );
	//Linear depth
	outVs.depth	= (outVs.gl_Position.z - passBuf.depthRange.x + shadowConstantBias * passBuf.depthRange.y) * passBuf.depthRange.y;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	outVs.gl_Position.z = outVs.gl_Position.z * (outVs.gl_Position.w * passBuf.depthRange.y);
@end

	return outVs;
}
