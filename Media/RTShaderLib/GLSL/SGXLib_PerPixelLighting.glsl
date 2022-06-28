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

#define M_PI 3.141592654

#ifdef OGRE_HLSL
void SGX_Flip_Backface_Normal(in float triArea, in float targetFlipped, inout vec3 normal)
{
#if OGRE_HLSL == 3
	triArea *= -1.0;
#endif
	triArea *= targetFlipped;
	if(triArea < 0.0)
		normal *= -1.0;
}
#else
void SGX_Flip_Backface_Normal(in bool frontFacing, in float targetFlipped, inout vec3 normal)
{
#ifdef VULKAN
	targetFlipped *= -1.0;
#endif
	if(targetFlipped < 0.0)
		frontFacing = !frontFacing;
	if(!frontFacing)
		normal *= -1.0;
}
#endif

//-----------------------------------------------------------------------------
void SGX_Light_Directional_Diffuse(
				   in vec3 vNormal,
				   in vec3 vLightDirView,
				   in vec3 vDiffuseColour, 
				   inout vec3 vOut)
{
	vec3 vNormalView = normalize(vNormal);
	float nDotL = dot(vNormalView, -vLightDirView);
	
	vOut += vDiffuseColour * clamp(nDotL, 0.0, 1.0);
	vOut = clamp(vOut, 0.0, 1.0);
}

//-----------------------------------------------------------------------------
void SGX_Light_Directional_DiffuseSpecular(
					in vec3 vNormal,
					in vec3 vViewPos,
					in vec3 vLightDirView,
					in vec3 vDiffuseColour, 
					in vec3 vSpecularColour, 
					in float fSpecularPower,
					inout vec3 vOutDiffuse,
					inout vec3 vOutSpecular)
{
	vec3 vNormalView = normalize(vNormal);		
	float nDotL		   = dot(vNormalView, -vLightDirView);
	vec3 vView       = -normalize(vViewPos);
	vec3 vHalfWay    = normalize(vView + -vLightDirView);
	float nDotH        = dot(vNormalView, vHalfWay);
	
	if (nDotL > 0.0)
	{
		vOutDiffuse  += vDiffuseColour * nDotL;		
#ifdef NORMALISED
		vSpecularColour *= (fSpecularPower + 8.0)/(8.0 * M_PI);
#endif
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower);
        vOutDiffuse = clamp(vOutDiffuse, 0.0, 1.0);
	    vOutSpecular = clamp(vOutSpecular, 0.0, 1.0);
	}
}

//-----------------------------------------------------------------------------
void SGX_Light_Point_Diffuse(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in vec3 vLightPos,
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    inout vec3 vOut)
{
	vec3 vLightView    = vLightPos - vViewPos;
	float fLightD      = length(vLightView);
	vec3 vNormalView = normalize(vNormal);
	float nDotL        = dot(vNormalView, normalize(vLightView));
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
			
		vOut += vDiffuseColour * nDotL * fAtten;
        vOut = clamp(vOut, 0.0, 1.0);
	}
}



//-----------------------------------------------------------------------------
void SGX_Light_Point_DiffuseSpecular(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in vec3 vLightPos,
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower,
					inout vec3 vOutDiffuse,
					inout vec3 vOutSpecular)
{
	vec3 vLightView    = vLightPos - vViewPos;
	float fLightD      = length(vLightView);
	vLightView		   = normalize(vLightView);	
	vec3 vNormalView = normalize(vNormal);
	float nDotL        = dot(vNormalView, vLightView);	
		
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{					
		vec3 vView       = -normalize(vViewPos);
		vec3 vHalfWay    = normalize(vView + vLightView);		
		float nDotH        = dot(vNormalView, vHalfWay);
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);					
		
		vOutDiffuse  += vDiffuseColour * nDotL * fAtten;
#ifdef NORMALISED
		vSpecularColour *= (fSpecularPower + 8.0)/(8.0 * M_PI);
#endif
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten;

        vOutDiffuse = clamp(vOutDiffuse, 0.0, 1.0);
        vOutSpecular = clamp(vOutSpecular, 0.0, 1.0);
	}
}

//-----------------------------------------------------------------------------
void SGX_Light_Spot_Diffuse(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in vec3 vLightPos,
				    in vec3 vLightDirView,
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    inout vec3 vOut)
{
	vec3 vLightView    = vLightPos - vViewPos;
	float fLightD      = length(vLightView);
	vLightView		   = normalize(vLightView);
	vec3 vNormalView = normalize(vNormal);
	float nDotL        = dot(vNormalView, vLightView);
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(-vLightDirView, vLightView);
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);	
						
		vOut += vDiffuseColour * nDotL * fAtten * fSpotT;
        vOut = clamp(vOut, 0.0, 1.0);
	}
}

//-----------------------------------------------------------------------------
void SGX_Light_Spot_DiffuseSpecular(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in vec3 vLightPos,
				    in vec3 vLightDirView,
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower,
					inout vec3 vOutDiffuse,
					inout vec3 vOutSpecular)
{
	vec3 vLightView    = vLightPos - vViewPos;
	float fLightD      = length(vLightView);
	vLightView		   = normalize(vLightView);
	vec3 vNormalView = normalize(vNormal);
	float nDotL        = dot(vNormalView, vLightView);
	
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		vec3 vView       = -normalize(vViewPos);
		vec3 vHalfWay    = normalize(vView + vLightView);				
		float nDotH        = dot(vNormalView, vHalfWay);
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(-vLightDirView, vLightView);
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);	
						
		vOutDiffuse  += vDiffuseColour * nDotL * fAtten * fSpotT;
#ifdef NORMALISED
		vSpecularColour *= (fSpecularPower + 8.0)/(8.0 * M_PI);
#endif
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten * fSpotT;
        vOutDiffuse = clamp(vOutDiffuse, 0.0, 1.0);
        vOutSpecular = clamp(vOutSpecular, 0.0, 1.0);
	}
}

