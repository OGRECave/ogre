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

#version 330

uniform sampler2D heightMap;

uniform vec2 heightMapResolution;
uniform vec3 vScale;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	ivec2 iCoord = ivec2( inPs.uv0 * heightMapResolution );

	ivec3 xN01;
	xN01.x = max( iCoord.x - 1, 0 );
	xN01.y = iCoord.x;
	xN01.z = min( iCoord.x + 1, int(heightMapResolution.x) );
	ivec3 yN01;
	yN01.x = max( iCoord.y - 1, 0 );
	yN01.y = iCoord.y;
	yN01.z = min( iCoord.y + 1, int(heightMapResolution.y) );

	//Watch out! It's heightXY, but texelFetch uses YX.
	float heightNN = texelFetch( heightMap, ivec2( xN01.x, yN01.x ), 0 ).x * vScale.y;
	float heightN0 = texelFetch( heightMap, ivec2( xN01.y, yN01.x ), 0 ).x * vScale.y;
	//float heightN1 = texelFetch( heightMap, ivec2( xN01.z, yN01.x ), 0 ).x * vScale.y;

	float height0N = texelFetch( heightMap, ivec2( xN01.x, yN01.y ), 0 ).x * vScale.y;
	float height00 = texelFetch( heightMap, ivec2( xN01.y, yN01.y ), 0 ).x * vScale.y;
	float height01 = texelFetch( heightMap, ivec2( xN01.z, yN01.y ), 0 ).x * vScale.y;

	//float height1N = texelFetch( heightMap, ivec2( xN01.x, yN01.z ), 0 ).x * vScale.y;
	float height10 = texelFetch( heightMap, ivec2( xN01.y, yN01.z ), 0 ).x * vScale.y;
	float height11 = texelFetch( heightMap, ivec2( xN01.z, yN01.z ), 0 ).x * vScale.y;

	vec3 vNN = vec3( -vScale.x, heightNN, -vScale.z );
	vec3 vN0 = vec3( -vScale.x, heightN0,  0 );
	//vec3 vN1 = vec3( -vScale.x, heightN1,  vScale.z );

	vec3 v0N = vec3(  0, height0N, -vScale.z );
	vec3 v00 = vec3(  0, height00,  0 );
	vec3 v01 = vec3(  0, height01,  vScale.z );

	//vec3 v1N = vec3(  vScale.x, height1N, -vScale.z );
	vec3 v10 = vec3(  vScale.x, height10,  0 );
	vec3 v11 = vec3(  vScale.x, height11,  vScale.z );

	vec3 vNormal = vec3( 0, 0, 0 );

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

	//fragColour.xy = vNormal.zx;
	fragColour = vec4( vNormal.zyx * 0.5 + 0.5, 1.0f );
}
