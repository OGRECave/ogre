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
void SGX_FetchNormal(in sampler2D s,
				   in vec2 uv,
				   out vec3 vOut)
{
	vOut = 2.0 * texture2D(s, uv).xyz - 1.0;
}

//-----------------------------------------------------------------------------
void SGX_TransformNormal(in vec3 vNormal,
				         in vec4 vTangent,
						 inout vec3 vNormalTS)
{
	// use non-normalised post-interpolation values as in mikktspace
	// resulting normal will be normalised by lighting
	vec3 vBinormal = cross(vNormal, vTangent.xyz) * sign(vTangent.w);
	// direction: from tangent space to world
	mat3 TBN = mtxFromCols(vTangent.xyz, vBinormal, vNormal);
	vNormalTS = mul(TBN, vNormalTS);
}

//-----------------------------------------------------------------------------
void SGX_Generate_Parallax_Texcoord(in sampler2D normalHeightMap,
						in vec2 texCoord,
						in vec3 viewPos,
						in vec2 scaleBias,
						out vec2 newTexCoord)
{
	vec3 eyeVec = -normalize(viewPos);
	float height = texture2D(normalHeightMap, texCoord).a;
	float displacement = (height * scaleBias.x) + scaleBias.y;
	vec2 scaledEyeDir = eyeVec.xy * displacement;
	newTexCoord = scaledEyeDir + texCoord;
}