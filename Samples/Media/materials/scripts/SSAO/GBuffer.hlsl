// courtesy of the ogre wiki and the defered shading sample
// http://www.ogre3d.org/wiki/index.php/Deferred_Shading
//------------------------------------------------------------------------------

struct v2p
{
    float4 oPosition : SV_POSITION;
    float3 oViewPos : TEXCOORD0;
    float3 oNormal : TEXCOORD1;
};

struct vertexIn
{
    float4 iPosition : POSITION;
    float3 iNormal   : NORMAL;
};

v2p GBuffer_vp(
		vertexIn input,
        uniform matrix cWorldViewProj,
        uniform matrix cWorldView
        )
{
	v2p output;
	output.oPosition = mul(cWorldViewProj, input.iPosition);         // transform the vertex position to the projection space
	output.oViewPos = mul(cWorldView, input.iPosition).xyz;          // transform the vertex position to the view space
	output.oNormal = mul(cWorldView, float4(input.iNormal,0)).xyz;   // transform the vertex normal to view space
	return output;
}

struct  NV // normal + view
{
    float4 oNormalDepth : SV_Target0;	// normal + linear depth [0, 1]
    float4 oViewPos : SV_Target1;		// view space position	
};

NV GBuffer_fp(
		v2p input,
        uniform float cNearClipDistance,
        uniform float cFarClipDistance		// !!! might be 0 for infinite view projection.
        )
{
    	float clipDistance = cFarClipDistance - cNearClipDistance;
		NV output;
		output.oNormalDepth = float4(normalize(input.oNormal).xyz, (length(input.oViewPos) - cNearClipDistance) / clipDistance);
		output.oViewPos = float4(input.oViewPos, 0);
		return output;
}