/* Copyright Torus Knot Software Ltd 2012-2014

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Adapted by Matias N. Goldberg (Dark Sylinc) to GLSL based on the Cg file shadows.cg
*/

#version 150

// Simple PCF 
// Number of samples in one dimension (square for total samples)
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1.0

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)
{
	return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadow(sampler2DShadow shadowMap, vec4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2.0 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float depth = textureProj(shadowMap, offsetSample(uv, vec2(x, y), invShadowMapSize));
			if (depth >= 1.0 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}


float calcSimpleShadow(sampler2DShadow shadowMap, vec4 shadowMapPos)
{
	return textureProj(shadowMap, shadowMapPos);
}

float calcPSSMDepthShadow(sampler2DShadow shadowMap0, sampler2DShadow shadowMap1, sampler2DShadow shadowMap2, 
						   vec4 lsPos0, vec4 lsPos1, vec4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   vec4 pssmSplitPoints, float camDepth)
{

	float shadow;
	vec4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = vec4(0.3, 0.0, 0, 0);
		shadow = calcDepthShadow(shadowMap0, lsPos0, invShadowmapSize0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		splitColour = vec4(0, 0.3, 0, 0);
		shadow = calcDepthShadow(shadowMap1, lsPos1, invShadowmapSize1);
	}
	else
	{
		splitColour = vec4(0.0, 0.0, 0.3, 0);
		shadow = calcDepthShadow(shadowMap2, lsPos2, invShadowmapSize2);
	}

	return shadow;
}

float calcPSSMSimpleShadow(sampler2DShadow shadowMap0, sampler2DShadow shadowMap1, sampler2DShadow shadowMap2, 
						   vec4 lsPos0, vec4 lsPos1, vec4 lsPos2,
						   vec4 pssmSplitPoints, float camDepth)
{

	float shadow;
	vec4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = vec4(0.3, 0.0, 0, 0);
		shadow = calcSimpleShadow(shadowMap0, lsPos0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		splitColour = vec4(0, 0.3, 0, 0);
		shadow = calcSimpleShadow(shadowMap1, lsPos1);
	}
	else
	{
		splitColour = vec4(0.0, 0.0, 0.3, 0);
		shadow = calcSimpleShadow(shadowMap2, lsPos2);
	}

	return shadow;
}



vec3 calcPSSMDebugShadow(sampler2DShadow shadowMap0, sampler2DShadow shadowMap1, sampler2DShadow shadowMap2, 
						   vec4 lsPos0, vec4 lsPos1, vec4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   vec4 pssmSplitPoints, float camDepth)
{

	vec4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		//splitColour = vec4(0.3, 0.0, 0, 0);
		//splitColour = lsPos0 / lsPos0.w;
		splitColour.rgb = vec3(textureProj(shadowMap0, lsPos0));
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		//splitColour = vec4(0, 0.3, 0, 0);
		//splitColour = lsPos1 / lsPos1.w;
		splitColour.rgb = vec3(textureProj(shadowMap1, lsPos1));
	}
	else
	{
		//splitColour = vec4(0.0, 0.0, 0.3, 0);
		//splitColour = lsPos2 / lsPos2.w;
		splitColour.rgb = vec3(textureProj(shadowMap2, lsPos2));
	}

	return splitColour.rgb;
}
