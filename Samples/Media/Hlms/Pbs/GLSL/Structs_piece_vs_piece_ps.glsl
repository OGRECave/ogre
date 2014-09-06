@piece( PassDecl )
struct ShadowReceiverData
{
	mat4 texWorldViewProj;
	vec2 shadowDepthRange;
};

struct Light
{
@property( hlms_num_shadow_maps )
	vec2 invShadowMapSize;
@end
	vec3 diffuse;
	vec3 specular;
	vec3 attenuation;
	vec3 spotDirection;
	vec3 spotParams;
};

//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader
@property( !hlms_shadowcaster )
	@property( hlms_num_shadow_maps )ShadowReceiverData shadowRcv[@value(hlms_num_shadow_maps)];@end
	mat4 view;
@end @property( hlms_shadowcaster )
	vec2 depthRange;
@end
	mat4 viewProj;

	//-------------------------------------------------------------------------

	//Pixel shader
	@property( hlms_lights_spot )Light lights[@value(hlms_lights_spot)]@end
	@property( envprobe_map )mat3 invViewMatCubemap;@end
} pass;
@end

@piece( MaterialDecl )
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	vec3 kD;
	float shadowConstantBias;
	vec3 kS;
	float roughness;
	@property( fresnel_scalar )@piece( FresnelType )vec3@end@end
	@property( !fresnel_scalar ) @piece( FresnelType )float@end @end
	//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
	@insertpiece( FresnelType ) F0;
	@property( normal_weight )float normalWeights[@value( normal_weight )];@end
	@property( detail_weights )vec4 cDetailWeights;@end
	@property( detail_offsetsD )vec4 detailOffsetScaleD[@value( detail_offsetsD )];@end
	@property( detail_offsetsN )vec4 detailOffsetScaleN[@value( detail_offsetsN )];@end

	uint indices[6];
};

layout(binding = 1) uniform MaterialBuf
{
	Material m[64];
} material;
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
layout(binding = 2) uniform InstanceBuffer
{
@property( !hlms_shadowcaster )
	//The lower 9 bits contain the material's start index.
	//The higher 23 bits contain the world matrix start index.
	uint worldMaterialIdx[4096];
@end @property( hlms_shadowcaster )
	//It's cheaper to send the shadowConstantBias rather than
	//sending the index to the material ID to read the bias.
	float shadowConstantBias;
@end
} instance;
@end

@piece( VStoPS_block )
	@property( !hlms_shadowcaster )
		@property( hlms_normal )
			vec3 pos;
			vec3 normal;
			@property( normal_map )vec3 tangent;
			@property( hlms_qtangent )flat float biNormalReflection;@end @end
		@end
		@foreach( hlms_uv_count, n )
			vec@value( hlms_uv_count@n ) uv@n;@end

		@foreach( hlms_num_shadow_maps, n )
			vec4 posL@n;@end
	@end
	@property( hlms_shadowcaster || hlms_pssm_splits )	float depth;@end
@end
