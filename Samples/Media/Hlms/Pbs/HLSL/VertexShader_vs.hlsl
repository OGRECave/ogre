float4x4 UNPACK_MAT4( Buffer<float4> matrixBuf, uint pixelIdx )
{
	float4 row1 = matrixBuf.Load( int((pixelIdx) << 2u) );
	float4 row2 = matrixBuf.Load( int(((pixelIdx) << 2u) + 1u) );
	float4 row3 = matrixBuf.Load( int(((pixelIdx) << 2u) + 2u) );
	float4 row4 = matrixBuf.Load( int(((pixelIdx) << 2u) + 3u) );

	return float4x4( row1, row2, row3, row4 );
}

float3x4 UNPACK_MAT3x4( Buffer<float4> matrixBuf, uint pixelIdx )
{
	float4 row1 = matrixBuf.Load( int((pixelIdx) << 2u) );
	float4 row2 = matrixBuf.Load( int(((pixelIdx) << 2u) + 1u) );
	float4 row3 = matrixBuf.Load( int(((pixelIdx) << 2u) + 2u) );

	return float3x4( row1, row2, row3 );
}

struct VS_INPUT
{
	float4 vertex : POSITION;
@property( hlms_normal )	float3 normal : NORMAL;@end
@property( hlms_qtangent )	float4 qtangent : NORMAL;@end

@property( normal_map && !hlms_qtangent )
	float3 tangent	: TANGENT;
	@property( hlms_binormal )float3 binormal	: BINORMAL;@end
@end

@property( hlms_skeleton )
	uint4 blendIndices	: BLENDINDICES;
	float4 blendWeights : BLENDWEIGHT;@end

@foreach( hlms_uv_count, n )
	float@value( hlms_uv_count@n ) uv@n : TEXCOORD@n;@end
	uint drawId : DRAWID;
};

struct PS_INPUT
{
@insertpiece( VStoPS_block )
	float4 gl_Position: SV_Position;
};

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@property( hlms_skeleton || hlms_shadowcaster )@insertpiece( InstanceDecl )@end
Buffer<float4> worldMatBuf : register(t0);
// END UNIFORM DECLARATION

@property( hlms_qtangent )
@insertpiece( DeclQuat_xAxis )
@property( normal_map )
@insertpiece( DeclQuat_yAxis )
@end @end

@property( !hlms_skeleton )
@piece( local_vertex )input.vertex@end
@piece( local_normal )normal@end
@piece( local_tangent )tangent@end
@end
@property( hlms_skeleton )
@piece( local_vertex )worldPos@end
@piece( local_normal )worldNorm@end
@piece( local_tangent )worldTang@end
@end

@property( hlms_skeleton )@piece( SkeletonTransform )
	uint _idx = (input.blendIndices[0] << 1u) + input.blendIndices[0]; //blendIndices[0] * 3u; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		uint matStart = worldMaterialIdx[input.drawId].x >> 9u;
	float4 worldMat[3];
		worldMat[0] = worldMatBuf.Load( int(matStart + _idx + 0u) );
		worldMat[1] = worldMatBuf.Load( int(matStart + _idx + 1u) );
		worldMat[2] = worldMatBuf.Load( int(matStart + _idx + 2u) );
	float4 worldPos;
	worldPos.x = dot( worldMat[0], input.vertex );
	worldPos.y = dot( worldMat[1], input.vertex );
	worldPos.z = dot( worldMat[2], input.vertex );
	worldPos *= input.blendWeights[0];
	@property( hlms_normal || hlms_qtangent )float3 worldNorm;
	worldNorm.x = dot( worldMat[0].xyz, normal );
	worldNorm.y = dot( worldMat[1].xyz, normal );
	worldNorm.z = dot( worldMat[2].xyz, normal );
	worldNorm *= input.blendWeights[0];@end
	@property( normal_map )float3 worldTang;
	worldTang.x = dot( worldMat[0].xyz, tangent );
	worldTang.y = dot( worldMat[1].xyz, tangent );
	worldTang.z = dot( worldMat[2].xyz, tangent );
	worldTang *= input.blendWeights[0];@end

	@psub( NeedsMoreThan1BonePerVertex, hlms_bones_per_vertex, 1 )
	@property( NeedsMoreThan1BonePerVertex )float4 tmp;@end
	@foreach( hlms_bones_per_vertex, n, 1 )
	_idx = (input.blendIndices[@n] << 1u) + input.blendIndices[@n]; //blendIndices[@n] * 3; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		worldMat[0] = worldMatBuf.Load( int(matStart + _idx + 0u) );
		worldMat[1] = worldMatBuf.Load( int(matStart + _idx + 1u) );
		worldMat[2] = worldMatBuf.Load( int(matStart + _idx + 2u) );
	tmp.x = dot( worldMat[0], input.vertex );
	tmp.y = dot( worldMat[1], input.vertex );
	tmp.z = dot( worldMat[2], input.vertex );
	worldPos += tmp * input.blendWeights[@n];
	@property( hlms_normal || hlms_qtangent )
	tmp.x = dot( worldMat[0].xyz, normal );
	tmp.y = dot( worldMat[1].xyz, normal );
	tmp.z = dot( worldMat[2].xyz, normal );
	worldNorm += tmp.xyz * input.blendWeights[@n];@end
	@property( normal_map )
	tmp.x = dot( worldMat[0].xyz, tangent );
	tmp.y = dot( worldMat[1].xyz, tangent );
	tmp.z = dot( worldMat[2].xyz, tangent );
	worldTang += tmp.xyz * input.blendWeights[@n];@end
	@end

    worldPos.w = 1.0;
