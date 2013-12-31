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
// Program Name: FFPLib_Common
// Program Desc: Common functions of the FFP.
// Program Type: Vertex/Pixel shader
// Language: GLSL
// Notes: Common functions needed by all FFP implementation classes.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void FFP_Assign(in float vIn, out float vOut)
{
	vOut = vIn;
}
//-----------------------------------------------------------------------------
void FFP_Assign(in vec2 vIn, out vec2 vOut)
{
	vOut = vIn;
}

//-----------------------------------------------------------------------------
void FFP_Assign(in vec3 vIn, out vec3 vOut)
{
	vOut = vIn;
}

//-----------------------------------------------------------------------------
void FFP_Assign(in vec4 vIn, out vec4 vOut)
{
	vOut = vIn;
}
//-----------------------------------------------------------------------------
void FFP_Assign(in vec4 vIn, out vec2 vOut)
{
	vOut = vIn.xy;
}
//-----------------------------------------------------------------------------
void FFP_Assign(in vec4 vIn, out vec3 vOut)
{
	vOut = vIn.xyz;
}
//-----------------------------------------------------------------------------
void FFP_Assign(in mat2x4 vIn, out mat2x4 vOut)
{
	vOut = vIn;
}
//-----------------------------------------------------------------------------
void FFP_Assign(in mat3x4 vIn, out mat3x4 vOut)
{
	vOut = vIn;
}
//-----------------------------------------------------------------------------
void FFP_Construct(in float r, 
			 in float g,
			 in float b,
			 in float a,
			 out vec4 vOut)
{
	vOut = vec4(r,g,b,a);
}

//-----------------------------------------------------------------------------
void FFP_Construct(in float r, 
			 in float g,
			 in float b,
			 out vec3 vOut)
{
	vOut = vec3(r,g,b);
}

//-----------------------------------------------------------------------------
void FFP_Construct(in float r, 				   
				   out vec4 vOut)
{
	vOut = vec4(r,r,r,r);
}

//-----------------------------------------------------------------------------
void FFP_Construct(in float r, 
			 in float g,
			 out vec2 vOut)
{
	vOut = vec2(r,g);
}

//-----------------------------------------------------------------------------
void FFP_Modulate(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 * vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Modulate(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 * vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Modulate(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 * vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Modulate(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 * vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 + vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 + vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 + vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 + vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in mat2x4 vIn0, in mat2x4 vIn1, out mat2x4 vOut)
{
	vOut = vIn0 + vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Add(in mat3x4 vIn0, in mat3x4 vIn1, out mat3x4 vOut)
{
	vOut = vIn0 + vIn1;
}
//-----------------------------------------------------------------------------
void FFP_Subtract(in float vIn0, in float vIn1, out float vOut)
{
	vOut = vIn0 - vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Subtract(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vIn0 - vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Subtract(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vIn0 - vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Subtract(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 - vIn1;
}

//-----------------------------------------------------------------------------
void FFP_Lerp(in float vIn0, in float vIn1, float T, out float vOut)
{
	vOut = mix(vIn0, vIn1, T);
}

//-----------------------------------------------------------------------------
void FFP_Lerp(in vec2 vIn0, in vec2 vIn1, float T, out vec2 vOut)
{
	vOut = mix(vIn0, vIn1, T);
}

//-----------------------------------------------------------------------------
void FFP_Lerp(in vec3 vIn0, in vec3 vIn1, float T, out vec3 vOut)
{
	vOut = mix(vIn0, vIn1, T);
}

//-----------------------------------------------------------------------------
void FFP_Lerp(in vec4 vIn0, in vec4 vIn1, float T, out vec4 vOut)
{
	vOut = mix(vIn0, vIn1, T);
}

//-----------------------------------------------------------------------------
void FFP_Lerp(in vec4 vIn0, in vec4 vIn1, vec4 T, out vec4 vOut)
{
	vOut = mix(vIn0, vIn1, T);
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in float vIn0, in float vIn1, out float vOut)
{
	vOut = dot(vIn0, vIn1);
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec2 vIn0, in vec2 vIn1, out vec2 vOut)
{
	vOut = vec2(dot(vIn0, vIn1), 1.0);
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec3 vIn0, in vec3 vIn1, out vec3 vOut)
{
	vOut = vec3(dot(vIn0, vIn1), 1.0, 1.0);
}

//-----------------------------------------------------------------------------
void FFP_DotProduct(in vec4 vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vec4(dot(vIn0, vIn1), 1.0, 1.0, 1.0);
}
