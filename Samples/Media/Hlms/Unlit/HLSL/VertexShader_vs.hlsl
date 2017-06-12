@insertpiece( SetCrossPlatformSettings )

@insertpiece( Common_Matrix_DeclUnpackMatrix4x4 )

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( InstanceDecl )
Buffer<float4> worldMatBuf : register(t0);
@property( texture_matrix )Buffer<float4> animationMatrixBuf : register(t1);@end
@insertpiece( custom_vs_uniformDeclaration )
// END UNIFORM DECLARATION

struct VS_INPUT
{
	float4 vertex : POSITION;
@property( hlms_colour )	float4 colour : COLOR0;@end
@foreach( hlms_uv_count, n )
	float@value( hlms_uv_count@n ) uv@n : TEXCOORD@n;@end
	uint drawId : DRAWID;
	@insertpiece( custom_vs_attributes )
};

struct PS_INPUT
{
@insertpiece( VStoPS_block )
	float4 gl_Position : SV_Position;
@property( hlms_global_clip_distances )
	float gl_ClipDistance0 : SV_ClipDistance0;
@end
};

@property( !hlms_identity_world )
	@piece( worldViewProj )worldViewProj@end
@end @property( hlms_identity_world )
	@property( !hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[@value(hlms_identity_viewproj)]@end
	@end @property( hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[worldMaterialIdx[input.drawId].z]@end
	@end
@end

PS_INPUT main( VS_INPUT input )
{
	PS_INPUT outVs;
	@insertpiece( custom_vs_preExecution )

	@property( !hlms_identity_world )
		float4x4 worldViewProj;
		worldViewProj = UNPACK_MAT4( worldMatBuf, input.drawId );
	@end

@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = mul( input.vertex, @insertpiece( worldViewProj ) );
@end

@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w		= 1.0f;
	outVs.gl_Position.xyz	= mul( input.vertex, @insertpiece( worldViewProj ) ).xyz;
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
		textureMatrix = UNPACK_MAT4( animationMatrixBuf, (worldMaterialIdx[input.drawId].x << 4u) + @value( out_uv@n_tex_unit ) );
		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = mul( float4( input.uv@value( out_uv@n_source_uv ).xy, 0, 1 ), textureMatrix ).xy;
	@end @property( !out_uv@n_texture_matrix )
		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = input.uv@value( out_uv@n_source_uv ).xy;
	@end @end

	outVs.drawId = input.drawId;

@end

	@property( hlms_global_clip_distances || (hlms_shadowcaster && (exponential_shadow_maps || hlms_shadowcaster_point)) )
		float3 worldPos = mul(outVs.gl_Position, passBuf.invViewProj).xyz;
	@end
	@insertpiece( DoShadowCasterVS )

@property( hlms_global_clip_distances )
	outVs.gl_ClipDistance0 = dot( float4( worldPos.xyz, 1.0 ), passBuf.clipPlane0.xyzw );
@end

	@insertpiece( custom_vs_posExecution )

	return outVs;
}
