
@piece( TerraMaterialDecl )
layout_constbuffer(binding = 1) uniform MaterialBuf
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	vec4 kD; //kD.w is padding
	vec4 roughness;
	vec4 metalness;
	vec4 detailOffsetScale[4];

	@insertpiece( custom_materialBuffer )
} material;
@end

@piece( TerraInstanceDecl )
struct CellData
{
	//.x = numVertsPerLine
	//.y = lodLevel
	//.z = vao->getPrimitiveCount() / m_verticesPerLine - 2u
	//.w = skirtY (float)
	uvec4 numVertsPerLine;
	ivec4 xzTexPosBounds;		//XZXZ
	vec4 pos;		//.w contains 1.0 / xzTexPosBounds.z
	vec4 scale;		//.w contains 1.0 / xzTexPosBounds.w
};

layout_constbuffer(binding = 2) uniform InstanceBuffer
{
	CellData cellData[256];
} instance;
@end

@piece( Terra_VStoPS_block )
	//flat uint drawId;
	vec3 pos;
	vec2 uv0;

	@foreach( hlms_num_shadow_map_lights, n )
		@property( !hlms_shadowmap@n_is_point_light )
			vec4 posL@n;@end @end
	@property( hlms_pssm_splits )float depth;@end
	@insertpiece( custom_VStoPS )
@end
