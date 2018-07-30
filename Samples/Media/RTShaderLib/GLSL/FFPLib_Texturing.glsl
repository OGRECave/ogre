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
// Program Name: FFPLib_Texturing
// Program Desc: Texture functions of the FFP.
// Program Type: Vertex/Pixel shader
// Language: GLSL
// Notes: Implements core functions for FFPTexturing class.
// based on texturing operations needed by render system.
// Implements texture coordinate processing:
// see http://msdn.microsoft.com/en-us/library/bb206247.aspx
// Implements texture blending operation:
// see http://msdn.microsoft.com/en-us/library/bb206241.aspx
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec2 v, out vec2 vOut)
{
	vOut = (m * vec4(v, 0.0, 1.0)).xy;
}
//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec4 v, out vec2 vOut)
{
	vOut = (m * v).xy;
}

//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec3 v, out vec3 vOut)
{
	vOut = (m * vec4(v, 1.0)).xyz;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Normal(in mat4 mWorldIT, 
						   in mat4 mView,
						   in vec3 vNormal,
						   out vec3 vOut)
{
	vec3 vWorldNormal = (mat3(mWorldIT) * vNormal);
	vec3 vViewNormal  = (mat3(mView) * vWorldNormal);

	vOut = vViewNormal;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Sphere(in 	mat4 mWorld,
										in 	mat4 mView,
										in 	mat4 mWorldIT,
										in 	vec4 vPos,
										in 	vec3 vNormal,
										out vec2 vOut)
{
	mat4 worldview = mView * mWorld;
	vec3 normal = normalize( (mWorldIT * vec4(vNormal,0.0)).xyz); 
	vec3 eyedir =  normalize(worldview * vPos).xyz;
	vec3 r = reflect(eyedir, normal);
	float two_p = 2.0 * sqrt( r.x *  r.x +  r.y *  r.y +  (r.z + 1) *  (r.z + 1));
	vOut = vec2(0.5 + r.x / two_p,0.5 - r.y / two_p);
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Reflect(in mat4 mWorld, 
							in mat4 mWorldIT, 
						   in mat4 mView,						  
						   in vec3 vNormal,
						   in vec4 vPos,						  
						   out vec3 vOut)
{		
	mView[0][2] = -mView[0][2];
	mView[1][2] = -mView[1][2];
	mView[2][2] = -mView[2][2];
	mView[3][2] = -mView[3][2];
	
	mat4 matViewT = transpose(mView);

	vec3 vWorldNormal = (mat3(mWorldIT) * vNormal);
	vec3 vViewNormal  = (mat3(mView) * vWorldNormal);
	vec4 vWorldPos    = mWorld * vPos;
	vec3 vNormViewPos  = normalize((mView * vWorldPos).xyz);
	
	vec3 vReflect = reflect(vNormViewPos, vViewNormal);

  	matViewT[0][2] = -matViewT[0][2];
 	matViewT[1][2] = -matViewT[1][2];
  	matViewT[2][2] = -matViewT[2][2];
 	vReflect = (mat3(matViewT) * vReflect);

	vOut = vReflect;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_Projection(in mat4 mWorld, 							
						   in mat4 mTexViewProjImage,					  			
						   in vec4 vPos,						  				  
						   out vec3 vOut)
{
	vec4 vWorldPos    = mWorld * vPos;
	vec4 vTexturePos  = mTexViewProjImage * vWorldPos;

	vOut = vTexturePos.xyw;
}

//-----------------------------------------------------------------------------
void FFP_SampleTexture(in sampler1D s, 
				   in float f,
				   out vec4 t)
{
	t = texture1D(s, f);
}

//-----------------------------------------------------------------------------
void FFP_SampleTexture(in sampler2D s, 
				   in vec2 f,
				   out vec4 t)
{
	t = texture2D (s, f);
}
//-----------------------------------------------------------------------------
void FFP_SampleTexture(in sampler2D s, 
				   in vec4 f,
				   out vec4 t)
{
	t = texture2D (s, vec2(f.xy));
}

//-----------------------------------------------------------------------------
void FFP_SampleTextureProj(in sampler2D s, 
				   in vec3 f,
				   out vec4 t)
{
	t = texture2D(s, f.xy/f.z);
}

//-----------------------------------------------------------------------------
void FFP_SampleTexture(in sampler3D s, 
				   in vec3 f,
				   out vec4 t)
{
	t = texture3D(s, f);
}

//-----------------------------------------------------------------------------
void FFP_SampleTexture(in samplerCube s, 
				   in vec3 f,
				   out vec4 t)
{
	t = textureCube(s, f);
}


//-----------------------------------------------------------------------------
void FFP_ModulateX2(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 * vIn1 * 2.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 * vIn1 * 2.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 * vIn1 * 2.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX2(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 * vIn1 * 2.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 * vIn1 * 4.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 * vIn1 * 4.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 * vIn1 * 4.0;
}

//-----------------------------------------------------------------------------
void FFP_ModulateX4(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 * vIn1 * 4.0;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSigned(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 + vIn1 - 0.5;
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}

//-----------------------------------------------------------------------------
void FFP_AddSmooth(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 + vIn1 - (vIn0 * vIn1);
}
