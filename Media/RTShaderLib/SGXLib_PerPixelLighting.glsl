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
// Program Name: SGXLib_Lighting
// Program Desc: Per pixel lighting functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
// Notes: Implements core functions for FFPLighting class.
// based on lighting engine. 
// See http://msdn.microsoft.com/en-us/library/bb147178.aspx
//-----------------------------------------------------------------------------

#include "RTSLib_Lighting.glsl"
#ifdef HAVE_AREA_LIGHTS
#include "RTSLib_LTC.glsl"
#endif

#ifdef USE_LINEAR_COLOURS
#define FFP_SATURATE(x) x
#else
#define FFP_SATURATE(x) saturate(x)
#endif

#ifdef OGRE_HLSL
void SGX_Flip_Backface_Normal(in float triArea, in float targetFlipped, inout vec3 normal)
{
#if OGRE_HLSL == 3
	triArea *= -1.0;
	triArea *= targetFlipped;
#endif
	if(triArea < 0.0)
		normal *= -1.0;
}
#else
void SGX_Flip_Backface_Normal(in bool frontFacing, in float targetFlipped, inout vec3 normal)
{
	if(!frontFacing)
		normal *= -1.0;
}
#endif

void evaluateLight(
				in vec3 vNormal,
				in f32vec3 vViewPos,
				in vec4 vLightPos,
				in vec4 vAttParams,
				in vec4 vLightDirView,
				in vec4 spotParams,
				in vec3 vDiffuseColour,
				in float shadowFactor,
				inout vec3 vOutDiffuse
#ifdef USE_SPECULAR
				, in vec3 vSpecularColour,
				in float fSpecularPower,
				inout vec3 vOutSpecular
#endif
				)
{

    vec3 vLightView = vLightPos.xyz;

	float fAtten = shadowFactor;
    if (vLightPos.w != 0.0)
    {
		f32vec3 tmp = vLightPos.xyz - vViewPos;
        float fLightD     = length(tmp);

        if(fLightD > vAttParams.x)
            return;

		vLightView = tmp / fLightD; // normalize
		fAtten    *= getDistanceAttenuation(vAttParams, fLightD);
    }
	else
	{
		vLightView = normalize(vLightView);
	}

	vec3 vNormalView = normalize(vNormal);
	float nDotL		   = saturate(dot(vNormalView, vLightView));
	
	if (nDotL <= 0.0)
		return;

    if(spotParams.w != 0.0)
    {
        fAtten *= getAngleAttenuation(spotParams.xyz, vLightDirView.xyz, vLightView);
    }

	vOutDiffuse  += vDiffuseColour * nDotL * fAtten;
	vOutDiffuse = FFP_SATURATE(vOutDiffuse);

#ifdef USE_SPECULAR
	f32vec3 vView       = -normalize(vViewPos);
	f32vec3 vHalfWay    = normalize(vView + vLightView);
	float32_t nDotH  = saturate(dot(vNormalView, vHalfWay));
#ifdef NORMALISED
	vSpecularColour *= (fSpecularPower + 8.0)/(8.0 * M_PI);
#endif
	vOutSpecular += vSpecularColour * pow(nDotH, fSpecularPower) * fAtten;
	vOutSpecular = FFP_SATURATE(vOutSpecular);
#endif
}

#if LIGHT_COUNT > 0
void FFP_Lights(
#ifdef SHADOWLIGHT_COUNT
				in float shadowFactor[SHADOWLIGHT_COUNT],
#endif
#ifdef HAVE_AREA_LIGHTS
				in sampler2D ltcLUT1,
				in sampler2D ltcLUT2,
#endif
				in vec3 vNormal,
				in f32vec3 vViewPos,
				in f32vec4 vLightPos[LIGHT_COUNT],
				in f32vec4 vAttParams[LIGHT_COUNT],
				in f32vec4 vLightDirView[LIGHT_COUNT],
				in f32vec4 spotParams[LIGHT_COUNT],
				in f32vec4 vDiffuseColour[LIGHT_COUNT],
				inout vec3 vOutDiffuse
#if defined(TVC_DIFFUSE) || defined(TVC_SPECULAR)
				, in vec4 vInVertexColour
#endif
#ifdef USE_SPECULAR
				, in f32vec4 vSpecularColour[LIGHT_COUNT],
				in float fSpecularPower,
				inout vec3 vOutSpecular
#endif
				)
{
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		// resolve per-light inputs: vertex colour tracking and shadows
		vec3 dcol = vDiffuseColour[i].rgb;
#ifdef TVC_DIFFUSE
		dcol *= vInVertexColour.rgb;
#endif
		vec3 scol = vec3_splat(0.0);
#ifdef USE_SPECULAR
		scol = vSpecularColour[i].rgb;
#ifdef TVC_SPECULAR
		scol *= vInVertexColour.rgb;
#endif
#endif
		float fShadowFactor = 1.0;
#ifdef SHADOWLIGHT_COUNT
		// lights beyond SHADOWLIGHT_COUNT do not cast shadows
		if (i < SHADOWLIGHT_COUNT)
			fShadowFactor = shadowFactor[i];
#endif

#ifdef HAVE_AREA_LIGHTS
		if (spotParams[i].w == 2.0)
		{
			// rect area light - evaluated in linear space
			float roughness = 1.0;
#ifdef USE_SPECULAR
			roughness = saturate(1.0 - fSpecularPower/128.0); // specular power to roughness
			roughness *= roughness; // perceptual to physical roughness
#endif
			evaluateRectLight(ltcLUT1, ltcLUT2, roughness, normalize(vNormal), vViewPos,
							  vLightPos[i].xyz, spotParams[i].xyz, vAttParams[i].xyz, scol, dcol);

			// linear to gamma
			dcol = pow(dcol * fShadowFactor, vec3_splat(1.0/2.2));
			vOutDiffuse = FFP_SATURATE(vOutDiffuse + dcol);
#ifdef USE_SPECULAR
			scol = pow(scol * fShadowFactor, vec3_splat(1.0/2.2));
			vOutSpecular = FFP_SATURATE(vOutSpecular + scol);
#endif
			continue;
		}
#endif

		evaluateLight(vNormal, vViewPos, vLightPos[i], vAttParams[i], vLightDirView[i],
					  spotParams[i], dcol, fShadowFactor, vOutDiffuse
#ifdef USE_SPECULAR
					  , scol, fSpecularPower, vOutSpecular
#endif
					  );
	}
}
#endif