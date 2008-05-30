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
	
	Material shader: Single colour normal mapped
*/
sampler Tex0: register(s0);

struct OUTPUT
{
	float4 color0 : COLOR0; // Diffuse color + ?
	float4 color1 : COLOR1; // Normal + depth
};

float3 colour;
float specularity;

OUTPUT main(
	float3 normal: TEXCOORD0,
	float4 texCoord0: TEXCOORD1,
	float depth: TEXCOORD2,
	float3 tangent: TEXCOORD3,
	float3 binormal: TEXCOORD4
)
{
	OUTPUT output;
	// Frame for normal mapping
	float3x3 rotation = float3x3(tangent, binormal, normal);
	float3 texnormal = (tex2D(Tex0, texCoord0)-0.5)*2;
	
	output.color0.rgb = colour;
	output.color0.a = specularity;
	output.color1.xyz = normalize(mul(texnormal,rotation)); // Do normalisation here, saves an operation per light
	output.color1.w = depth;
	
	return output;
}
