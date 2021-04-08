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

#ifdef PSSM_SAMPLE_CMP
#define SAMPLER_TYPE sampler2DShadow
#else
#define SAMPLER_TYPE sampler2D
#endif

#ifdef DEBUG_PSSM
STATIC vec3 pssm_lod_info = vec3(0.0, 0.0, 0.0);
#endif

//-----------------------------------------------------------------------------
void SGX_ApplyShadowFactor_Diffuse(in vec4 ambient, 
					  in vec4 lightSum, 
					  in float fShadowFactor, 
					  out vec4 oLight)
{
	oLight.rgb = ambient.rgb + (lightSum.rgb - ambient.rgb) * fShadowFactor;
	oLight.a   = lightSum.a;

#ifdef DEBUG_PSSM
	oLight.rgb += pssm_lod_info;
#endif
}
	
float sampleDepth(in SAMPLER_TYPE shadowMap, vec2 uv, float depth)
{
#ifdef PSSM_SAMPLE_CMP
#	if defined(OGRE_GLSL) && OGRE_GLSL < 130
	return shadow2D(shadowMap, vec3(uv, depth)).r;
#	else
	return texture(shadowMap, vec3(uv, depth));
#	endif
#else
	return (depth <= texture2D(shadowMap, uv).r) ? 1.0 : 0.0;
#endif
}

//-----------------------------------------------------------------------------
void SGX_ShadowPCF4(in SAMPLER_TYPE shadowMap, in vec4 shadowMapPos, in vec2 invTexSize, out float c)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
#if !defined(OGRE_REVERSED_Z) && !defined(OGRE_HLSL)
	shadowMapPos.z = shadowMapPos.z * 0.5 + 0.5; // convert -1..1 to 0..1
#endif
	vec2 uv = shadowMapPos.xy;
	vec3 o = vec3(invTexSize, -invTexSize.x) * 0.5;

    // depth must be clamped to support floating-point depth formats. This is to avoid comparing a
    // value from the depth texture (which is never greater than 1.0) with a greater-than-one
    // comparison value (which is possible with floating-point formats).
	float depth = clamp(shadowMapPos.z, 0.0, 1.0);

	// Note: We using 2x2 PCF. Good enough and is a lot faster.
	c =	 sampleDepth(shadowMap, uv.xy - o.xy, depth); // top left
	c += sampleDepth(shadowMap, uv.xy + o.xy, depth); // bottom right
	c += sampleDepth(shadowMap, uv.xy + o.zy, depth); // bottom left
	c += sampleDepth(shadowMap, uv.xy - o.zy, depth); // top right
		
	c /= 4.0;
#ifdef OGRE_REVERSED_Z
    c = 1.0 - c;
#endif
}

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
							out float oShadowFactor)
{
	if (fDepth  <= vSplitPoints.x)
	{									
		SGX_ShadowPCF4(shadowMap0, lightPosition0, invShadowMapSize0, oShadowFactor);
#ifdef DEBUG_PSSM
        pssm_lod_info.r = 1.0;
#endif
	}
#if PSSM_NUM_SPLITS > 2
	else if (fDepth <= vSplitPoints.y)
	{									
		SGX_ShadowPCF4(shadowMap1, lightPosition1, invShadowMapSize1, oShadowFactor);
#ifdef DEBUG_PSSM
        pssm_lod_info.g = 1.0;
#endif
	}
#endif
#if PSSM_NUM_SPLITS > 3
	else if (fDepth <= vSplitPoints.z)
	{
		SGX_ShadowPCF4(shadowMap2, lightPosition2, invShadowMapSize2, oShadowFactor);
#ifdef DEBUG_PSSM
		pssm_lod_info.r = 1.0;
        pssm_lod_info.g = 1.0;
#endif
	}
#endif
	else
	{
		SGX_ShadowPCF4(shadowMap3, lightPosition3, invShadowMapSize3, oShadowFactor);
#ifdef DEBUG_PSSM
        pssm_lod_info.b = 1.0;
#endif
	}
}
