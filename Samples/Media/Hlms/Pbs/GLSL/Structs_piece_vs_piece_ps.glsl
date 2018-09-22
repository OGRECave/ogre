@piece( PassDecl )
struct ShadowReceiverData
{
    mat4 texViewProj;
@property( exponential_shadow_maps )
	vec4 texViewZRow;
@end
	vec2 shadowDepthRange;
	vec4 invShadowMapSize;
};

struct Light
{
	vec4 position; //.w contains the objLightMask
	vec3 diffuse;
	vec3 specular;
@property( hlms_num_shadow_map_lights )
	vec3 attenuation;
    //Spotlights:
    //  spotDirection.xyz is direction
    //  spotParams.xyz contains falloff params
    //Custom 2D Shape:
    //  spotDirection.xyz direction
    //  spotDirection.w customShapeHalfRectSize.x
    //  spotParams.xyz tangent
    //  spotParams.w customShapeHalfRectSize.y
    vec4 spotDirection;
    vec4 spotParams;
@end
};

#define areaLightDiffuseMipmapStart areaApproxLights[0].diffuse.w
#define areaLightNumMipmapsSpecFactor areaApproxLights[0].specular.w

struct AreaLight
{
	vec4 position; //.w contains the objLightMask
	vec4 diffuse;		//[0].w contains diffuse mipmap start
	vec4 specular;		//[0].w contains mipmap scale
	vec4 attenuation;	//.w contains texture array idx
	//Custom 2D Shape:
	//  direction.xyz direction
	//  direction.w invHalfRectSize.x
	//  tangent.xyz tangent
	//  tangent.w invHalfRectSize.y
	vec4 direction;
	vec4 tangent;
	vec4 doubleSided;
};

struct AreaLtcLight
{
	vec4 position;		//.w contains the objLightMask
	vec4 diffuse;		//.w contains attenuation range
	vec4 specular;		//.w contains doubleSided
	vec3 points[4];
};

@insertpiece( DeclCubemapProbeStruct )

//Uniforms that change per pass
layout_constbuffer(binding = 0) uniform PassBuffer
{
	//Vertex shader (common to both receiver and casters)
	mat4 viewProj;

@property( hlms_global_clip_planes )
	vec4 clipPlane0;
@end

@property( hlms_shadowcaster_point )
	vec4 cameraPosWS;	//Camera position in world space
@end

@property( !hlms_shadowcaster )
	//Vertex shader
	mat4 view;
	@property( hlms_num_shadow_map_lights )ShadowReceiverData shadowRcv[@value(hlms_num_shadow_map_lights)];@end

	//-------------------------------------------------------------------------

	//Pixel shader
	mat3 invViewMatCubemap;


@property( hlms_use_prepass )
	vec4 windowHeight;
@end

@property( ambient_hemisphere || ambient_fixed || envmap_scale )
	vec4 ambientUpperHemi;
@end
@property( ambient_hemisphere )
	vec4 ambientLowerHemi;
	vec4 ambientHemisphereDir;
@end

@property( irradiance_volumes )
	vec4 irradianceOrigin;	//.w = maxPower
	vec4 irradianceSize;	//.w = 1.0f / irradianceTexture->getHeight()
	mat4 invView;
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
	@property( exponential_shadow_maps )vec4 viewZRow;@end
	vec2 depthRange;
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
	vec4 f3dData;
	@property( hlms_forwardplus == forward3d )
		vec4 f3dGridHWW[@value( forward3d_num_slices )];
		vec4 f3dViewportOffset;
	@end
	@property( hlms_forwardplus != forward3d )
		vec4 fwdScreenToGrid;
	@end
@end

	@insertpiece( DeclPlanarReflUniforms )

@property( parallax_correct_cubemaps )
	CubemapProbe autoProbe;
@end

	@insertpiece( custom_passBuffer )
} passBuf;
@end

@property( fresnel_scalar )@piece( FresnelType )vec3@end @piece( FresnelSwizzle )xyz@end @end
@property( !fresnel_scalar )@piece( FresnelType )float@end @piece( FresnelSwizzle )x@end @end

@piece( MaterialDecl )
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	vec4 bgDiffuse;
	vec4 kD; //kD.w is alpha_test_threshold
	vec4 kS; //kS.w is roughness
	//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
	//F0.w is transparency
	vec4 F0;
	vec4 normalWeights;
	vec4 cDetailWeights;
	vec4 detailOffsetScale[4];
	vec4 emissive;		//emissive.w contains mNormalMapWeight.
	vec4 userValue[3];

	uvec4 indices0_3;
	uvec4 indices4_7;

	@insertpiece( custom_materialBuffer )
};

layout_constbuffer(binding = 1) uniform MaterialBuf
{
	Material m[@value( materials_per_buffer )];
} materialArray;
@end

@piece( InstanceDecl )
//Uniforms that change per Item/Entity
layout_constbuffer(binding = 2) uniform InstanceBuffer
{
    //.x =
	//The lower 9 bits contain the material's start index.
    //The higher 23 bits contain the world matrix start index.
    //
    //.y =
    //shadowConstantBias. Send the bias directly to avoid an
    //unnecessary indirection during the shadow mapping pass.
    //Must be loaded with uintBitsToFloat
    //
    //.z =
    //lightMask. Ogre must have been compiled with OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
    uvec4 worldMaterialIdx[4096];
} instance;
@end

@property( envprobe_map && envprobe_map != target_envprobe_map && use_parallax_correct_cubemaps )
@piece( PccManualProbeDecl )
layout_constbuffer(binding = 3) uniform ManualProbe
{
	CubemapProbe probe;
} manualProbe;
@end
@end

@piece( VStoPS_block )
    @property( !hlms_shadowcaster )
		@property( !lower_gpu_overhead )
			flat uint drawId;
		@end
		@property( hlms_normal || hlms_qtangent )
			vec3 pos;
			vec3 normal;
			@property( normal_map )vec3 tangent;
				@property( hlms_qtangent )flat float biNormalReflection;@end
			@end
		@end
		@foreach( hlms_uv_count, n )
			vec@value( hlms_uv_count@n ) uv@n;@end

		@foreach( hlms_num_shadow_map_lights, n )
			@property( !hlms_shadowmap@n_is_point_light )
				vec4 posL@n;@end @end
		@property( hlms_pssm_splits )float depth;@end
		@property( hlms_use_prepass_msaa > 1 )
			float2 zwDepth;
		@end
	@end
	@property( hlms_shadowcaster )
		@property( alpha_test )
			flat uint drawId;
			@foreach( hlms_uv_count, n )
				vec@value( hlms_uv_count@n ) uv@n;@end
		@end
		@property( (!hlms_shadow_uses_depth_texture || exponential_shadow_maps) && !hlms_shadowcaster_point )
			float depth;
		@end
		@property( hlms_shadowcaster_point )
			vec3 toCameraWS;
			@property( !exponential_shadow_maps )
				flat float constBias;
			@end
		@end
	@end
	@insertpiece( custom_VStoPS )
@end
