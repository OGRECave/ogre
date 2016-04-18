// Our terrain has the following pattern:
//
//	 1N  10   11
//		o-o-o
//		|/|/|
//	 0N	o-+-o 01
//		|/|/|
//		o-o-o
//	 NN  N0   N1
//
// We need to calculate the normal of the vertex in
// the center '+', which is shared by 6 triangles.

Texture2D<float> heightMap;

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main
(
	PS_INPUT inPs,
	uniform float2 heightMapResolution,
	uniform float3 vScale
) : SV_Target
{
	int2 iCoord = int2( inPs.uv0 * heightMapResolution );

	int3 xN01;
	xN01.x = max( iCoord.x - 1, 0 );
	xN01.y = iCoord.x;
	xN01.z = min( iCoord.x + 1, int(heightMapResolution.x) );
	int3 yN01;
	yN01.x = max( iCoord.y - 1, 0 );
	yN01.y = iCoord.y;
	yN01.z = min( iCoord.y + 1, int(heightMapResolution.y) );

	//Watch out! It's heightXY, but heightMap.Load uses YX.
	float heightNN = heightMap.Load( int3( xN01.x, yN01.x, 0 ) ).x * vScale.y;
	float heightN0 = heightMap.Load( int3( xN01.y, yN01.x, 0 ) ).x * vScale.y;
	//float heightN1 = heightMap.Load( int3( xN01.z, yN01.x, 0 ) ).x * vScale.y;

	float height0N = heightMap.Load( int3( xN01.x, yN01.y, 0 ) ).x * vScale.y;
	float height00 = heightMap.Load( int3( xN01.y, yN01.y, 0 ) ).x * vScale.y;
	float height01 = heightMap.Load( int3( xN01.z, yN01.y, 0 ) ).x * vScale.y;

	//float height1N = heightMap.Load( int3( xN01.x, yN01.z, 0 ) ).x * vScale.y;
	float height10 = heightMap.Load( int3( xN01.y, yN01.z, 0 ) ).x * vScale.y;
	float height11 = heightMap.Load( int3( xN01.z, yN01.z, 0 ) ).x * vScale.y;

	float3 vNN = float3( -vScale.x, heightNN, -vScale.z );
	float3 vN0 = float3( -vScale.x, heightN0,  0 );
	//float3 vN1 = float3( -vScale.x, heightN1,  vScale.z );

	float3 v0N = float3(  0, height0N, -vScale.z );
	float3 v00 = float3(  0, height00,  0 );
	float3 v01 = float3(  0, height01,  vScale.z );

	//float3 v1N = float3(  vScale.x, height1N, -vScale.z );
	float3 v10 = float3(  vScale.x, height10,  0 );
	float3 v11 = float3(  vScale.x, height11,  vScale.z );

	float3 vNormal = float3( 0, 0, 0 );

	vNormal += cross( (v01 - v00), (v11 - v00) );
	vNormal += cross( (v11 - v00), (v10 - v00) );
	vNormal += cross( (v10 - v00), (v0N - v00) );
	vNormal += cross( (v0N - v00), (vNN - v00) );
	vNormal += cross( (vNN - v00), (vN0 - v00) );
	vNormal += cross( (vN0 - v00), (v01 - v00) );

//	vNormal += cross( (v01 - v00), (v11 - v00) );
//	vNormal += cross( (v11 - v00), (v10 - v00) );
//	vNormal += cross( (v10 - v00), (v1N - v00) );
//	vNormal += cross( (v1N - v00), (v0N - v00) );
//	vNormal += cross( (v0N - v00), (vNN - v00) );
//	vNormal += cross( (vNN - v00), (vN0 - v00) );
//	vNormal += cross( (vN0 - v00), (vN1 - v00) );
//	vNormal += cross( (vN1 - v00), (v01 - v00) );

	vNormal = normalize( vNormal );

	//return vNormal.zx;
	return float4( vNormal.zyx * 0.5f + 0.5f, 1.0f );
}
