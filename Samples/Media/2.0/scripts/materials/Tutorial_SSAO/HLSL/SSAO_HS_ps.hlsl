struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float3 cameraDir	: TEXCOORD1;
};

Texture2D<float> depthTexture	: register(t0);
Texture2D<float3> noiseTexture  : register(t1);

SamplerState samplerState0		: register(s0);
SamplerState samplerState1		: register(s1);

uniform float2 projectionParams;
uniform float invKernelSize;
uniform float kernelRadius;
uniform float2 noiseScale;
uniform matrix projection;

uniform float4 sampleDirs[64];


float3 getScreenSpacePos(float2 uv, float3 cameraNormal)
{
	float fDepth = depthTexture.Sample(samplerState0, uv).x;
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);

	return (cameraNormal * linearDepth);
}

float3 reconstructNormal(float3 posInView)
{
	return cross(normalize(ddy(posInView)), normalize(ddx(posInView)));
}

float3 getNoiseVec(float2 uv)
{
	float3 randomVec = noiseTexture.Sample(samplerState1, uv*noiseScale).xyz;
	return randomVec;
}

float main
(
	PS_INPUT inPs
) : SV_Target
{
	float3 viewPosition = getScreenSpacePos(inPs.uv0, inPs.cameraDir);
	float3 viewNormal = reconstructNormal(viewPosition);
	float3 randomVec = getNoiseVec(inPs.uv0);

	float3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
	float3 bitangent = cross(viewNormal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, viewNormal);

	// Iterate over the sample kernel and calculate occlusion
	float occlusion = 0.0;
	for (int i = 0; i < 8; ++i)
	{
		for (int a = 0; a < 8; ++a)
		{
			float3 sNoise = sampleDirs[(a << 2u) + i].xyz;

			// get sample position
			float3 oSample = mul(sNoise, TBN); //to view-space
			oSample = viewPosition + oSample * kernelRadius;

			// project sample position to get UV coords
			float4 offset = float4(oSample, 1.0);
			offset = mul(projection, offset).xyzw; // from view to clip-space
			offset.xyz /= offset.w; // perspective divide
			offset.xy = offset.xy * 0.5 + float2(0.5, 0.5); // transform to range [0-1]
			offset.y = 1.0 - offset.y;

			float sampleDepth = getScreenSpacePos(offset.xy, inPs.cameraDir).z;

			// range check and occlusion
			float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(viewPosition.z - sampleDepth));
			occlusion += (sampleDepth >= oSample.z ? 1.0 : 0.0) * rangeCheck;
		}
	}

	occlusion = 1.0 - (occlusion * invKernelSize);

	return occlusion;
}
