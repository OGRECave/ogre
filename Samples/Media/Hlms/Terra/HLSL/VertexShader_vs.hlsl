//To render a 2x2 (quads) terrain:
//You'll normally need 6 vertices per line + 2 for degenerates.
//You'll need 8 vertices per line.
//So you'll need a total of 16 vertices.

//To render a 4x2 (quads) terrain:
//You'll need 10 vertices per line.
//If we include degenerate vertices, you'll need 12 per line
//So you'll need a total of 24 vertices.
//in int gl_VertexID;

struct VS_INPUT
{
	uint gl_VertexID : SV_VertexID;
	uint drawId : DRAWID;
	@insertpiece( custom_vs_attributes )
};

struct PS_INPUT
{
@insertpiece( Terra_VStoPS_block )
	float4 gl_Position: SV_Position;
};

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( TerraInstanceDecl )
Texture2D<float> heightMap: register(t0);
@insertpiece( custom_vs_uniformDeclaration )
// END UNIFORM DECLARATION

@piece( VertexTransform )
	//Lighting is in view space
	outVs.pos		= mul( passBuf.view, float4( worldPos.xyz, 1.0f ) ).xyz;
@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = mul( passBuf.viewProj, float4( worldPos.xyz, 1.0f ) );@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w	= 1.0f;
	outVs.gl_Position.xyz	= outVs.pos;
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
	@insertpiece( custom_vs_preExecution )
    CellData cellData = cellDataArray[input.drawId];

	//Map pointInLine from range [0; 12) to range [0; 9] so that it reads:
	// 0 0 1 2 3 4 5 6 7 8 9 9
	uint pointInLine = input.gl_VertexID % (cellData.numVertsPerLine.x); //cellData.numVertsPerLine.x = 12
	pointInLine = uint(clamp( int(pointInLine) - 1, 0, int(cellData.numVertsPerLine.x - 3u) ));

	uint2 uVertexPos;

	uVertexPos.x = pointInLine >> 1u;
    //Even numbers are the next line, odd numbers are current line.
	uVertexPos.y = (pointInLine & 0x01u) == 0u ? 1u : 0u;
	uVertexPos.y += input.gl_VertexID / cellData.numVertsPerLine.x;
	//uVertexPos.y += floor( (float)input.gl_VertexID / (float)cellData.numVertsPerLine ); Could be faster on GCN.

@property( use_skirts )
	//Apply skirt.
	bool isSkirt =( pointInLine.x <= 1u ||
					pointInLine.x >= (cellData.numVertsPerLine.x - 4u) ||
					uVertexPos.y == 0u ||
					uVertexPos.y == (cellData.numVertsPerLine.z + 2u) );

	//Now shift X position for the left & right skirts
	uVertexPos.x = uint( max( int(uVertexPos.x) - 1, 0 ) );
	uVertexPos.x = min( uVertexPos.x, ((cellData.numVertsPerLine.x - 7u) >> 1u) );
	// uVertexPos.x becomes:
	// 0 0 0 1 1 2 2 3 3 4 4 4
	// 0 0 0 0 0 1 1 2 2 3 3 3
	// 0 0 0 0 0 1 1 2 2 2 2 2

	//Now shift Y position for the front & back skirts
	uVertexPos.y = uint( max( int(uVertexPos.y) - 1, 0 ) );
	uVertexPos.y = min( uVertexPos.y, cellData.numVertsPerLine.z );
@end

	uint lodLevel = cellData.numVertsPerLine.y;
	uVertexPos = uVertexPos << lodLevel;

	uVertexPos.xy = uint2( clamp( int2(uVertexPos.xy) + cellData.xzTexPosBounds.xy,
                           int2( 0, 0 ), cellData.xzTexPosBounds.zw ) );

    float3 worldPos;
	worldPos.y = heightMap.Load( int3( uVertexPos.xy, 0 ) ).x;
@property( use_skirts )
	worldPos.y = isSkirt ? asfloat(cellData.numVertsPerLine.w) : worldPos.y;
@end
	worldPos.xz = uVertexPos.xy;
    worldPos.xyz = worldPos.xyz * cellData.scale.xyz + cellData.pos.xyz;

	@insertpiece( VertexTransform )

	outVs.uv0.xy = float2( uVertexPos.xy ) * float2( cellData.pos.w, cellData.scale.w );

	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n.z = outVs.posL@n.z * passBuf.shadowRcv[@n].shadowDepthRange.y;@end

@property( hlms_pssm_splits )	outVs.depth = outVs.gl_Position.z;@end

	//outVs.drawId = input.drawId;

	@insertpiece( custom_vs_posExecution )

	return outVs;
}
