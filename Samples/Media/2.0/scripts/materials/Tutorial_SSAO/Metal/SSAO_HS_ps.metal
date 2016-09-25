#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
	float3 cameraDir;
};

struct Params
{
	float2 projectionParams;
	float invKernelSize;
	float kernelRadius;
	float2 noiseScale;
	float4x4 projection;

	float4 sampleDirs[64];
};

inline float3 getScreenSpacePos( float2 uv, float3 cameraNormal, constant const Params &p,
								 texture2d<float> depthTexture, sampler samplerState0 )
{
	float fDepth = depthTexture.sample(samplerState0, uv).x;
	float linearDepth = p.projectionParams.y / (fDepth - p.projectionParams.x);

	return (cameraNormal * linearDepth);
}

inline float3 reconstructNormal( float3 posInView )
{
	return cross( normalize( dfdy(posInView) ), normalize( dfdx(posInView) ) );
}

inline float3 getNoiseVec( float2 uv, constant const Params &p,
						   texture2d<float> noiseTexture, sampler samplerState1 )
{
	float3 randomVec = noiseTexture.sample( samplerState1, uv * p.noiseScale ).xyz;
	return randomVec;
}

fragment float main_metal
(
	PS_INPUT inPs [[stage_in]],

	texture2d<float>	depthTexture	[[texture(0)]],
	texture2d<float>	noiseTexture	[[texture(1)]],

	sampler				samplerState0	[[sampler(0)]],
	sampler				samplerState1	[[sampler(1)]],

	constant Params &p					[[buffer(PARAMETER_SLOT)]]
)
{
	float3 viewPosition = getScreenSpacePos( inPs.uv0, inPs.cameraDir, p, depthTexture, samplerState0 );
	float3 viewNormal = reconstructNormal( viewPosition );
	float3 randomVec = getNoiseVec( inPs.uv0, p, noiseTexture, samplerState1 );

	float3 tangent = normalize( randomVec - viewNormal * dot(randomVec, viewNormal) );
	float3 bitangent = cross( viewNormal, tangent );
	float3x3 TBN = float3x3( tangent, bitangent, viewNormal );

	// Iterate over the sample kernel and calculate occlusion
	float occlusion = 0.0;
	for( int i = 0; i < 8; ++i )
	{
		for( int a = 0; a < 8; ++a )
		{
			float3 sNoise = p.sampleDirs[(a << 2u) + i].xyz;

			// get sample position
			float3 oSample = TBN * sNoise; //to view-space
			oSample = viewPosition + oSample * p.kernelRadius;

			// project sample position to get UV coords
			float4 offset = float4( oSample, 1.0 );
			offset = (p.projection * offset).xyzw; // from view to clip-space
			offset.xyz /= offset.w; // perspective divide
			offset.xy = offset.xy * 0.5 + float2(0.5, 0.5); // transform to range [0-1]
			offset.y = 1.0 - offset.y;

			float sampleDepth = getScreenSpacePos( offset.xy, inPs.cameraDir, p,
												   depthTexture, samplerState0 ).z;

			// range check and occlusion
			float rangeCheck = smoothstep( 0.0, 1.0, p.kernelRadius /
													 abs(viewPosition.z - sampleDepth) );
			occlusion += (sampleDepth >= oSample.z ? 1.0 : 0.0) * rangeCheck;
		}
	}

	occlusion = 1.0 - (occlusion * p.invKernelSize);

	return occlusion;
}
