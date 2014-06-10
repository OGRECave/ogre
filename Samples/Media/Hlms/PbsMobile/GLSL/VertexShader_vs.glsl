@property( GL3+ )#version 330@end
@property( !GL3+ )#define in attribute
#define out varying@end

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
@property( !hlms_shadowcaster )@property( hlms_num_shadow_maps )
//Uniforms that change per pass
uniform mat4 texWorldViewProj[@value(hlms_num_shadow_maps)];
uniform vec4 shadowDepthRange[@value(hlms_num_shadow_maps)];@end @end
@property( hlms_shadowcaster )uniform vec4 depthRange;@end
//Uniforms that change per pass (skeleton anim) or per entity (non-skeleton anim)
//Note: worldView becomes "view" and worldViewProj "viewProj" on skel. anim.
uniform mat4 worldViewProj;
@property( !hlms_shadowcaster )uniform mat4 worldView;@end
//Uniforms that change per entity
@property( hlms_skeleton )uniform vec4 worldMat[180]; //180 = 60 matrices * 3 rows per matrix@end
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
	int _idx = int(blendIndices[0] * 3);
	vec4 _localPos;
	_localPos.x = dot( worldMat[_idx + 0], vertex );
	_localPos.y = dot( worldMat[_idx + 1], vertex );
	_localPos.z = dot( worldMat[_idx + 2], vertex );
	_localPos *= blendWeights[0];
	@property( hlms_normal )vec3 _localNorm;
	_localNorm.x = dot( worldMat[_idx + 0].xyz, normal );
	_localNorm.y = dot( worldMat[_idx + 1].xyz, normal );
	_localNorm.z = dot( worldMat[_idx + 2].xyz, normal );
	_localNorm *= blendWeights[0];@end
	@property( normal_map )vec3 _localTang;
	_localTang.x = dot( worldMat[_idx + 0].xyz, tangent );
	_localTang.y = dot( worldMat[_idx + 1].xyz, tangent );
	_localTang.z = dot( worldMat[_idx + 2].xyz, tangent );
	_localTang *= blendWeights[0];@end

	@psub( NeedsMoreThan1BonePerVertex, hlms_bones_per_vertex, 1 )
	@property( NeedsMoreThan1BonePerVertex )
	for( int i=1; i<@value( hlms_bones_per_vertex ); ++i )
	{
		_idx = int(blendIndices[i] * 3);
		vec4 tmp;
		tmp.x = dot( worldMat[_idx + 0], vertex );
		tmp.y = dot( worldMat[_idx + 1], vertex );
		tmp.z = dot( worldMat[_idx + 2], vertex );
		_localPos += tmp * blendWeights[i];
		@property( hlms_normal )vec3 _localNorm;
		tmp.x = dot( worldMat[_idx + 0].xyz, normal );
		tmp.y = dot( worldMat[_idx + 1].xyz, normal );
		tmp.z = dot( worldMat[_idx + 2].xyz, normal );
		_localNorm += tmp.xyz * blendWeights[i];@end
		@property( normal_map )vec3 _localTang;
		tmp.x = dot( worldMat[_idx + 0].xyz, tangent );
		tmp.y = dot( worldMat[_idx + 1].xyz, tangent );
		tmp.z = dot( worldMat[_idx + 2].xyz, tangent );
		_localTang += tmp.xyz * blendWeights[i];@end
	}
	@end

	_localPos.w = 1.0;
@end @end

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

@property( !hlms_shadowcaster )
	@insertpiece( ShadowReceive )
@foreach( hlms_num_shadow_maps, n )
	psPosL@n.z = (psPosL@n.z - shadowDepthRange[@n].x) * shadowDepthRange[@n].w;@end

@property( hlms_pssm_splits )	psDepth = gl_Position.z;@end
@end @property( hlms_shadowcaster )
	//Linear depth
	psDepth	= (gl_Position.z - depthRange.x + shadowConstantBias) * depthRange.w;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	gl_Position.z = gl_Position.z * (gl_Position.w * depthRange.w);
@end
}
