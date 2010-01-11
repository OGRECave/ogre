
void generalPurposeProjection_vp(
		float4 pos			: POSITION,
		
		out float4 oPos		: POSITION,
		out float4 texCoord : TEXCOORD0,

		uniform float4x4 worldViewProjMatrix,
		uniform float4x4 worldMatrix,
		uniform float4x4 texViewProjMatrix)
{
	oPos = mul(worldViewProjMatrix, pos);
	// multiply position by world matrix, then by projective view/proj
	float4 newpos = mul(worldMatrix, pos);
	texCoord = mul(texViewProjMatrix, newpos);
}

void generalPurposeProjection_fp(
		float4 texCoord		: TEXCOORD0,
		out float4 col		: COLOR,
		uniform sampler2D texMap)
{
	col = tex2Dproj(texMap, texCoord);
}
		

