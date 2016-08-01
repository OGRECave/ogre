@insertpiece( SetCrossPlatformSettings )

@insertpiece( Common_Matrix_DeclUnpackMatrix4x4 )
@insertpiece( Common_Matrix_DeclUnpackMatrix3x4 )

@property( hlms_normal || hlms_qtangent || normal_map )
	@insertpiece( Common_Matrix_Conversions )
@end

struct VS_INPUT
{
	float4 position [[attribute(VES_POSITION)]];
@property( hlms_normal )	float3 normal [[attribute(VES_NORMAL)]];@end
@property( hlms_qtangent )	float4 qtangent [[attribute(VES_NORMAL)]];@end

@property( normal_map && !hlms_qtangent )
	float3 tangent	[[attribute(VES_TANGENT)]];
	@property( hlms_binormal )float3 binormal	[[attribute(VES_BINORMAL)]];@end
@end

@property( hlms_skeleton )
	uint4 blendIndices	[[attribute(VES_BLEND_INDICES)]];
	float4 blendWeights [[attribute(VES_BLEND_WEIGHTS)]];@end

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

// START UNIFORM STRUCT DECLARATION
@insertpiece( PassStructDecl )
@insertpiece( custom_vs_uniformStructDeclaration )
// END UNIFORM STRUCT DECLARATION

@property( hlms_qtangent )
@insertpiece( DeclQuat_xAxis )
@property( normal_map )
@insertpiece( DeclQuat_yAxis )
@end @end

@property( !hlms_skeleton )
@piece( local_vertex )input.position@end
@piece( local_normal )normal@end
@piece( local_tangent )tangent@end
@end
@property( hlms_skeleton )
@piece( local_vertex )worldPos@end
@piece( local_normal )worldNorm@end
@piece( local_tangent )worldTang@end
@end

@property( hlms_skeleton )@piece( SkeletonTransform )
	int _idx = int((input.blendIndices[0] << 1u) + input.blendIndices[0]); //blendIndices[0] * 3u; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		int matStart = int(worldMaterialIdx[drawId].x >> 9u);
	float4 worldMat[3];
		worldMat[0] = worldMatBuf[matStart + _idx + 0u];
		worldMat[1] = worldMatBuf[matStart + _idx + 1u];
		worldMat[2] = worldMatBuf[matStart + _idx + 2u];
	float4 worldPos;
	worldPos.x = dot( worldMat[0], input.position );
	worldPos.y = dot( worldMat[1], input.position );
	worldPos.z = dot( worldMat[2], input.position );
	worldPos.xyz *= input.blendWeights[0];
	@property( hlms_normal || hlms_qtangent )float3 worldNorm;
	worldNorm.x = dot( worldMat[0].xyz, normal );
	worldNorm.y = dot( worldMat[1].xyz, normal );
	worldNorm.z = dot( worldMat[2].xyz, normal );
	worldNorm *= input.blendWeights[0];@end
	@property( normal_map )float3 worldTang;
	worldTang.x = dot( worldMat[0].xyz, tangent );
	worldTang.y = dot( worldMat[1].xyz, tangent );
	worldTang.z = dot( worldMat[2].xyz, tangent );
	worldTang *= input.blendWeights[0];@end

	@psub( NeedsMoreThan1BonePerVertex, hlms_bones_per_vertex, 1 )
	@property( NeedsMoreThan1BonePerVertex )float4 tmp;
	tmp.w = 1.0;@end //!NeedsMoreThan1BonePerVertex
	@foreach( hlms_bones_per_vertex, n, 1 )
	_idx = (input.blendIndices[@n] << 1u) + input.blendIndices[@n]; //blendIndices[@n] * 3; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		worldMat[0] = worldMatBuf[matStart + _idx + 0u];
		worldMat[1] = worldMatBuf[matStart + _idx + 1u];
		worldMat[2] = worldMatBuf[matStart + _idx + 2u];
	tmp.x = dot( worldMat[0], input.position );
	tmp.y = dot( worldMat[1], input.position );
	tmp.z = dot( worldMat[2], input.position );
	worldPos.xyz += (tmp * input.blendWeights[@n]).xyz;
	@property( hlms_normal || hlms_qtangent )
	tmp.x = dot( worldMat[0].xyz, normal );
	tmp.y = dot( worldMat[1].xyz, normal );
	tmp.z = dot( worldMat[2].xyz, normal );
	worldNorm += tmp.xyz * input.blendWeights[@n];@end
	@property( normal_map )
	tmp.x = dot( worldMat[0].xyz, tangent );
	tmp.y = dot( worldMat[1].xyz, tangent );
	tmp.z = dot( worldMat[2].xyz, tangent );
	worldTang += tmp.xyz * input.blendWeights[@n];@end
	@end

