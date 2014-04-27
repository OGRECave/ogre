#version 330

in vec4 vertex;

@property( hlms_normal )in vec3 normal;@end
@property( hlms_qtangent )in vec4 normal;@end

@property( normal_map && !hlms_qtangent )
in vec3 tangent;
@property( hlms_binormal )in vec3 binormal;@end
@end
@property( hlms_normal )
out vec3 psPos;
out vec3 psNormal;
@property( normal_map )out vec3 psTangent;@end
@end

@property( hlms_skeleton )
in vec4 blendIndices;
in vec4 blendWeights;@end

@property( hlms_shadowcaster || hlms_pssm_splits )out float psDepth;@end

@foreach( hlms_uv_count, n )
in vec@value( hlms_uv_count@n ) uv@n;@end
@foreach( hlms_uv_count, n )
out vec@value( hlms_uv_count@n ) psUv@n;@end

@foreach( hlms_num_shadow_maps, n )
out vec4 psPosL@n;@end

// START UNIFORM DECLARATION
@property( hlms_num_shadow_maps )
//Uniforms that change per pass
uniform mat4 texWorldViewProj[@value(hlms_num_shadow_maps)];
uniform vec4 shadowDepthRange[@value(hlms_num_shadow_maps)];@end
@property( hlms_shadowcaster )uniform vec4 depthRange;@end
//Uniforms that change per entity
@property( hlms_skeleton )uniform mat4x3 worldMat[60];@end
@property( !hlms_shadowcaster )uniform mat4 worldView;@end
uniform mat4 worldViewProj;
@property( hlms_shadowcaster )uniform float shadowConstantBias;@end
// END UNIFORM DECLARATION

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
	int _idx = (int)(blendIndices[0]);
	vec4 _localPos = (worldMat[_idx] * vertex) * blendWeights[0];
	@property( hlms_normal )vec4 _localNorm = (mat3(worldMat[_idx]) * normal) * blendWeights[0];@end
	@property( normal_map )vec4 _localTang = (mat3(worldMat[_idx]) * tangent) * blendWeights[0];@end

	for( int i=0; i<@value( hlms_bones_per_vertex ); ++i )
	{
		_idx = (int)(blendIndices[i]);
		_localPos += (worldMat[_idx] * vertex) * blendWeights[i];
		@property( hlms_normal )_localNorm += (mat3(worldMat[_idx]) * normal) * blendWeights[i];@end
		@property( normal_map )_localTang += (mat3(worldMat[_idx]) * tangent) * blendWeights[i];@end
	}
@end@end

@piece( CalculatePsPos )(worldView * @insertpiece(local_vertex)).xyz@end

@piece( VertexTransform )
	//Lighting is in view space
	@property( hlms_normal )psPos		= @insertpiece( CalculatePsPos );@end
	@property( hlms_normal )psNormal	= mat3(worldView) * @insertpiece(local_normal);@end
	@property( normal_map )psTangent	= mat3(worldView) * @insertpiece(local_tangent);@end
@property( !hlms_dual_paraboloid_mapping )
	gl_Position = worldViewProj * @insertpiece(local_vertex);@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	gl_Position.w	= 1.0f;
	@property( hlms_normal )gl_Position.xyz	= psPos;@end
	@property( !hlms_normal )gl_Position.xyz	= @insertpiece( CalculatePsPos );@end
	float L = length( gl_Position.xyz );
	gl_Position.z	+= 1.0f;
	gl_Position.xy	/= gl_Position.z;
	gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);@end
@end
@piece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	psPosL@n = texWorldViewProj[@n] * @insertpiece(local_vertex);@end
@end

void main()
{
	@insertpiece( SkeletonTransform )
	@insertpiece( VertexTransform )
@foreach( hlms_uv_count, n )
	psUv@n = uv@n;@end

	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	psPosL@n.z = (psPosL@n.z - shadowDepthRange[@n].x) * shadowDepthRange[@n].w;@end

@property( hlms_pssm_splits )	psDepth = gl_Position.z;@end
@property( hlms_shadowcaster )
	//Linear depth
	psDepth	= (gl_Position.z - depthRange.x + shadowConstantBias) * depthRange.w;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	gl_Position.z = gl_Position.z * (gl_Position.w * depthRange.w);
@end
}
