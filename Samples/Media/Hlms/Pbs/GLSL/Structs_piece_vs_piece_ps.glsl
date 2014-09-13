@piece( PassDecl )
struct ShadowReceiverData
{
	mat4 texWorldViewProj;
	vec2 shadowDepthRange;
};

struct Light
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
@property( hlms_num_shadow_maps )
	vec3 attenuation;
	vec3 spotDirection;
	vec3 spotParams;

	vec2 invShadowMapSize;
@end
};

//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader (common to both receiver and casters)
	mat4 viewProj;

@property( !hlms_shadowcaster )
	//Vertex shader
	mat4 view;
	@property( hlms_num_shadow_maps )ShadowReceiverData shadowRcv[@value(hlms_num_shadow_maps)];@end

	//-------------------------------------------------------------------------

	//Pixel shader
	mat3 invViewMatCubemap;
@property( hlms_pssm_splits )@foreach( hlms_pssm_splits, n )
	float pssmSplitPoints@n;@end @end
	@property( hlms_lights_spot )Light lights[@value(hlms_lights_spot)]@end
@end @property( hlms_shadowcaster )
	//Vertex shader
	vec2 depthRange;
@end
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
	vec3 kS;
	float roughness;
	@property( fresnel_scalar )@piece( FresnelType )vec3@end@end
	@property( !fresnel_scalar ) @piece( FresnelType )float@end @end
	//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
	@insertpiece( FresnelType ) F0;
	@property( !fresnel_scalar )vec2 padding;@end
	float normalWeights0;
	float normalWeights1;
	float normalWeights2;
	float normalWeights3;
	float normalWeights4;
	vec4 cDetailWeights;
	vec4 detailOffsetScaleD[4];
	vec4 detailOffsetScaleN[4];

	uint indices0;
	uint indices1;
	uint indices2;
	uint indices3;
	uint indices4;
	uint indices5;
	uint indices6;
};

layout(binding = 1) uniform MaterialBuf
{
	Material m[273];
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
