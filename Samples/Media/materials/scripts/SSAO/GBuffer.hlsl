// courtesy of the ogre wiki and the defered shading sample
// http://www.ogre3d.org/wiki/index.php/Deferred_Shading
//------------------------------------------------------------------------------

struct v2p
{
    float4 oPosition : SV_POSITION;
    float3 oViewPos : TEXCOORD0;
    float3 oNormal : TEXCOORD1;
};

v2p GBuffer_vp(
        float4 iPosition : POSITION,
        float3 iNormal   : NORMAL,

        uniform float4x4 cWorldViewProj,
        uniform float4x4 cWorldView
        )
{
	v2p output;
	output.oPosition = mul(cWorldViewProj, iPosition);         // transform the vertex position to the projection space
	output.oViewPos = mul(cWorldView, iPosition).xyz;          // transform the vertex position to the view space
	output.oNormal = mul(cWorldView, float4(iNormal,0)).xyz;   // transform the vertex normal to view space
	return output;
}

struct  NV // normal + view
{
    float4 oNormalDepth : SV_Target0;	// normal + linear depth [0, 1]
    float4 oViewPos : SV_Target1;		// view space position	
};

NV GBuffer_fp(
		float4 position : SV_POSITION,
        float3 iViewPos : TEXCOORD0,
        float3 iNormal  : TEXCOORD1,
      
        uniform float cNearClipDistance,
        uniform float cFarClipDistance		// !!! might be 0 for infinite view projection.
        )
{
    	float clipDistance = cFarClipDistance - cNearClipDistance;
		NV output;
		output.oNormalDepth = float4(normalize(iNormal).xyz, (length(iViewPos) - cNearClipDistance) / clipDistance);
		output.oViewPos = float4(iViewPos, 0);
		return output;
}