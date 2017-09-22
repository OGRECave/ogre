@piece( TerraMaterialDecl )
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	float4 kD; //kD.w is padding
	float4 roughness;
	float4 metalness;
	float4 detailOffsetScale[4];

	@insertpiece( custom_materialBuffer )
};

cbuffer MaterialBuf : register(b1)
{
	Material material;
};
@end


@piece( TerraInstanceDecl )
struct CellData
{
	//.x = numVertsPerLine
	//.y = lodLevel
	//.z = vao->getPrimitiveCount() / m_verticesPerLine - 2u
	//.w = skirtY (float)
	uint4 numVertsPerLine;
	int4  xzTexPosBounds;		//XZXZ
	float4 pos;		//.w contains 1.0 / xzTexPosBounds.z
	float4 scale;	//.w contains 1.0 / xzTexPosBounds.w
};
//Uniforms that change per Item/Entity
cbuffer InstanceBuffer : register(b2)
{
	CellData cellDataArray[256];
};
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( Terra_VStoPS_block )
	float3 pos : TEXCOORD@counter(texcoord);
	float2 uv0 : TEXCOORD@counter(texcoord);
	@foreach( hlms_num_shadow_map_lights, n )
		@property( !hlms_shadowmap@n_is_point_light )
			float4 posL@n	: TEXCOORD@counter(texcoord);@end @end

	@property( hlms_pssm_splits )float depth	: TEXCOORD@counter(texcoord);@end
	@insertpiece( custom_VStoPS )
@end
