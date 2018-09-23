@piece( PassDecl )
struct ShadowReceiverData
{
	float4x4 texViewProj;
@property( exponential_shadow_maps )
	float4 texViewZRow;
@end
	float2 shadowDepthRange;
	float2 padding;
	float4 invShadowMapSize;
};

struct Light
{
	float4 position; //.w contains the objLightMask
	float3 diffuse;
	float3 specular;
@property( hlms_num_shadow_map_lights )
	float3 attenuation;
	float3 spotDirection;
	float3 spotParams;
@end
};

#define areaLightDiffuseMipmapStart areaApproxLights[0].diffuse.w
#define areaLightNumMipmapsSpecFactor areaApproxLights[0].specular.w

struct AreaLight
{
	float4 position;	//.w contains the objLightMask
	float4 diffuse;		//[0].w contains diffuse mipmap start
	float4 specular;	//[0].w contains mipmap scale
	float4 attenuation;	//.w contains texture array idx
	//Custom 2D Shape:
	//  direction.xyz direction
	//  direction.w invHalfRectSize.x
	//  tangent.xyz tangent
	//  tangent.w invHalfRectSize.y
	float4 direction;
	float4 tangent;
	float4 doubleSided;
};

struct AreaLtcLight
{
	float4 position;		//.w contains the objLightMask
	float4 diffuse;		//.w contains attenuation range
	float4 specular;		//.w contains doubleSided
	float3 points[4];
};

@insertpiece( DeclCubemapProbeStruct )

//Uniforms that change per pass
cbuffer PassBuffer : register(b0)
{
	struct PassData
	{
	//Vertex shader (common to both receiver and casters)
	float4x4 viewProj;

@property( hlms_global_clip_planes )
	float4 clipPlane0;
@end

@property( hlms_shadowcaster_point )
	float4 cameraPosWS;	//Camera position in world space
@end

@property( !hlms_shadowcaster )
	//Vertex shader
	float4x4 view;
	@property( hlms_num_shadow_map_lights )ShadowReceiverData shadowRcv[@value(hlms_num_shadow_map_lights)];@end

	//-------------------------------------------------------------------------

	//Pixel shader
	float3x3 invViewMatCubemap;
	float padding; //Compatibility with GLSL.

@property( hlms_use_prepass )
	float4 windowHeight;
@end

@property( ambient_hemisphere || ambient_fixed || envmap_scale )
	float4 ambientUpperHemi;
@end
@property( ambient_hemisphere )
	float4 ambientLowerHemi;
	float4 ambientHemisphereDir;
@end
@property( irradiance_volumes )
	float4 irradianceOrigin;	//.w = maxPower
	float4 irradianceSize;		//.w = 1.0f / irradianceTexture->getHeight()
	float4x4 invView;
@end
@property( hlms_pssm_splits )@psub( hlms_pssm_splits_minus_one, hlms_pssm_splits, 1 )@foreach( hlms_pssm_splits, n )
	float pssmSplitPoints@n;@end @end
@property( hlms_pssm_blend )@foreach( hlms_pssm_splits_minus_one, n )
	float pssmBlendPoints@n;@end @end
@property( hlms_pssm_fade )
	float pssmFadePoint;@end
	@property( hlms_lights_spot )Light lights[@value(hlms_lights_spot)];@end
	@property( hlms_lights_area_approx )AreaLight areaApproxLights[@value(hlms_lights_area_approx)];@end
	@property( hlms_lights_area_ltc )AreaLtcLight areaLtcLights[@value(hlms_lights_area_ltc)];@end
@end @property( hlms_shadowcaster )
	//Vertex shader
	@property( exponential_shadow_maps )float4 viewZRow;@end
	float2 depthRange;
@end

@property( hlms_forwardplus )
	//Forward3D
	//f3dData.x = minDistance;
	//f3dData.y = invMaxDistance;
	//f3dData.z = f3dNumSlicesSub1;
	//f3dData.w = uint cellsPerTableOnGrid0 (floatBitsToUint);

	//Clustered Forward:
	//f3dData.x = minDistance;
	//f3dData.y = invExponentK;
	//f3dData.z = f3dNumSlicesSub1;
	//f3dData.w = renderWindow->getHeight();
	float4 f3dData;
	@property( hlms_forwardplus == forward3d )
		float4 f3dGridHWW[@value( forward3d_num_slices )];
		float4 f3dViewportOffset;
	@end
	@property( hlms_forwardplus != forward3d )
		float4 fwdScreenToGrid;
	@end
@end

	@insertpiece( DeclPlanarReflUniforms )

@property( parallax_correct_cubemaps )
	CubemapProbe autoProbe;
@end

	@insertpiece( custom_passBuffer )
	} passBuf;
};
@end

