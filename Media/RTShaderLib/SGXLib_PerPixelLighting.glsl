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
				in vec3 vViewPos,
				in vec4 vLightPos,
				in vec4 vAttParams,
				in vec4 vLightDirView,
				in vec4 spotParams,
				in vec4 vDiffuseColour,
				inout vec3 vOutDiffuse
#if defined(TVC_DIFFUSE) || defined(TVC_SPECULAR)
				, in vec4 vInVertexColour
#endif
#ifdef USE_SPECULAR
				, in vec4 vSpecularColour,
				in float fSpecularPower,
				inout vec3 vOutSpecular
#endif
#ifdef SHADOWLIGHT_COUNT
				, in float shadowFactor
#endif
#ifdef HAVE_AREA_LIGHTS
				, in sampler2D ltcLUT1,
				in sampler2D ltcLUT2
#endif
				)
{

    vec3 vLightView = vLightPos.xyz;
    float fLightD = 0.0;

#ifdef TVC_DIFFUSE
	vDiffuseColour *= vInVertexColour;
#endif

#ifdef HAVE_AREA_LIGHTS
	if(spotParams.w == 2.0)
	{
		// rect area light
		vec3 dcol = vDiffuseColour.rgb;
#ifdef USE_SPECULAR
#ifdef TVC_SPECULAR
		vSpecularColour *= vInVertexColour;
#endif
		vec3 scol = vSpecularColour.rgb;
#else
		vec3 scol = vec3_splat(0.0);
		float fSpecularPower = 0.0;
#endif
		float roughness = saturate(1.0 - fSpecularPower/128.0); // convert specular to roughness
		roughness *= roughness; // perceptual to physical roughness
		evaluateRectLight(ltcLUT1, ltcLUT2, roughness, normalize(vNormal), vViewPos, vLightPos.xyz, spotParams.xyz, vAttParams.xyz, scol, dcol);

#ifdef SHADOWLIGHT_COUNT
		dcol *= shadowFactor;
		scol *= shadowFactor;
#endif

		// linear to gamma
		dcol = pow(dcol, vec3_splat(1.0/2.2));
		vOutDiffuse.rgb = saturate(vOutDiffuse.rgb + dcol);
#ifdef USE_SPECULAR
		scol = pow(scol, vec3_splat(1.0/2.2));
		vOutSpecular.rgb = saturate(vOutSpecular.rgb + scol);
#endif
		return;
	}
#endif

    if (vLightPos.w != 0.0)
    {
        vLightView -= vViewPos; // to light
        fLightD     = length(vLightView);

        if(fLightD > vAttParams.x)
            return;
    }

	vLightView		   = normalize(vLightView);
	vec3 vNormalView = normalize(vNormal);
	float nDotL		   = saturate(dot(vNormalView, vLightView));
	
	if (nDotL <= 0.0)
		return;

	float fAtten	   = getDistanceAttenuation(vAttParams.yzw, fLightD);

#ifdef SHADOWLIGHT_COUNT
	fAtten *= shadowFactor;
#endif

    if(spotParams.w != 0.0)
    {
        fAtten *= getAngleAttenuation(spotParams.xyz, vLightDirView.xyz, vLightView);
    }

	vOutDiffuse  += vDiffuseColour.rgb * nDotL * fAtten;
	vOutDiffuse = saturate(vOutDiffuse);

#ifdef USE_SPECULAR
	vec3 vView       = -normalize(vViewPos);
	vec3 vHalfWay    = normalize(vView + vLightView);
	float nDotH        = saturate(dot(vNormalView, vHalfWay));
#ifdef TVC_SPECULAR
	vSpecularColour *= vInVertexColour;
#endif
#ifdef NORMALISED
	vSpecularColour *= (fSpecularPower + 8.0)/(8.0 * M_PI);
#endif
	vOutSpecular += vSpecularColour.rgb * pow(nDotH, fSpecularPower) * fAtten;
	vOutSpecular = saturate(vOutSpecular);
#endif
}