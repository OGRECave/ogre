@property( GL430 )#version 430 core
@end @property( !GL430 )
#version 330 core
#extension GL_ARB_shading_language_420pack
@end

layout(std140) uniform;

in vec4 vertex;

@property( hlms_normal )in vec3 normal;@end
@property( hlms_qtangent )in vec4 qtangent;@end

@property( normal_map && !hlms_qtangent )
in vec3 tangent;
@property( hlms_binormal )in vec3 binormal;@end
@end

@property( hlms_skeleton )
in vec4 blendIndices;
in vec4 blendWeights;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end

out block
{
@insertpiece( VStoPS_block )
} outVs;

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( MaterialDecl )
@property( !hlms_skeleton || hlms_shadowcaster )@insertpiece( InstanceDecl )@end
layout(binding = 0) uniform samplerBuffer worldMatBuf;
// END UNIFORM DECLARATION

@property( hlms_qtangent )
@insertpiece( DeclQuat_xAxis )
@insertpiece( DeclQuat_yAxis )
@end

@property( !hlms_skeleton )
@piece( local_vertex )vertex@end
@piece( local_normal )normal@end
@piece( local_tangent )tangent@end
@end
@property( hlms_skeleton )
@piece( local_vertex )_localPos@end
@piece( local_normal )_localNorm@end
@piece( local_tangent )_localTang@end
@end

@property( hlms_skeleton )@piece( SkeletonTransform )
	int _idx = int(blendIndices[0] * 3.0);
	uint matStart = (instance.worldMaterialIdx[drawId] & 0xFFFFFE00) >> 9;
	vec4 worldMat[3];
	worldMat[0] = texelFetch( worldMatBuf, matStart + _idx + 0 );
	worldMat[1] = texelFetch( worldMatBuf, matStart + _idx + 1 );
	worldMat[2] = texelFetch( worldMatBuf, matStart + _idx + 2 );
	vec4 _localPos;
	_localPos.x = dot( worldMat[0], vertex );
	_localPos.y = dot( worldMat[1], vertex );
	_localPos.z = dot( worldMat[2], vertex );
	_localPos *= blendWeights[0];
	@property( hlms_normal )vec3 _localNorm;
	_localNorm.x = dot( worldMat[0].xyz, normal );
	_localNorm.y = dot( worldMat[1].xyz, normal );
	_localNorm.z = dot( worldMat[2].xyz, normal );
	_localNorm *= blendWeights[0];@end
	@property( normal_map )vec3 _localTang;
	_localTang.x = dot( worldMat[0].xyz, tangent );
	_localTang.y = dot( worldMat[1].xyz, tangent );
	_localTang.z = dot( worldMat[2].xyz, tangent );
	_localTang *= blendWeights[0];@end

	@psub( NeedsMoreThan1BonePerVertex, hlms_bones_per_vertex, 1 )
	@property( NeedsMoreThan1BonePerVertex )vec4 tmp;@end
	@foreach( hlms_bones_per_vertex, n, 1 )
	_idx = int(blendIndices[@n] * 3.0);
	worldMat[0] = texelFetch( worldMatBuf, matStart + _idx + 0 );
	worldMat[1] = texelFetch( worldMatBuf, matStart + _idx + 1 );
	worldMat[2] = texelFetch( worldMatBuf, matStart + _idx + 2 );
	tmp.x = dot( worldMat[0], vertex );
	tmp.y = dot( worldMat[1], vertex );
	tmp.z = dot( worldMat[2], vertex );
	_localPos += tmp * blendWeights[@n];
	@property( hlms_normal )
	tmp.x = dot( worldMat[0].xyz, normal );
	tmp.y = dot( worldMat[1].xyz, normal );
	tmp.z = dot( worldMat[2].xyz, normal );
	_localNorm += tmp.xyz * blendWeights[@n];@end
	@property( normal_map )
	tmp.x = dot( worldMat[0].xyz, tangent );
	tmp.y = dot( worldMat[1].xyz, tangent );
	tmp.z = dot( worldMat[2].xyz, tangent );
	_localTang += tmp.xyz * blendWeights[@n];@end
	@end

	_localPos.w = 1.0;
@end @end

@property( hlms_skeleton )
	@piece( worldViewMat )pass.view@end
	@piece( worldViewProjMat )pass.viewProj@end
@end @property( !hlms_skeleton )
	@piece( worldViewMat )worldView@end
	@piece( worldViewProjMat )worldViewProj@end
@end

@piece( CalculatePsPos )(@insertpiece( worldViewMat ) * @insertpiece(local_vertex)).xyz@end

@piece( VertexTransform )
	//Lighting is in view space
	@property( hlms_normal )outVs.pos		= @insertpiece( CalculatePsPos );@end
	@property( hlms_normal )outVs.normal	= mat3(@insertpiece( worldViewMat )) * @insertpiece(local_normal);@end
	@property( normal_map )outVs.tangent	= mat3(@insertpiece( worldViewMat )) * @insertpiece(local_tangent);@end
@property( !hlms_dual_paraboloid_mapping )
	gl_Position = @insertpiece( worldViewProjMat ) * @insertpiece(local_vertex);@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	gl_Position.w	= 1.0f;
	@property( hlms_normal )gl_Position.xyz	= outVs.pos;@end
	@property( !hlms_normal )gl_Position.xyz	= @insertpiece( CalculatePsPos );@end
	float L = length( gl_Position.xyz );
	gl_Position.z	+= 1.0f;
	gl_Position.xy	/= gl_Position.z;
	gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);@end
@end
@piece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n = pass.shadowRcv[@n].texWorldViewProj * @insertpiece(local_vertex);@end
@end

void main()
{
@property( !hlms_skeleton )
	mat4 worldViewProj;
	worldViewProj[0] = texelFetch( worldMatBuf, drawId * 2 );
	worldViewProj[1] = texelFetch( worldMatBuf, drawId * 2 + 1 );
	worldViewProj[2] = texelFetch( worldMatBuf, drawId * 2 + 2 );
	worldViewProj[3] = texelFetch( worldMatBuf, drawId * 2 + 3 );
	@property( hlms_normal )
	mat4 worldView;
	worldView[0] = texelFetch( worldMatBuf, drawId * 2 + 4 );
	worldView[1] = texelFetch( worldMatBuf, drawId * 2 + 5 );
	worldView[2] = texelFetch( worldMatBuf, drawId * 2 + 6 );
	worldView[3] = texelFetch( worldMatBuf, drawId * 2 + 7 );
	@end
@end

@property( hlms_qtangent )
	//Decode qTangent to TBN with reflection
	vec3 normal		= xAxis( qtangent );
	vec3 tangent	= yAxis( qtangent );
	outVs.biNormalReflection = sign( qtangent.w ); //We ensure in C++ qtangent.w is never 0
@end

	@insertpiece( SkeletonTransform )
	@insertpiece( VertexTransform )
@foreach( hlms_uv_count, n )
	psUv@n = uv@n;@end

@property( !hlms_shadowcaster )
	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	outVs.posL@n.z = (outVs.posL@n.z - pass.shadowRcv[@n].shadowDepthRange.x) * pass.shadowRcv[@n].shadowDepthRange.y;@end

@property( hlms_pssm_splits )	outVs.depth = gl_Position.z;@end
@end @property( hlms_shadowcaster )
	//Linear depth
	outVs.depth	= (gl_Position.z - pass.depthRange.x + instance.shadowConstantBias) * pass.depthRange.y;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	gl_Position.z = gl_Position.z * (gl_Position.w * pass.depthRange.y);
@end
}
