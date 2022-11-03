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

#ifdef OGRE_HLSL
mat3 to_mat3(mat4 m) { return (mat3)m; }
#else
#define to_mat3 mat3
#endif

//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec2 v, out vec2 vOut)
{
	vOut = mul(m, vec4(v, 0.0, 1.0)).xy;
}
//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec4 v, out vec2 vOut)
{
	vOut = mul(m, v).xy;
}

//-----------------------------------------------------------------------------
void FFP_TransformTexCoord(in mat4 m, in vec3 v, out vec3 vOut)
{
	vOut = mul(m, vec4(v, 1.0)).xyz;
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Normal(in mat3 mWorldIT,
						   in vec3 vNormal,
						   out vec3 vOut)
{
	vOut = normalize(mul(mWorldIT, vNormal));
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Sphere(in 	mat4 mWorldView,
										in 	mat3 mWorldIT,
										in 	vec4 vPos,
										in 	vec3 vNormal,
										out vec2 vOut)
{
	vec3 normal = normalize( mul(mWorldIT, vNormal));
	vec3 eyedir =  normalize(mul(mWorldView, vPos)).xyz;
	vec3 r = reflect(eyedir, normal);
	r.z += 1.0;
	float two_p = 2.0 * length(r);
	vOut = vec2(0.5 + r.x / two_p, 0.5 - r.y / two_p);
}

//-----------------------------------------------------------------------------
void FFP_GenerateTexCoord_EnvMap_Reflect(in mat4 mWorld, 
							in mat4 mWorldIT, 
						   in mat4 mView,						  
						   in vec3 vNormal,
						   in vec4 vPos,						  
						   out vec3 vOut)
{
#ifdef OGRE_HLSL
	mView[2][0] = -mView[2][0];
	mView[2][1] = -mView[2][1];
	mView[2][2] = -mView[2][2];
	mView[2][3] = -mView[2][3];
#else
	mView[0][2] = -mView[0][2];
	mView[1][2] = -mView[1][2];
	mView[2][2] = -mView[2][2];
	mView[3][2] = -mView[3][2];
#endif

	mat4 matViewT = transpose(mView);

	vec3 vWorldNormal = mul(to_mat3(mWorldIT), vNormal);
	vec3 vViewNormal  = mul(to_mat3(mView), vWorldNormal);
	vec4 vWorldPos    = mul(mWorld, vPos);
	vec3 vNormViewPos  = normalize(mul(mView, vWorldPos).xyz);
	
	vec3 vReflect = reflect(vNormViewPos, vViewNormal);

#ifdef OGRE_HLSL
	matViewT[2][0] = -matViewT[2][0];
	matViewT[2][1] = -matViewT[2][1];
	matViewT[2][2] = -matViewT[2][2];
#else
	matViewT[0][2] = -matViewT[0][2];
	matViewT[1][2] = -matViewT[1][2];
	matViewT[2][2] = -matViewT[2][2];
#endif
	vReflect = mul(to_mat3(matViewT), vReflect);

	vOut = vReflect;
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
//-----------------------------------------------------------------------------
void FFP_DotProduct(in float vIn0, in float vIn1, out float vOut)
{
	vOut = dot(vIn0, vIn1);
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vec2_splat(dot(vIn0, vIn1));
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vec3_splat(dot(vIn0, vIn1));
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vec4_splat(dot(vIn0, vIn1));
}