@end @end

@property( hlms_skeleton )
	@piece( worldViewMat )passBuf.view@end
@end @property( !hlms_skeleton )
    @piece( worldViewMat )worldView@end
@end

@piece( CalculatePsPos )mul( @insertpiece( worldViewMat ), @insertpiece(local_vertex) ).xyz@end

@piece( VertexTransform )
	//Lighting is in view space
	@property( hlms_normal || hlms_qtangent )outVs.pos		= @insertpiece( CalculatePsPos );@end
	@property( hlms_normal || hlms_qtangent )outVs.normal	= mul( (float3x3)@insertpiece( worldViewMat ), @insertpiece(local_normal) );@end
	@property( normal_map )outVs.tangent	= mul( (float3x3)@insertpiece( worldViewMat ), @insertpiece(local_tangent) );@end
@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = mul( passBuf.viewProj, worldPos );@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w	= 1.0f;
	@property( hlms_normal || hlms_qtangent )outVs.gl_Position.xyz	= outVs.pos;@end
	@property( !hlms_normal && !hlms_qtangent )outVs.gl_Position.xyz	= @insertpiece( CalculatePsPos );@end
	float L = length( outVs.gl_Position.xyz );
	outVs.gl_Position.z	+= 1.0f;
	outVs.gl_Position.xy	/= outVs.gl_Position.z;
	outVs.gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);@end
@end
@piece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n = mul( passBuf.shadowRcv[@n].texViewProj, float4(worldPos.xyz, 1.0f) );@end
@end

PS_INPUT main( VS_INPUT input )
{
	PS_INPUT outVs;
@property( !hlms_skeleton )
	float3x4 worldMat = UNPACK_MAT3x4( worldMatBuf, input.drawId @property( !hlms_shadowcaster )<< 1u@end );
	@property( hlms_normal || hlms_qtangent )
    float4x4 worldView = UNPACK_MAT4( worldMatBuf, (input.drawId << 1u) + 1u );
	@end

	float4 worldPos = float4( mul( worldMat, input.vertex ).xyz, 1.0f );
@end

@property( hlms_qtangent )
	//Decode qTangent to TBN with reflection
	float3 normal	= xAxis( normalize( input.qtangent ) );
	@property( normal_map )
	float3 tangent	= yAxis( input.qtangent );
	outVs.biNormalReflection = sign( input.qtangent.w ); //We ensure in C++ qtangent.w is never 0
	@end
@end @property( !hlms_qtangent && hlms_normal )
	float3 normal	= input.normal;
	@property( normal_map )float3 tangent	= input.tangent;@end
@end

	@insertpiece( SkeletonTransform )
	@insertpiece( VertexTransform )
@foreach( hlms_uv_count, n )
	outVs.uv@n = input.uv@n;@end

@property( !hlms_shadowcaster )
	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n.z = (outVs.posL@n.z - passBuf.shadowRcv[@n].shadowDepthRange.x) * passBuf.shadowRcv[@n].shadowDepthRange.y;@end

@property( hlms_pssm_splits )	outVs.depth = outVs.gl_Position.z;@end

	outVs.drawId = input.drawId;
@end @property( hlms_shadowcaster )
	float shadowConstantBias = asfloat( worldMaterialIdx[input.drawId].y );
	//Linear depth
	outVs.depth	= (outVs.gl_Position.z - passBuf.depthRange.x + shadowConstantBias) * passBuf.depthRange.y;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	outVs.gl_Position.z = outVs.gl_Position.z * (outVs.gl_Position.w * passBuf.depthRange.y);
@end

	return outVs;
}
