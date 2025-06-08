/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd
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
-----------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Integrated PSSM functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

#ifdef PSSM_ARRAY_TEXTURE
#   ifdef PSSM_SAMPLE_CMP
    #define SAMPLER_TYPE sampler2DArrayShadow
#   else
    #define SAMPLER_TYPE sampler2DArray
#   endif
#define PSSM_LAYER_ARG(x) x,
#else
#   ifdef PSSM_SAMPLE_CMP
    #define SAMPLER_TYPE sampler2DShadow
#   else
    #define SAMPLER_TYPE sampler2D
#   endif
#define PSSM_LAYER_ARG(x)
#endif

// default to 2x2 PCF
#ifndef PCF_XSAMPLES
#define PCF_XSAMPLES 2.0
#endif

//-----------------------------------------------------------------------------
#ifdef SHADOWLIGHT_COUNT
void SGX_ApplyShadowFactor_Modulative(in vec4 ambient,
					  in float fShadowFactor[SHADOWLIGHT_COUNT],
					  inout vec4 diffCol
#ifdef USE_SPECULAR
					  , inout vec4 specCol
#endif
					  )
{
	float shadowFactor = fShadowFactor[0];
	for(int i = 1; i < SHADOWLIGHT_COUNT; ++i)
		shadowFactor *= fShadowFactor[i];

	diffCol.rgb = mix(ambient.rgb, diffCol.rgb, shadowFactor);

#ifdef USE_SPECULAR
	specCol.rgb *= shadowFactor;
#endif
}
#endif
//-----------------------------------------------------------------------------
#ifdef PSSM_SAMPLE_COLOUR
void SGX_ShadowPCF4(in SAMPLER_TYPE shadowMap, in vec4 shadowMapPos, in vec2 invTexSize, PSSM_LAYER_ARG(in float layer) out float c)
{
#	ifdef PSSM_ARRAY_TEXTURE
	c = texture2DArray(shadowMap, vec3(shadowMapPos.xy/shadowMapPos.w, layer)).x;
#	else
	c = texture2DProj(shadowMap, shadowMapPos).x;
#	endif
}
#else
float sampleDepth(in SAMPLER_TYPE shadowMap, vec2 uv, PSSM_LAYER_ARG(in float layer) float depth)
{
#ifdef PSSM_ARRAY_TEXTURE
	// we can assume PSSM_SAMPLE_CMP is always defined when using array textures
	return shadow2D(shadowMap, vec4(uv, layer, depth));
#elif defined(PSSM_SAMPLE_CMP)
#	if defined(OGRE_GLSL) && OGRE_GLSL < 130
	return shadow2D(shadowMap, vec3(uv, depth)).r;
#	else
	return shadow2D(shadowMap, vec3(uv, depth));
#	endif
#else
	return (depth <= texture2D(shadowMap, uv).r) ? 1.0 : 0.0;
#endif
}
void SGX_ShadowPCF4(in SAMPLER_TYPE shadowMap, in vec4 shadowMapPos, in vec2 invTexSize, PSSM_LAYER_ARG(in float layer) out float c)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
#if !defined(OGRE_REVERSED_Z) && !defined(OGRE_HLSL) && !defined(VULKAN)
	shadowMapPos.z = shadowMapPos.z * 0.5 + 0.5; // convert -1..1 to 0..1
#endif
	vec2 uv = shadowMapPos.xy;

    // depth must be clamped to support floating-point depth formats. This is to avoid comparing a
    // value from the depth texture (which is never greater than 1.0) with a greater-than-one
    // comparison value (which is possible with floating-point formats).
	float depth = clamp(shadowMapPos.z, 0.0, 1.0);

	c = 0.0;
	float scale = 1.0;
	float offset = (PCF_XSAMPLES / 2.0 - 0.5) * scale;
	for (float y = -offset; y <= offset; y += scale)
		for (float x = -offset; x <= offset; x += scale)
			c += sampleDepth(shadowMap, uv + invTexSize * vec2(x, y), PSSM_LAYER_ARG(layer) depth);

	c /= PCF_XSAMPLES * PCF_XSAMPLES;
}
#endif

//-----------------------------------------------------------------------------
void SGX_ComputeShadowFactor_PSSM3(in float fDepth,
							in vec4 vSplitPoints,	
							in vec4 lightPosition0,
							in SAMPLER_TYPE shadowMap0,
							in vec2 invShadowMapSize0,
							#if PSSM_NUM_SPLITS > 2
							in vec4 lightPosition1,
							in SAMPLER_TYPE shadowMap1,
							in vec2 invShadowMapSize1,
							#endif
							#if PSSM_NUM_SPLITS > 3
							in vec4 lightPosition2,
							in SAMPLER_TYPE shadowMap2,
							in vec2 invShadowMapSize2,
							#endif
							in vec4 lightPosition3,
							in SAMPLER_TYPE shadowMap3,
							in vec2 invShadowMapSize3,
							out float oShadowFactor
							#ifdef DEBUG_PSSM
							, out vec4 oDiffuse
							#endif
							)
{
#if !defined(OGRE_REVERSED_Z) && !defined(OGRE_HLSL) && !defined(VULKAN)
	vSplitPoints = vSplitPoints * 0.5 + 0.5; // convert -1..1 to 0..1
#endif

#ifdef OGRE_REVERSED_Z
	vSplitPoints = vec4_splat(1.0) - vSplitPoints;
	fDepth = 1.0 - fDepth;
#endif

	if (fDepth  <= vSplitPoints.x)
	{
		SGX_ShadowPCF4(shadowMap0, lightPosition0, invShadowMapSize0, PSSM_LAYER_ARG(0.0) oShadowFactor);
#ifdef DEBUG_PSSM
        oDiffuse.r += 1.0;
#endif
	}
#if PSSM_NUM_SPLITS > 2
	else if (fDepth <= vSplitPoints.y)
	{
		SGX_ShadowPCF4(shadowMap1, lightPosition1, invShadowMapSize1, PSSM_LAYER_ARG(1.0) oShadowFactor);
#ifdef DEBUG_PSSM
        oDiffuse.g += 1.0;
#endif
	}
#endif
#if PSSM_NUM_SPLITS > 3
	else if (fDepth <= vSplitPoints.z)
	{
        SGX_ShadowPCF4(shadowMap2, lightPosition2, invShadowMapSize2, PSSM_LAYER_ARG(2.0) oShadowFactor);
#ifdef DEBUG_PSSM
		oDiffuse.r += 1.0;
        oDiffuse.g += 1.0;
#endif
	}
#endif
	else if (fDepth <= vSplitPoints.w)
	{
		SGX_ShadowPCF4(shadowMap3, lightPosition3, invShadowMapSize3, PSSM_LAYER_ARG(float(PSSM_NUM_SPLITS - 1)) oShadowFactor);
#ifdef DEBUG_PSSM
        oDiffuse.b += 1.0;
#endif
	}
	else
	{
		// behind far distance
		oShadowFactor = 1.0;
	}
}
