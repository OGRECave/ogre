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
void main(
	float4 Pos: POSITION,
	
	out float4 oPos: POSITION,
	out float2 oTexCoord: TEXCOORD0,
	
	out float3 oRay : TEXCOORD1,
	
	uniform float3 farCorner,
	uniform float flip
	)
{
	// Clean up inaccuracies
	Pos.xy = sign(Pos.xy);
	
	oPos = float4(Pos.xy, 0, 1);
	oPos.y *= flip;
	
	// Image-space
	oTexCoord.x = 0.5 * (1 + Pos.x);
	oTexCoord.y = 0.5 * (1 - Pos.y);

	// This ray will be interpolated and will be the ray from the camera
	// to the far clip plane, per pixel
	oRay = farCorner * float3(Pos.xy, 1);
}



