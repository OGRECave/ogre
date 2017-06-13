@insertpiece( SetCrossPlatformSettings )

// START UNIFORM STRUCT DECLARATION
@insertpiece( PassStructDecl )
@insertpiece( custom_vs_uniformStructDeclaration )
// END UNIFORM STRUCT DECLARATION

struct VS_INPUT
{
	float4 position [[attribute(VES_POSITION)]];
@property( hlms_colour )	float4 colour [[attribute(VES_DIFFUSE)]];@end
@foreach( hlms_uv_count, n )
	float@value( hlms_uv_count@n ) uv@n [[attribute(VES_TEXTURE_COORDINATES@n)]];@end
@property( !iOS )
	ushort drawId [[attribute(15)]];
@end
	@insertpiece( custom_vs_attributes )
};

struct PS_INPUT
{
@insertpiece( VStoPS_block )
	float4 gl_Position [[position]];
@property( hlms_global_clip_distances )
	float gl_ClipDistance0 [[clip_distance]];
@end
};

@property( !hlms_identity_world )
	@piece( worldViewProj )worldViewProj@end
@end @property( hlms_identity_world )
	@property( !hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[@value(hlms_identity_viewproj)]@end
	@end @property( hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[worldMaterialIdx[drawId].z]@end
	@end
@end

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]]
	@property( iOS )
		, ushort instanceId [[instance_id]]
		, constant ushort &baseInstance [[buffer(15)]]
	@end
	// START UNIFORM DECLARATION
	@insertpiece( PassDecl )
	@insertpiece( InstanceDecl )
	, device const float4x4 *worldMatBuf [[buffer(TEX_SLOT_START+0)]]
	@property( texture_matrix ), device const float4x4 *animationMatrixBuf [[buffer(TEX_SLOT_START+1)]]@end
	@insertpiece( custom_vs_uniformDeclaration )
	// END UNIFORM DECLARATION
)
{
	@property( iOS )
		ushort drawId = baseInstance + instanceId;
	@end @property( !iOS )
		ushort drawId = input.drawId;
	@end

	PS_INPUT outVs;
	@insertpiece( custom_vs_preExecution )

	@property( !hlms_identity_world )
		float4x4 worldViewProj;
		worldViewProj = worldMatBuf[drawId];
	@end

@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = input.position * @insertpiece( worldViewProj );
@end

@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w		= 1.0f;
	outVs.gl_Position.xyz	= ( input.position * @insertpiece( worldViewProj ) ).xyz;
	float L = length( outVs.gl_Position.xyz );
	outVs.gl_Position.z		+= 1.0f;
	outVs.gl_Position.xy	/= outVs.gl_Position.z;
	outVs.gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);
@end

@property( !hlms_shadowcaster )
@property( hlms_colour )	outVs.colour = input.colour;@end

@property( texture_matrix )	float4x4 textureMatrix;@end

@foreach( out_uv_count, n )
	@property( out_uv@n_texture_matrix )
		textureMatrix = animationMatrixBuf[(worldMaterialIdx[drawId].x << 4u) + @value( out_uv@n_tex_unit )];
		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = (float4( input.uv@value( out_uv@n_source_uv ).xy, 0, 1 ) * textureMatrix).xy;
	@end @property( !out_uv@n_texture_matrix )
		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = input.uv@value( out_uv@n_source_uv ).xy;
	@end @end

	outVs.materialId = (ushort)worldMaterialIdx[drawId].x;

@end

	@property( hlms_global_clip_distances || (hlms_shadowcaster && (exponential_shadow_maps || hlms_shadowcaster_point)) )
		float3 worldPos = (outVs.gl_Position * passBuf.invViewProj).xyz;
	@end
	@insertpiece( DoShadowCasterVS )

@property( hlms_global_clip_distances )
	outVs.gl_ClipDistance0 = dot( float4( worldPos.xyz, 1.0 ), passBuf.clipPlane0.xyzw );
@end

	@insertpiece( custom_vs_posExecution )

	return outVs;
}
