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
// Program Name: SGXLib_NormalMapLighting 
// Program Desc: Normal map lighting functions. 
// Program Type: Vertex/Pixel shader 
// Language: GLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_ConstructTBNMatrix(in vec3 vNormal, 
				   in vec3 vTangent, 
				   out mat3 vOut)
{
	vec3 vBinormal = cross(vTangent, vNormal);

	vOut[0][0] = vTangent.x;
	vOut[1][0] = vTangent.y;
	vOut[2][0] = vTangent.z;	

	vOut[0][1] = vBinormal.x;
	vOut[1][1] = vBinormal.y;
	vOut[2][1] = vBinormal.z;

	vOut[0][2] = vNormal.x;
	vOut[1][2] = vNormal.y;
	vOut[2][2] = vNormal.z;
}

//-----------------------------------------------------------------------------
void SGX_TransformNormal(in mat3 m, 
				   in vec3 v, 
				   out vec3 vOut)
{
	vOut = (m * v) ;
}

//-----------------------------------------------------------------------------
void SGX_TransformNormal(in mat4 m, 
				   in vec3 v, 
				   out vec3 vOut)
{
	vOut = (m * vec4(v, 1.0)).xyz ;
}

//-----------------------------------------------------------------------------
void SGX_TransformPosition(in mat4 m, 
				   in vec4 v, 
				   out vec3 vOut)
{
	vOut = (m * v).xyz;
}

//-----------------------------------------------------------------------------
void SGX_FetchNormal(in sampler2D s, 
				   in vec2 uv, 
				   out vec3 vOut)
{
	vOut = 2.0 * texture(s, uv).xyz - 1.0;
}

void SGX_FetchNormal(in sampler2D s, 
				   in vec2 uv, 
				   out vec4 vOut)
{
	
	vec4 color = texture(s, uv);
	vOut = vec4(2.0 * color.xyz - 1.0,color.w);
}


	

//-----------------------------------------------------------------------------
void SGX_Light_Directional_Diffuse(
				   in vec4 vTSNormal,
				   in vec3 vTSNegLightDir,
				   in vec3 vDiffuseColour, 
				   in vec3 vBaseColour, 
				   out vec3 vOut)
{
	vec3 vTSNegLightDirNorm	= normalize(vTSNegLightDir);		
	float nDotL					= dot(vTSNormal.xyz, vTSNegLightDirNorm);
	
	vOut = vBaseColour + vDiffuseColour * clamp(nDotL, 0.0, 1.0);
}

//-----------------------------------------------------------------------------
void SGX_Generate_Parallax_Texcoord(in sampler2D normalHeightMap,
						in vec2 texCoord,
						in vec3 eyeVec,
						in vec2 scaleBias,
						out vec2 newTexCoord)
{
	eyeVec = normalize(eyeVec);
	float height = texture(normalHeightMap, texCoord).a;
	float displacement = (height * scaleBias.x) + scaleBias.y;
	vec3 scaledEyeDir = eyeVec * displacement;
	newTexCoord = (scaledEyeDir  + vec3(texCoord, 1.0)).xy;
}





//-----------------------------------------------------------------------------
void SGX_Light_Directional_DiffuseSpecular(
					in vec4 vTSNormal,
					in vec3 vTSView,					
					in vec3 vTSNegLightDir,
					in vec3 vDiffuseColour, 
					in vec3 vSpecularColour, 
					in float fSpecularPower, 
					in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
		
	vec3 vTSNegLightDirNorm		= normalize(vTSNegLightDir);		
	float nDotL		   			= dot(vTSNormal.xyz, vTSNegLightDirNorm);		
	vec3 vTSViewNorm 			= normalize(vTSView);
	vec3 vHalfWay    			= normalize(vTSView + vTSNegLightDir);
	float nDotH        			= dot(vTSNormal.xyz, vHalfWay);
	
	if (nDotL > 0.0)
	{
		vOutDiffuse  += vDiffuseColour * nDotL;		
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * vTSNormal.w;						
	}
}



//-----------------------------------------------------------------------------
void SGX_Light_Point_Diffuse(
				    in vec4 vTSNormal,				    
				    in vec3 vTSToLight,
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vBaseColour, 
				    out vec3 vOut)
{
	vOut = vBaseColour;		
	
	float fLightD      = length(vTSToLight);	
	float nDotL        = dot(vTSNormal.xyz, normalize(vTSToLight));
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
			
		vOut += vDiffuseColour * nDotL * fAtten;
	}		
}



//-----------------------------------------------------------------------------
void SGX_Light_Point_DiffuseSpecular(
				    in vec4 vTSNormal,
				    in vec3 vTSView,
				    in vec3 vTSToLight,				  
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower, 
				    in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
	
	float fLightD				= length(vTSToLight);
	vec3 vTSNegLightDirNorm	= normalize(vTSToLight);		
	float nDotL					= dot(vTSNormal.xyz, vTSNegLightDirNorm);	
		
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{					
		vec3 vTSViewNorm = normalize(vTSView);	
		vec3 vHalfWay    = normalize(vTSViewNorm + vTSNegLightDirNorm);		
		float nDotH        = dot(vTSNormal.xyz, vHalfWay);
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);					
		
		vOutDiffuse  += vDiffuseColour * nDotL * fAtten;
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten * vTSNormal.w;					
	}		
}

//-----------------------------------------------------------------------------
void SGX_Light_Spot_Diffuse(
				    in vec4 vTSNormal,
				    in vec3 vTSToLight,
				    in vec3 vTSNegLightDir,				  
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vBaseColour, 
				    out vec3 vOut)
{
	vOut = vBaseColour;
	
	float fLightD			= length(vTSToLight);
	vec3 vTSToLightNorm	= normalize(vTSToLight);
	float nDotL				= dot(vTSNormal.xyz, vTSToLightNorm);
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		vec3 vTSNegLightDirNorm	= normalize(vTSNegLightDir);
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(vTSNegLightDirNorm, vTSToLightNorm);
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);
		
		vOut += vDiffuseColour * nDotL * fAtten * fSpotT;
	}		
}

//-----------------------------------------------------------------------------
void SGX_Light_Spot_DiffuseSpecular(
				    in vec4 vTSNormal,
				    in vec3 vTSView,
				    in vec3 vTSToLight,					    
				    in vec3 vTSNegLightDir,		
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower, 
				    in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
	
	float fLightD			= length(vTSToLight);
	vec3 vTSToLightNorm	= normalize(vTSToLight);
	float nDotL				= dot(vTSNormal.xyz, vTSToLightNorm);
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		vec3 vTSNegLightDirNorm	= normalize(vTSNegLightDir);
		vec3 vTSViewNorm = normalize(vTSView);
		vec3 vHalfWay    = normalize(vTSViewNorm + vTSNegLightDirNorm);	
		float nDotH        = dot(vTSNormal.xyz, vHalfWay);
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(vTSNegLightDirNorm, vTSToLightNorm);
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);

		vOutDiffuse  += vDiffuseColour * nDotL * fAtten * fSpotT;
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten * fSpotT * vTSNormal.w;
	}	
}

