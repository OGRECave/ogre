#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

SamplerState samplerstate
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
	AddressW = Wrap;
};

float4 offsetSample(float4 uv, float2 offset, float invMapSize)
{
	return float4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadow(Texture2D shadowMap, float4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float4 tmpTexcoord = offsetSample(uv, float2(x, y), invShadowMapSize);
			float depth = shadowMap.Sample(samplerstate, tmpTexcoord.xy / tmpTexcoord.w).x;
			if (depth >= 1 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}


float calcSimpleShadow(Texture2D shadowMap, float4 shadowMapPos)
{
	return shadowMap.Sample(samplerstate, shadowMapPos.xy / shadowMapPos.w ).x;
}

float calcPSSMDepthShadow(Texture2D shadowMap0, Texture2D shadowMap1, Texture2D shadowMap2, 
						   float4 lsPos0, float4 lsPos1, float4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   float4 pssmSplitPoints, float camDepth)
{

	float shadow;
	float4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = float4(0.3, 0.0, 0, 0);
		shadow = calcDepthShadow(shadowMap0, lsPos0, invShadowmapSize0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		splitColour = float4(0, 0.3, 0, 0);
		shadow = calcDepthShadow(shadowMap1, lsPos1, invShadowmapSize1);
	}
	else
	{
		splitColour = float4(0.0, 0.0, 0.3, 0);
		shadow = calcDepthShadow(shadowMap2, lsPos2, invShadowmapSize2);
	}

	return shadow;
}

float calcPSSMSimpleShadow(Texture2D shadowMap0, Texture2D shadowMap1, Texture2D shadowMap2, 
						   float4 lsPos0, float4 lsPos1, float4 lsPos2,
						   float4 pssmSplitPoints, float camDepth)
{

	float shadow;
	float4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = float4(0.3, 0.0, 0, 0);
		shadow = calcSimpleShadow(shadowMap0, lsPos0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		splitColour = float4(0, 0.3, 0, 0);
		shadow = calcSimpleShadow(shadowMap1, lsPos1);
	}
	else
	{
		splitColour = float4(0.0, 0.0, 0.3, 0);
		shadow = calcSimpleShadow(shadowMap2, lsPos2);
	}

	return shadow;
}



float3 calcPSSMDebugShadow(Texture2D shadowMap0, Texture2D shadowMap1, Texture2D shadowMap2, 
						   float4 lsPos0, float4 lsPos1, float4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   float4 pssmSplitPoints, float camDepth)
{

	float4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		//splitColour = float4(0.3, 0.0, 0, 0);
		//splitColour = lsPos0 / lsPos0.w;
		splitColour.rgb = shadowMap0.Sample(samplerstate, lsPos0.xy / lsPos0.w).xxx;
		//splitColour = tex2Dproj(shadowMap0, lsPos0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		//splitColour = float4(0, 0.3, 0, 0);
		//splitColour = lsPos1 / lsPos1.w;
		splitColour.rgb = shadowMap1.Sample(samplerstate, lsPos1.xy / lsPos1.w).xxx;
		//splitColour = tex2Dproj(shadowMap1, lsPos1);
	}
	else
	{
		//splitColour = float4(0.0, 0.0, 0.3, 0);
		//splitColour = lsPos2 / lsPos2.w;
		splitColour.rgb = shadowMap2.Sample(samplerstate, lsPos2.xy / lsPos2.w).xxx;
		//splitColour = tex2Dproj(shadowMap2, lsPos2);
	}

	return splitColour.rgb;
}
