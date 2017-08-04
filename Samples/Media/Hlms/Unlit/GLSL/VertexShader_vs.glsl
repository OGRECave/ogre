@insertpiece( SetCrossPlatformSettings )
@insertpiece( SetCompatibilityLayer )

out gl_PerVertex
{
	vec4 gl_Position;
@property( hlms_global_clip_distances )
	float gl_ClipDistance[1];
@end
};

layout(std140) uniform;

@insertpiece( Common_Matrix_DeclUnpackMatrix4x4 )

in vec4 vertex;
@property( hlms_colour )in vec4 colour;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end

@property( GL_ARB_base_instance )
	in uint drawId;
@end

@insertpiece( custom_vs_attributes )

@property( !hlms_shadowcaster || !hlms_shadow_uses_depth_texture || exponential_shadow_maps )
out block
{
@insertpiece( VStoPS_block )
} outVs;
@end

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( InstanceDecl )
/*layout(binding = 0) */uniform samplerBuffer worldMatBuf;
@property( texture_matrix )/*layout(binding = 1) */uniform samplerBuffer animationMatrixBuf;@end
@insertpiece( custom_vs_uniformDeclaration )
@property( !GL_ARB_base_instance )uniform uint baseInstance;@end
// END UNIFORM DECLARATION

@property( !hlms_identity_world )
	@piece( worldViewProj )worldViewProj@end
@end @property( hlms_identity_world )
	@property( !hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[@value(hlms_identity_viewproj)]@end
	@end @property( hlms_identity_viewproj_dynamic )
		@piece( worldViewProj )passBuf.viewProj[instance.worldMaterialIdx[drawId].z]@end
	@end
@end

void main()
{
@property( !GL_ARB_base_instance )
    uint drawId = baseInstance + uint( gl_InstanceID );
@end
    
	@insertpiece( custom_vs_preExecution )
	@property( !hlms_identity_world )
		mat4 worldViewProj;
		worldViewProj = UNPACK_MAT4( worldMatBuf, drawId );
	@end

@property( !hlms_dual_paraboloid_mapping )
	gl_Position = vertex * @insertpiece( worldViewProj );
@end

@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	gl_Position.w	= 1.0f;
	gl_Position.xyz	= (vertex * @insertpiece( worldViewProj )).xyz;
	float L = length( gl_Position.xyz );
	gl_Position.z	+= 1.0f;
	gl_Position.xy	/= gl_Position.z;
	gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);
@end

@property( !hlms_shadowcaster )
@property( hlms_colour )	outVs.colour = colour;@end

@property( texture_matrix )	mat4 textureMatrix;@end

@foreach( out_uv_count, n )
	@property( out_uv@n_texture_matrix )
		textureMatrix = UNPACK_MAT4( animationMatrixBuf, (instance.worldMaterialIdx[drawId].x << 4u) + @value( out_uv@n_tex_unit )u );
 		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = (vec4( uv@value( out_uv@n_source_uv ).xy, 0, 1 ) * textureMatrix).xy;
	@end @property( !out_uv@n_texture_matrix )
		outVs.uv@value( out_uv@n_out_uv ).@insertpiece( out_uv@n_swizzle ) = uv@value( out_uv@n_source_uv ).xy;
	@end @end

	outVs.drawId = drawId;

@end

	@property( hlms_global_clip_distances || (hlms_shadowcaster && (exponential_shadow_maps || hlms_shadowcaster_point)) )
		float3 worldPos = (gl_Position * passBuf.invViewProj).xyz;
	@end
	@insertpiece( DoShadowCasterVS )

@property( hlms_global_clip_distances )
	gl_ClipDistance[0] = dot( float4( worldPos.xyz, 1.0 ), passBuf.clipPlane0.xyzw );
@end

	@insertpiece( custom_vs_posExecution )
}
