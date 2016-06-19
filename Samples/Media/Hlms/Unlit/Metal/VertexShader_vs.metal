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
};

@property( !hlms_identity_world )
	@piece( worldViewProj )worldViewProj@end
@end @property( hlms_identity_world )
	@property( !hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )pass.viewProj[@value(hlms_identity_viewproj)]@end
	@end @property( hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )pass.viewProj[materialIdx[drawId].z]@end
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
	, device float4x4 *worldMatBuf [[buffer(24)]]
	@property( texture_matrix ), device float4x4 *animationMatrixBuf [[buffer(25)]]@end
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
	@property( out_uv@_texture_matrix )textureMatrix = animationMatrixBuf[(materialIdx[drawId].x << 4u) + @value( out_uv@n_tex_unit )];@end
	outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) =
@property( out_uv@_texture_matrix )
			input.uv@value( out_uv@n_source_uv ).xy * textureMatrix;
@end @property( !out_uv@_texture_matrix )
			input.uv@value( out_uv@n_source_uv ).xy;@end @end

	outVs.drawId = drawId;

@end @property( hlms_shadowcaster )
	float shadowConstantBias = asfloat( materialIdx[drawId].y );
	//Linear depth
	outVs.depth	= (outVs.gl_Position.z - pass.depthRange.x + shadowConstantBias * pass.depthRange.y) * pass.depthRange.y;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	outVs.gl_Position.z = outVs.gl_Position.z * (outVs.gl_Position.w * pass.depthRange.y);
@end

	@insertpiece( custom_vs_posExecution )

	return outVs;
}
