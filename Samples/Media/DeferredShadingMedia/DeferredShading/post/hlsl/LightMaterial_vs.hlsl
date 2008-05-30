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
	
	Post shader: Light geometry material
*/
struct VS_OUTPUT 
{
	float4 pos: POSITION;
	float2 texCoord: TEXCOORD0;
	float3 projCoord: TEXCOORD1;
};

uniform float vpWidth, vpHeight;
uniform float4x4 worldViewProj;
uniform float4x4 invProj;

VS_OUTPUT main(float4 Pos: POSITION)
{
	VS_OUTPUT Out;

	float4 projPos = mul(worldViewProj, Pos);
	projPos = projPos/projPos.w;
	//projPos[2] = 0;

	// projPos is now in nonhomogeneous 2d space -- this makes sure no perspective interpolation is
	// done that could mess with our concept.
	Out.pos = projPos;

	//float3 position = mul(invProj, float4(projCoord, 0, 1))*distance; 
	// Acquire view space position via inverse projection transformation
	// Optimisation for perspective, symmetric view frustrums 
	// These interpolate over the frustrum plane for w=1
	Out.projCoord = float3(projPos[0], projPos[1], 1)*float3(
		invProj[0][0], // X vector component from X
		invProj[1][1], // Y vector component from Y
		invProj[2][3]  // Z vector component from W
	);

	// Texture coordinate magic, this compensates for jitter
	float2 texCoord = float2(projPos[0]/2+0.5, -projPos[1]/2+0.5);
	float2 texSize = float2(vpWidth, vpHeight);
	texCoord = floor(texCoord * texSize)/texSize;
	Out.texCoord = texCoord;

	return Out;
}
