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
// Program Name: FFPLib_TextureStage
// Program Desc: Texture functions of the FFP.
// Program Type: Vertex/Pixel shader
// Language: HLSL
// Notes: Implements core functions for FFPTexturing class.
// based on texturing operations needed by render system.
// Implements texture coordinate processing:
// see http://msdn.microsoft.com/en-us/library/bb206247.aspx
// Implements texture blending operation:
// see http://msdn.microsoft.com/en-us/library/bb206241.aspx
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in float4x4 m, in float2 v, out float2 vOut)
{
	vOut = mul(m, float4(v, 0, 1)).xy;
}

//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in float4x4 m, in float3 v, out float3 vOut)
{
	vOut = mul(m, float4(v, 1)).xyz;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Normal(in float4x4 mWorldIT, 
						   in float4x4 mView,
						   in float3 vNormal,
						   out float3 vOut)
{
	float3 vWorldNormal = mul((float3x3)mWorldIT, vNormal);
	float3 vViewNormal  = mul((float3x3)mView, vWorldNormal);

	vOut = vViewNormal;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Normal(in float4x4 mWorldIT, 
						   in float4x4 mView,
						   in float4x4 mTexture,
						   in float3 vNormal,
						   out float3 vOut)
{
	float3 vWorldNormal = mul((float3x3)mWorldIT, vNormal);
	float3 vViewNormal  = mul((float3x3)mView, vWorldNormal);
	
	vOut = mul(mTexture, float4(vViewNormal, 1)).xyz;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Sphere(in float4x4		mWorld,
										in  float4x4	mView,
										in  float4x4	mWorldIT,
										in  float4		vPos,
										in  float3		vNormal,
										in  float4x4	mTexture,
										out float2		vOut)
{
	float4x4 worldview = mul(mView,mWorld);
	float3 normal = normalize(mul(mWorldIT,float4(vNormal,0.0)).xyz); 
	float3 eyedir =  normalize(mul(worldview, vPos)).xyz;
	float3 r = reflect(eyedir, normal);
	float two_p = 2 * sqrt( r.x *  r.x +  r.y *  r.y +  (r.z + 1) *  (r.z + 1));
	vOut = mul(mTexture, float4(0.5 + r.x / two_p,0.5 - r.y / two_p, 0, 0)).xy;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Sphere(in	float4x4	mWorld,
										in	float4x4	mView,
										in	float4x4	mWorldIT,
										in	float4		vPos,
										in	float3		vNormal,
										out float2		vOut)
{
	float4x4 worldview = mul(mView,mWorld);
	float3 normal = normalize(mul(mWorldIT,float4(vNormal,0.0)).xyz); 
	float3 eyedir =  normalize(mul(worldview, vPos)).xyz;
	float3 r = reflect(eyedir, normal);
	float two_p = 2 * sqrt( r.x *  r.x +  r.y *  r.y +  (r.z + 1) *  (r.z + 1));
	vOut = float2(0.5 + r.x / two_p,0.5 - r.y / two_p);
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Reflect(in float4x4 mWorld, 
							in float4x4 mWorldIT, 
						   in float4x4 mView,						  
						   in float3 vNormal,
						   in float4 vPos,						  
						   out float3 vOut)
{		
	mView[2][0] = -mView[2][0];
	mView[2][1] = -mView[2][1];
	mView[2][2] = -mView[2][2];
	mView[2][3] = -mView[2][3];
	
	float4x4 matViewT = transpose(mView);

	float3 vWorldNormal = mul((float3x3)mWorldIT, vNormal);
	float3 vViewNormal  = mul((float3x3)mView, vWorldNormal);
	float4 vWorldPos    = mul(mWorld, vPos);
	float3 vNormViewPos  = normalize(mul(mView, vWorldPos).xyz);
	
	float3 vReflect = reflect(vNormViewPos, vViewNormal);
	
  	matViewT[2][0] = -matViewT[2][0];
 	matViewT[2][1] = -matViewT[2][1];
  	matViewT[2][2] = -matViewT[2][2];
 	vReflect = mul((float3x3)matViewT, vReflect);
 	
	vOut = vReflect;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Reflect(in float4x4 mWorld, 
							in float4x4 mWorldIT, 
						   in float4x4 mView,	
						   in float4x4 mTexture,					  
						   in float3 vNormal,
						   in float4 vPos,						  
						   out float3 vOut)
{		
	mView[2][0] = -mView[2][0];
	mView[2][1] = -mView[2][1];
	mView[2][2] = -mView[2][2];
	mView[2][3] = -mView[2][3];
	
	float4x4 matViewT = transpose(mView);

	float3 vWorldNormal = mul((float3x3)mWorldIT, vNormal);
	float3 vViewNormal  = mul((float3x3)mView, vWorldNormal);
	float4 vWorldPos    = mul(mWorld, vPos);
	float3 vNormViewPos  = normalize(mul(mView, vWorldPos).xyz);
	
	float3 vReflect = reflect(vNormViewPos, vViewNormal);

	matViewT[2][0] = -matViewT[2][0];
 	matViewT[2][1] = -matViewT[2][1];
  	matViewT[2][2] = -matViewT[2][2];
 	vReflect = mul((float3x3)matViewT, vReflect);	

 	vReflect = mul(mTexture, float4(vReflect,1)).xyz;
 	
	vOut = vReflect;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_Projection(in float4x4 mWorld, 							
						   in float4x4 mTexViewProjImage,						  						  
						   in float4 vPos,						  				  
						   out float3 vOut)
{
	float4 vWorldPos    = mul(mWorld, vPos);
	float4 vTexturePos  = mul(mTexViewProjImage, vWorldPos);

	vOut = float3(vTexturePos.xy, vTexturePos.w);
}

struct SamplerData1D 
	{ 
		#ifdef D3D11
			SamplerState	samplerState; 
			Texture1D		samplerObject;
		#else
			sampler1D		samplerObject;
		#endif

	};
	
struct SamplerData2D 
	{ 
		#ifdef D3D11
			SamplerState	samplerState; 
			Texture2D		samplerObject;
		#else
			sampler2D		samplerObject;
		#endif
	};
	
struct SamplerData3D 
	{ 
		#ifdef D3D11
			SamplerState	samplerState; 
			Texture3D		samplerObject;
		#else
			sampler3D		samplerObject;
		#endif
	};

struct SamplerDataCube
	{ 
		#ifdef D3D11
			SamplerState	samplerState; 
			TextureCube		samplerObject;
		#else
			samplerCUBE		samplerObject;
		#endif
	};


//-----------------------------------------------------------------------------

#ifdef D3D11

float4 FFP_SampleTextureLOD(in SamplerData1D s,in float f,in float lod) 
	{ 
		return s.samplerObject.SampleLevel(s.samplerState,f,lod);
	}
	
	float4 FFP_SampleTextureLOD(in SamplerData2D s,in float2 f,in float lod) 
	{ 
		return s.samplerObject.SampleLevel(s.samplerState,f,lod);
	}
	
	float4 FFP_SampleTextureLOD(in SamplerData3D s,in float3 f,in float lod) 
	{ 
		return s.samplerObject.SampleLevel(s.samplerState,f,lod);
	}
	float4 FFP_SampleTextureLOD(in SamplerDataCube s,in float3 f,in float lod) 
	{ 
		return s.samplerObject.SampleLevel(s.samplerState,f,lod);
	}
	float4 FFP_SampleTextureLOD(in Texture2DArray s,in SamplerState state, in float3 f,float lod) 
	{ 
		return s.SampleLevel(state,f,lod);
	}
#ifdef HLSL_VS
	float4 FFP_SampleTexture(in SamplerData1D	s,	in float f)		{ return FFP_SampleTextureLOD(s,f,0);}
	float4 FFP_SampleTexture(in SamplerData2D	s,	in float2 f)	{ return FFP_SampleTextureLOD(s,f,0);}
	float4 FFP_SampleTexture(in SamplerData3D	s,	in float3 f)	{ return FFP_SampleTextureLOD(s,f,0);}
	float4 FFP_SampleTexture(in SamplerDataCube	s,	in float3 f)	{ return FFP_SampleTextureLOD(s,f,0);}
	//Special case for TetxtureArray in D3D11
	float4 FFP_SampleTexture(in Texture2DArray s,in SamplerState state, in float3 f)	{ return FFP_SampleTextureLOD(s,state,f,0);}
#else
	float4 FFP_SampleTexture(in SamplerData1D	s,	in float f)		{ return s.samplerObject.Sample(s.samplerState,f);}	
	float4 FFP_SampleTexture(in SamplerData2D	s,	in float2 f)	{ return s.samplerObject.Sample(s.samplerState,f);}
	float4 FFP_SampleTexture(in SamplerData3D	s,	in float3 f)	{ return s.samplerObject.Sample(s.samplerState,f);}
	float4 FFP_SampleTexture(in SamplerDataCube	s,	in float3 f)	{ return s.samplerObject.Sample(s.samplerState,f);}
	//Special case for TetxtureArray in D3D11
	float4 FFP_SampleTexture(in Texture2DArray s,in SamplerState state, in float3 f)	{ return s.Sample(state,f);}
#endif	
	
	void FFP_Construct_Sampler_Wrapper(in Texture1D samplerObject,in SamplerState samplerState,out SamplerData1D samplerData)
	{
		samplerData.samplerObject = samplerObject;
		samplerData.samplerState = samplerState;
	}
	
	void FFP_Construct_Sampler_Wrapper(in Texture2D samplerObject,in SamplerState samplerState,out SamplerData2D samplerData)
	{
			samplerData.samplerObject = samplerObject;
			samplerData.samplerState = samplerState;
	}
	
	void FFP_Construct_Sampler_Wrapper(in Texture3D samplerObject,in SamplerState samplerState,out SamplerData3D samplerData)
	{
			samplerData.samplerObject = samplerObject;
			samplerData.samplerState = samplerState;
	}
	
	void FFP_Construct_Sampler_Wrapper(in TextureCube samplerObject,in SamplerState samplerState,out SamplerDataCube samplerData)
	{
			samplerData.samplerObject = samplerObject;
			samplerData.samplerState = samplerState;
	}
#else //D3D9 
	float4 FFP_SampleTexture(in SamplerData1D	s,	in float f)		{ return tex1D	(s.samplerObject, f);}
	float4 FFP_SampleTexture(in SamplerData2D	s,	in float2 f)	{ return tex2D	(s.samplerObject, f);}
	float4 FFP_SampleTexture(in SamplerData3D	s,	in float3 f)	{ return tex3D	(s.samplerObject, f);}
	float4 FFP_SampleTexture(in SamplerDataCube	s,	in float3 f)	{ return texCUBE(s.samplerObject, f);}
	
	float4 FFP_SampleTextureLOD(in SamplerData2D s,in float2 f,in float lod) 
	{ 
		return tex2Dlod(s.samplerObject, float4(f,0.0,lod));
	}
	
	float4 FFP_SampleTextureLOD(in SamplerData3D s,in float3 f,in float lod) 
	{ 
		return tex3Dlod(s.samplerObject, float4(f,lod));
	}
	
	void FFP_Construct_Sampler_Wrapper(in sampler1D samplerObject,out SamplerData1D samplerData)
	{
		samplerData.samplerObject = samplerObject;
	}
	
	void FFP_Construct_Sampler_Wrapper(in sampler2D samplerObject,out SamplerData2D samplerData)
	{
			samplerData.samplerObject = samplerObject;
	}
	
	void FFP_Construct_Sampler_Wrapper(in sampler3D samplerObject,out SamplerData3D samplerData)
	{
			samplerData.samplerObject = samplerObject;
	}
	
	void FFP_Construct_Sampler_Wrapper(in samplerCUBE samplerObject,out SamplerDataCube samplerData)
	{
			samplerData.samplerObject = samplerObject;
	}
	
#endif
//-----------------------------------------------------------------------------
//Special case for TetxtureArray in D3D11
#ifdef D3D11
void FFP_SampleTexture( in Texture2DArray s,
						in SamplerState state,
						in float3 f,
						out float4 t)
{
	t = FFP_SampleTexture(s,state, f);
}
#endif
//-----------------------------------------------------------------------------
void FFP_SampleTextureProj(in SamplerData2D s, 
				   in float3 f,
				   out float4 t)
{
	t = FFP_SampleTexture(s, f.xy/f.z);
}

//-----------------------------------------------------------------------------
void FFP_SampleTextureLOD(in SamplerData2D s, 
				   in float2 f,
				   in float lod,
				   out float4 t)
{
	t = FFP_SampleTextureLOD(s,f,lod);
}
//-----------------------------------------------------------------------------
void FFP_SampleTextureLOD(in SamplerData3D s, 
				   in float3 f,
				   in float lod,
				   out float4 t)
{
	t = FFP_SampleTextureLOD(s,f,lod);
}
//-----------------------------------------------------------------------------
void FFP_SampleTexture(in SamplerDataCube s, 
				   in float3 f,
				   out float4 t)
{
	t = FFP_SampleTexture(s, f);
}

//-----------------------------------------------------------------------------
void FFP_SampleTexture(in SamplerData2D s, 
				   in float2 f,
				   out float4 t)
{
	t = FFP_SampleTexture(s, f);
}
//-----------------------------------------------------------------------------
void FFP_SampleTexture(in SamplerData1D s, 
				   in float f,
				   out float4 t)
{
	t = FFP_SampleTexture(s, f);
}
//-----------------------------------------------------------------------------
void FFP_SampleTexture(in SamplerData3D s, 
				   in float3 f,
				   out float4 t)
{
	t = FFP_SampleTexture(s, f);
}
//-----------------------------------------------------------------------------
void FFP_ModulateX2(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 * vIn1 * 2;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in float2 vIn0, in float2 vIn1, out float2 vOut)
{
	vOut = vIn0 * vIn1 * 2;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in float3 vIn0, in float3 vIn1, out float3 vOut)
{
	vOut = vIn0 * vIn1 * 2;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in float4 vIn0, in float4 vIn1, out float4 vOut)
{
	vOut = vIn0 * vIn1 * 2;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 * vIn1 * 4;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in float2 vIn0, in float2 vIn1, out float2 vOut)
{
	vOut = vIn0 * vIn1 * 4;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in float3 vIn0, in float3 vIn1, out float3 vOut)
{
	vOut = vIn0 * vIn1 * 4;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in float4 vIn0, in float4 vIn1, out float4 vOut)
{
	vOut = vIn0 * vIn1 * 4;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in float2 vIn0, in float2 vIn1, out float2 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in float3 vIn0, in float3 vIn1, out float3 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in float4 vIn0, in float4 vIn1, out float4 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in float2 vIn0, in float2 vIn1, out float2 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in float3 vIn0, in float3 vIn1, out float3 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in float4 vIn0, in float4 vIn1, out float4 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}
