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
	
	Post shader: Generic fullscreen quad
*/
struct VS_OUTPUT 
{
   float4 Pos: POSITION;
   float2 texCoord: TEXCOORD0;
   float3 projCoord: TEXCOORD1;
};
float4x4 invProj;

VS_OUTPUT main(float4 Pos: POSITION)
{
	VS_OUTPUT Out;

	// Clean up inaccuracies
	Pos.xy = sign(Pos.xy);

	Out.Pos = float4(Pos.xy, 0, 1);

	// Image-space
	Out.texCoord.x = 0.5 * (1 + Pos.x);
	Out.texCoord.y = 0.5 * (1 - Pos.y);

	// Projection coordinates
	// Inverted projection matrix
	// These interpolate over the frustrum plane for w=1
	Out.projCoord = float3(Pos.x, Pos.y, 1) *
		float3(
		invProj[0][0], // X vector component from X
		invProj[1][1], // Y vector component from Y
		invProj[2][3]  // Z vector component from W
	);

	return Out;
}