	worldPos.w = 1.0;
@end @end  //SkeletonTransform // !hlms_skeleton

@property( hlms_skeleton )
	@piece( worldViewMat )pass.view@end
@end @property( !hlms_skeleton )
	@piece( worldViewMat )worldView@end
@end

@piece( CalculatePsPos )( @insertpiece(local_vertex) * @insertpiece( worldViewMat ) ).xyz@end

@piece( VertexTransform )
	//Lighting is in view space
	@property( hlms_normal || hlms_qtangent || normal_map )float3x3 mat3x3 = toMat3x3( @insertpiece( worldViewMat ) );@end
	@property( hlms_normal || hlms_qtangent )outVs.pos		= @insertpiece( CalculatePsPos );@end
	@property( hlms_normal || hlms_qtangent )outVs.normal	= @insertpiece(local_normal) * mat3x3;@end
	@property( normal_map )outVs.tangent	= @insertpiece(local_tangent) * mat3x3;@end
@property( !hlms_dual_paraboloid_mapping )
	outVs.gl_Position = worldPos * pass.viewProj;@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	outVs.gl_Position.w	= 1.0f;
	@property( hlms_normal || hlms_qtangent )outVs.gl_Position.xyz	= outVs.pos;@end
	@property( !hlms_normal && !hlms_qtangent )outVs.gl_Position.xyz	= @insertpiece( CalculatePsPos );@end
	float L = length( outVs.gl_Position.xyz );
	outVs.gl_Position.z	+= 1.0f;
	outVs.gl_Position.xy	/= outVs.gl_Position.z;
	outVs.gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);@end
@end
@piece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n = float4(worldPos.xyz, 1.0f) * pass.shadowRcv[@n].texViewProj;@end
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
	@property( hlms_skeleton || hlms_shadowcaster )@insertpiece( InstanceDecl )@end
	, device const float4 *worldMatBuf [[buffer(TEX_SLOT_START+0)]]
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
@property( !hlms_skeleton )
	float3x4 worldMat = UNPACK_MAT3x4( worldMatBuf, drawId @property( !hlms_shadowcaster )<< 1u@end );
	@property( hlms_normal || hlms_qtangent )
	float4x4 worldView = UNPACK_MAT4( worldMatBuf, (drawId << 1u) + 1u );
	@end

	float4 worldPos = float4( ( input.position * worldMat ).xyz, 1.0f );
@end

@property( hlms_qtangent )
	//Decode qTangent to TBN with reflection
	float3 normal	= xAxis( normalize( input.qtangent ) );
	@property( normal_map )
	float3 tangent	= yAxis( input.qtangent );
	outVs.biNormalReflection = sign( input.qtangent.w ); //We ensure in C++ qtangent.w is never 0
	@end
@end @property( !hlms_qtangent && hlms_normal )
	float3 normal	= input.normal;
	@property( normal_map )float3 tangent	= input.tangent;@end
@end

	@insertpiece( SkeletonTransform )
	@insertpiece( VertexTransform )

@property( !hlms_shadowcaster )
	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n.z = outVs.posL@n.z * pass.shadowRcv[@n].shadowDepthRange.y;@end

@property( hlms_pssm_splits )	outVs.depth = outVs.gl_Position.z;@end

@end @property( hlms_shadowcaster )
	float shadowConstantBias = as_type<float>( worldMaterialIdx[drawId].y );

	@property( !hlms_shadow_uses_depth_texture )
		//Linear depth
		outVs.depth	= (outVs.gl_Position.z + shadowConstantBias * pass.depthRange.y) * pass.depthRange.y;
	@end

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	outVs.gl_Position.z = (outVs.gl_Position.z + shadowConstantBias * pass.depthRange.y) * pass.depthRange.y * outVs.gl_Position.w;
@end

	/// hlms_uv_count will be 0 on shadow caster passes w/out alpha test
@foreach( hlms_uv_count, n )
	outVs.uv@n = input.uv@n;@end

@property( !hlms_shadowcaster || alpha_test )
	outVs.drawId = drawId;@end

	@insertpiece( custom_vs_posExecution )

	return outVs;
}
