#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

float4 offsetSample(float4 uv, float2 offset, float invMapSize)
{
	return float4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadow(sampler2D shadowMap, float4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float depth = tex2Dproj(shadowMap, offsetSample(uv, float2(x, y), invShadowMapSize)).x;
			if (depth >= 1 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}


float calcSimpleShadow(sampler2D shadowMap, float4 shadowMapPos)
{
	return tex2Dproj(shadowMap, shadowMapPos).x;
}

float calcPSSMDepthShadow(sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2, 
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

float calcPSSMSimpleShadow(sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2, 
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



float3 calcPSSMDebugShadow(sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2, 
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
		splitColour.rgb = tex2Dproj(shadowMap0, lsPos0).xxx;
		//splitColour = tex2Dproj(shadowMap0, lsPos0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		//splitColour = float4(0, 0.3, 0, 0);
		//splitColour = lsPos1 / lsPos1.w;
		splitColour.rgb = tex2Dproj(shadowMap1, lsPos1).xxx;
		//splitColour = tex2Dproj(shadowMap1, lsPos1);
	}
	else
	{
		//splitColour = float4(0.0, 0.0, 0.3, 0);
		//splitColour = lsPos2 / lsPos2.w;
		splitColour.rgb = tex2Dproj(shadowMap2, lsPos2).xxx;
		//splitColour = tex2Dproj(shadowMap2, lsPos2);
	}

	return splitColour.rgb;
}