@property( fresnel_scalar )@piece( FresnelType )float3@end @piece( FresnelSwizzle )xyz@end @end
@property( !fresnel_scalar )@piece( FresnelType )float@end @piece( FresnelSwizzle )x@end @end

@piece( MaterialDecl )
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	float4 bgDiffuse;
	float4 kD; //kD.w is alpha_test_threshold
	float4 kS; //kS.w is roughness
	//Fresnel coefficient, may be per colour component (float3) or scalar (float)
	//F0.w is transparency
	float4 F0;
	float4 normalWeights;
	float4 cDetailWeights;
	float4 detailOffsetScale[4];
	float4 emissive;		//emissive.w contains mNormalMapWeight.
	float4 userValue[3];

	uint4 indices0_3;
	uint4 indices4_7;

	@insertpiece( custom_materialBuffer )
};

cbuffer MaterialBuf : register(b1)
{
	Material materialArray[@value( materials_per_buffer )];
};
@end


@piece( InstanceDecl )
//Uniforms that change per Item/Entity
cbuffer InstanceBuffer : register(b2)
{
    //.x =
	//The lower 9 bits contain the material's start index.
    //The higher 23 bits contain the world matrix start index.
    //
    //.y =
    //shadowConstantBias. Send the bias directly to avoid an
    //unnecessary indirection during the shadow mapping pass.
    //Must be loaded with uintBitsToFloat
	@property( fast_shader_build_hack )
		uint4 worldMaterialIdx[2];
	@end @property( !fast_shader_build_hack )
		uint4 worldMaterialIdx[4096];
	@end
};
@end

@property( envprobe_map && envprobe_map != target_envprobe_map && use_parallax_correct_cubemaps )
@piece( PccManualProbeDecl )
cbuffer ManualProbe : register(b3)
{
	CubemapProbe manualProbe;
};
@end
@end

//Reset texcoord to 0 for every shader stage (since values are preserved).
@pset( texcoord, 0 )

@piece( VStoPS_block )
    @property( !hlms_shadowcaster )
		@property( !lower_gpu_overhead )
			nointerpolation uint drawId	: TEXCOORD@counter(texcoord);
		@end
		@property( hlms_normal || hlms_qtangent )
			float3 pos	: TEXCOORD@counter(texcoord);
			float3 normal	: TEXCOORD@counter(texcoord);
			@property( normal_map )float3 tangent	: TEXCOORD@counter(texcoord);
				@property( hlms_qtangent )nointerpolation float biNormalReflection	: TEXCOORD@counter(texcoord);@end
			@end
		@end
		@foreach( hlms_uv_count, n )
			float@value( hlms_uv_count@n ) uv@n	: TEXCOORD@counter(texcoord);@end

		@foreach( hlms_num_shadow_map_lights, n )
			@property( !hlms_shadowmap@n_is_point_light )
				float4 posL@n	: TEXCOORD@counter(texcoord);@end @end

		@property( hlms_pssm_splits )float depth	: TEXCOORD@counter(texcoord);@end

		@property( hlms_use_prepass_msaa > 1 )
			float2 zwDepth	: TEXCOORD@counter(texcoord);
		@end
	@end

	@property( hlms_shadowcaster )
		@property( alpha_test )
			nointerpolation uint drawId	: TEXCOORD@counter(texcoord);
			@foreach( hlms_uv_count, n )
				float@value( hlms_uv_count@n ) uv@n	: TEXCOORD@counter(texcoord);@end
		@end
		@property( (!hlms_shadow_uses_depth_texture || exponential_shadow_maps) && !hlms_shadowcaster_point )
			float depth	: TEXCOORD@counter(texcoord);
		@end
		@property( hlms_shadowcaster_point )
			float3 toCameraWS	: TEXCOORD@counter(texcoord);
			@property( !exponential_shadow_maps )
				nointerpolation float constBias	: TEXCOORD@counter(texcoord);
			@end
		@end
	@end

	@insertpiece( custom_VStoPS )
@end
