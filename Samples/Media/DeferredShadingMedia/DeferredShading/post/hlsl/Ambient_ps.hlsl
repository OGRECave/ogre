/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
/** Deferred shading framework
	// W.J. :wumpus: van der Laan 2005 //
	
	Post shader: Multipass, ambient (base) pass
*/
sampler Tex0: register(s0);
sampler Tex1: register(s1);

float4x4 proj;

float4 ambientColor;

struct POUTPUT
{
	float4 colour: COLOR;
	float depth: DEPTH;
};

POUTPUT main(float2 texCoord: TEXCOORD0, float3 projCoord: TEXCOORD1)
{
	float4 a0 = tex2D(Tex0, texCoord); // Attribute 0: Diffuse color+shininess
	float4 a1 = tex2D(Tex1, texCoord); // Attribute 1: Normal+depth

	// Clip fragment if depth is too close, so the skybox can be rendered on the background
	clip(a1.w-0.001);

	POUTPUT o;
	// Calculate ambient colour of fragment
	o.colour = float4( ambientColor*a0.rgb ,0);

	// Calculate depth of fragment;
	o.depth = projCoord.z*proj[2][2] + proj[2][3]/a1.w;
	return o;
}
